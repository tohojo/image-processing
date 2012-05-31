#include "rectification_processor.h"
#include "util.h"
#include <QDebug>

RectificationProcessor::RectificationProcessor(QObject *parent)
  : TwoImageProcessor(parent),
    calibration_results(),
    focal_length(1.0),
    R(3,3,CV_32F),
    T(3,1,CV_32F),
    rect(3,3,CV_32F),
    width(0),
    height(0)
{
  rect = Mat::eye(3,3,CV_32F);
  R = Mat::eye(3,3,CV_32F);
}

RectificationProcessor::~RectificationProcessor()
{
}

void RectificationProcessor::run()
{

	// Please ignore this stupid hack :3
/*	QString qfile("Database/cal.txt");
	QFileInfo qinf(qfile);
	setCalibrationResults(qinf);
	for (int i = 4072; i <= 4114; i++){
		std::string left = "Database/DSCF";
		std::string right = "Database/DSCF";
		std::stringstream ss;
		ss << i;
		left.append(ss.str());
		right.append(ss.str());
		std::stringstream leftout;
		leftout << left;
		leftout << "rec_l.jpg";
		std::stringstream rightout;
		rightout << right;
		rightout << "rec_r.jpg";
		left.append("_l.jpg");
		right.append("_r.jpg");
		QString qleft(left.c_str());
		QString qright(right.c_str());
		input_image = Util::load_image_colour(qleft);
		right_image = Util::load_image_colour(qright);
		rectify();
		imwrite(leftout.str(), left_rectified);
		imwrite(rightout.str(), right_rectified);
	}*/

  forever {
    if(abort) return;
    emit progress(10);
    rectify();
    emit progress(100);
    emit updated();
    if(once) return;

    mutex.lock();
    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}

void RectificationProcessor::loadCalibrationResults()
{
  QMutexLocker l(&mutex);
  QString filename = calibration_results.canonicalFilePath();
  bool valid = calibration_results.exists();

  if(!valid) {
    rect = Mat::eye(3,3,CV_32F);
    R = Mat::eye(3,3,CV_32F);
    return;
  }

  Mat Rl(3,3,CV_32F),Rr(3,3,CV_32F),Tl(3,1,CV_32F),Tr(3,1,CV_32F);
  QFile file(filename);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  QString flString = file.readLine();
  bool ok;
  float f1,f2;
  int w, h;
  f1 = flString.section(" ", 0, 0).toFloat(&ok);
  if(!ok) return;
  w = flString.section(" ", 1, 1).toInt(&ok);
  if(!ok) return;
  h = flString.section(" ", 2, 2).toInt(&ok);
  if(!ok) return;
  if(!Util::read_matrix(Rl,&file)) return;
  if(!Util::read_matrix(Tl,&file)) return;
  flString = file.readLine();
  f2 = flString.section(" ", 0, 0).toFloat(&ok);
  if(!ok) return;
  if(!Util::read_matrix(Rr,&file)) return;
  if(!Util::read_matrix(Tr,&file)) return;

  qDebug() << "Dimensions:" << w << h;
  width = w; height = h;

  qDebug() << "Focal lengths:" << f1 << f2;

  // Calculate the needed rotation matrix and translation vector
  //
  // The rotation matrix should be rotating from the right image to
  // the left image, and is found by composing the inversion of the
  // right-image rotation matrix (which takes the world coordinate
  // system into the right camera coordinate system) with the
  // left-image rotation matrix (which takes the world coordinate
  // system into the left camera coordinate system).
  //
  // The translation vector is the difference between the two
  // translation vectors; the right translation vector is in the right
  // camera reference system, so it first have to be rotated by the
  // rotation matrix just computed, so the result becomes a
  // translation vector in the left camera reference frame.
  R = Rl*Rr.inv();
  T = R*Tr-Tl;

  // The focal length should be the same for both cameras, so we take
  // the average of the measured values.
  focal_length = (f1+f2)/2;


  qDebug() << "Matrix R:";
  qDebug() << R.at<float>(0,0) << R.at<float>(0,1) << R.at<float>(0,2);
  qDebug() << R.at<float>(1,0) << R.at<float>(1,1) << R.at<float>(1,2);
  qDebug() << R.at<float>(2,0) << R.at<float>(2,1) << R.at<float>(2,2);

  qDebug() << "Vector T:";
  qDebug() << T.at<float>(0,0);
  qDebug() << T.at<float>(1,0);
  qDebug() << T.at<float>(2,0);

  if(T.at<float>(0,0) < 0)
    qWarning() << "X-component of translation vector is < 0 - switched right and left images?";

  mutex.unlock();
  calculateRectMatrix();
}

void RectificationProcessor::calculateRectMatrix()
{
  mutex.lock();
  Mat e1;
  normalize(T, e1);
  mutex.unlock();

  // Compute the components of the rectification matrix.
  //
  // Using the fact that applying the rotation to an identity matrix
  // yields the same rotation matrix, we can construct the rotation
  // matrix by constructing an orthonormal base with the x coordinate
  // in the direction of the translation of the two camera origins.
  //
  // The orthonormal base is constructed using the cross product. It
  // is different from the slides because we want a right-hand
  // coordinate system.

  Mat dz = Mat::zeros(3,1,CV_32F);
  dz.at<float>(2,0) = 1;
  Mat e2;
  normalize(dz.cross(e1), e2);
  Mat e3;
  normalize(e1.cross(e2), e3);


  mutex.lock();
  rect.row(0) = e1.t();
  rect.row(1) = e2.t();
  rect.row(2) = e3.t();

  qDebug() << "Matrix rect:";
  qDebug() << rect.at<float>(0,0) << rect.at<float>(0,1) << rect.at<float>(0,2);
  qDebug() << rect.at<float>(1,0) << rect.at<float>(1,1) << rect.at<float>(1,2);
  qDebug() << rect.at<float>(2,0) << rect.at<float>(2,1) << rect.at<float>(2,2);
  mutex.unlock();
}

bool RectificationProcessor::canProcess()
{
  QMutexLocker l(&mutex);
  if(input_image.empty() || right_image.empty()) return false;
  if(input_image.rows != right_image.rows || input_image.cols != right_image.cols) {
    return false;
  }
  if(calibration_results.canonicalFilePath().isEmpty()) return false;
  return true;
}

Point RectificationProcessor::mapPoint(Point p, Side side)
{
  mutex.lock();
  Mat map;
  if(side == LEFT) {
    map = rect;
  } else {
    map = R * rect;
  }
  float flength = qAbs(focal_length);
  int w = width; int h = height;
  mutex.unlock();

  float x_offset = w/2.0;
  float y_offset = h/2.0;
  float rx = p.x-x_offset;
  float ry = p.y-y_offset;

  Mat point = (Mat_<float>(3,1) << rx ,ry, flength);
  Mat origin = (Mat_<float>(3,1) << 0.0, 0.0, flength);

  Mat mapped(map*point);
  float mapped_l = mapped.at<float>(2);
  mapped *= flength/mapped_l;

  Mat mapped_origin(map*origin);
  mapped_l = mapped_origin.at<float>(2);
  mapped_origin *= flength/mapped_l;

  qDebug() << "Point:" << point.at<float>(0) << point.at<float>(1) << " >> " << mapped.at<float>(0) << mapped.at<float>(1);
  qDebug() << "Origin:" << 0 << 0 << " >> " << mapped_origin.at<float>(0) << mapped_origin.at<float>(1);
  Point pt(mapped.at<float>(0)+x_offset, mapped.at<float>(1)+y_offset);
  qDebug() << pt.x  << pt.y;
  return pt;
}

void RectificationProcessor::rectify()
{
  if(input_image.empty() || right_image.empty()) return;
  if(input_image.rows != right_image.rows || input_image.cols != right_image.cols) {
    qWarning() << "Left and right images differ in size. Not rectifying.";
    return;
  }
  mutex.lock();
  Mat Rl = rect.inv();
  Mat Rr = (R * rect).inv();
  /*Mat left_img;
  Mat right_img;
  if(uses_colour){
	  left_img = Util::load_image_colour(input_image);
	  right_img = Util::load_image_colour(right_);
  }
  else{
	  left_img = Util::load_image(img_filename);
	  right_img = Util::load_image(img_filename);
  }*/
  Mat left_img = input_image;
  Mat right_img = right_image;
  mutex.unlock();

  qDebug() << "Rl:";
  qDebug() << Rl.at<float>(0,0) << Rl.at<float>(0,1) << Rl.at<float>(0,2);
  qDebug() << Rl.at<float>(1,0) << Rl.at<float>(1,1) << Rl.at<float>(1,2);
  qDebug() << Rl.at<float>(2,0) << Rl.at<float>(2,1) << Rl.at<float>(2,2);

  qDebug() << "Rr:";
  qDebug() << Rr.at<float>(0,0) << Rr.at<float>(0,1) << Rr.at<float>(0,2);
  qDebug() << Rr.at<float>(1,0) << Rr.at<float>(1,1) << Rr.at<float>(1,2);
  qDebug() << Rr.at<float>(2,0) << Rr.at<float>(2,1) << Rr.at<float>(2,2);

  float flength = qAbs(focal_length);
  qDebug() << "Focal length:" << flength;

  Mat map_left_x(left_img.rows, left_img.cols, CV_32F);
  Mat map_left_y(left_img.rows, left_img.cols, CV_32F);
  Mat map_right_x(left_img.rows, left_img.cols, CV_32F);
  Mat map_right_y(left_img.rows, left_img.cols, CV_32F);

  left_rectified = Mat::zeros(left_img.rows, left_img.cols, left_img.type());
  right_rectified = Mat::zeros(right_img.rows, right_img.cols, right_img.type());

  float x_offset = left_img.cols/2;
  float y_offset = left_img.rows/2;
  qDebug() << "Offsets:" << x_offset << y_offset;

  emit progress(20);

  // Readjust the centre after computing
  float max_x_l=-10000, max_x_r=-10000, max_y_l=-1000, max_y_r=-10000;
  float min_x_l=10000, min_x_r=10000, min_y_l=10000, min_y_r=10000;

  for(int x = 0; x < left_img.cols; x++) {
    int prog = 20 + (int)(70*((double)x / (double)left_img.cols)); // 20% to 90%
    if (prog % 5 == 0) emit progress(prog);
    for(int y = 0; y < left_img.rows; y++) {
      if(abort) return;
      Mat dest(3,1,CV_32F);
      float rx,ry;
      rx = x-x_offset;
      ry = y-y_offset;
      dest.at<float>(0) = rx;
      dest.at<float>(1) = ry;
      dest.at<float>(2) = flength;
      Mat left(Rl*dest);
      float rect_l = left.at<float>(2);
      left *= flength/rect_l;
      Mat right(Rr*dest);
      float rect_r = right.at<float>(2);
      right *= flength/rect_r;

      Point src(x,y);

      float x_l = left.at<float>(0);
      float y_l = left.at<float>(1);
      if(x_l>max_x_l) max_x_l = x_l;
      if(x_l<min_x_l) min_x_l = x_l;
      if(y_l>max_y_l) max_y_l = y_l;
      if(y_l<min_y_l) min_y_l = y_l;

      map_left_x.at<float>(src) = x_l;
      map_left_y.at<float>(src) = y_l;

      float x_r = right.at<float>(0);
      float y_r = right.at<float>(1);
      if(x_r>max_x_r) max_x_r = x_r;
      if(x_r<min_x_r) min_x_r = x_r;
      if(y_r>max_y_r) max_y_r = y_r;
      if(y_r<min_y_r) min_y_r = y_r;

      map_right_x.at<float>(src) = x_r;
      map_right_y.at<float>(src) = y_r;
    }
  }

  map_left_x += (max_x_l-min_x_l)/2;
  map_left_y += (max_y_l-min_y_l)/2;
  map_right_x += (max_x_r-min_x_r)/2;
  map_right_y += (max_y_r-min_y_r)/2;
  emit progress(90);
  remap(left_img, left_rectified, map_left_x, map_left_y, INTER_CUBIC);
  emit progress(95);
  remap(right_img, right_rectified, map_right_x, map_right_y, INTER_CUBIC);

  set_output_images(left_rectified, right_rectified);
}

void RectificationProcessor::setCalibrationResults(QFileInfo path)
{
  QMutexLocker locker(&mutex);
  if(path.canonicalFilePath() == calibration_results.canonicalFilePath()) return;
  calibration_results = path;
  mutex.unlock();
  loadCalibrationResults();
  process();
}

void RectificationProcessor::setFocalLength(float length)
{
  QMutexLocker locker(&mutex);
  if(length == focal_length) return;
  focal_length = length;
  mutex.unlock();
  process();
}

void RectificationProcessor::setuses_colour(bool yn){
  QMutexLocker locker(&mutex);
  if(uses_colour == yn) return;
  uses_colour = yn;
  mutex.unlock();
  process();
}
