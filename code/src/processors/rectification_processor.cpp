#include "rectification_processor.h"
#include "util.h"
#include <QDebug>
#include <QtCore/qmath.h>

RectificationProcessor::RectificationProcessor(QObject *parent)
  : TwoImageProcessor(parent),
    calibration_results(),
    focal_length(1.0),
    R(3,3,CV_32F),
    T(3,1,CV_32F),
    rect(3,3,CV_32F),
    width(0),
    height(0),
    test_chessboard(false),
    chessboard_horiz(10),
    chessboard_vert(8)
{
  rect = Mat::eye(3,3,CV_32F);
  R = Mat::eye(3,3,CV_32F);
  uses_colour = true;
}

RectificationProcessor::~RectificationProcessor()
{
}

void RectificationProcessor::run()
{
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

  Mat left_rectified = Mat::zeros(left_img.rows, left_img.cols, left_img.type());
  Mat right_rectified = Mat::zeros(right_img.rows, right_img.cols, right_img.type());

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
      if(abort || restart) return;
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

  if(test_chessboard) test();
}

void RectificationProcessor::test()
{
  Mat left = getLeftOutput();
  Mat right = getRightOutput();
  if(left.empty() || right.empty()) return;

  qDebug() << "Finding chessboard corners for accuracy testing...";

  Size corner_size(chessboardHoriz(), chessboardVert());
  std::vector<Point2f> corners_l, corners_r;
  if(!findChessboardCorners(left, corner_size, corners_l,
                            CV_CALIB_CB_ADAPTIVE_THRESH + CV_CALIB_CB_NORMALIZE_IMAGE)) {
    qDebug() << "Error getting corners for left image.";
    return;
  }
  qDebug() << "Found" << corners_l.size() << "chessboard corners for left image.";
  emit progress(97);
  if(!findChessboardCorners(right, corner_size, corners_r,
                            CV_CALIB_CB_ADAPTIVE_THRESH + CV_CALIB_CB_NORMALIZE_IMAGE)) {
    qDebug() << "Error getting corners for right image.";
    return;
  }
  qDebug() << "Found" << corners_r.size() << "chessboard corners for right image.";

  if (corners_l.size() != corners_r.size()) {
    qDebug() << "Different numbers of corners found. Left:" << corners_l.size() << "Right:" << corners_r.size();
    return;
  }


  float total_diff;
  emit clearPOIs();
  QList<float> diffs;

  for(int i = 0; i < corners_l.size(); i++) {
    Point2f l = corners_l[i];
    Point2f r = corners_r[i];
    float diff = qAbs(l.y-r.y);
    diffs << diff;
    total_diff += diff;
    emit newPOI(QPoint(l.x, l.y));
    emit newPOI(QPoint(r.x+left.cols+5, r.y));
  }
  qDebug () << "Total diff:" << total_diff;
  float avg_diff = total_diff / diffs.size();
  float standard_dev;
  foreach(float d, diffs) {
    standard_dev += qPow(d-avg_diff, 2);
  }
  standard_dev = qSqrt(standard_dev/(diffs.size()-1));

  qDebug() << "Mean y-value difference between corners:" << avg_diff << "std dev" <<standard_dev;
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

void RectificationProcessor::setTestChessboard (bool test){
  QMutexLocker locker(&mutex);
  if(test_chessboard == test) return;
  test_chessboard = test;
  mutex.unlock();
  process();
}

void RectificationProcessor::setChessboardHoriz (int value){
  QMutexLocker locker(&mutex);
  if(chessboard_horiz == value) return;
  chessboard_horiz = value;
  mutex.unlock();
  process();
}

void RectificationProcessor::setChessboardVert (int value){
  QMutexLocker locker(&mutex);
  if(chessboard_vert == value) return;
  chessboard_vert = value;
  mutex.unlock();
  process();
}
