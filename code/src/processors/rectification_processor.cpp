#include "rectification_processor.h"
#include "util.h"
#include <QDebug>

RectificationProcessor::RectificationProcessor(QObject *parent)
  : TwoImageProcessor(parent),
    calibration_results(),
    focal_length(1.0),
    R(3,3,CV_32F),
    T(3,1,CV_32F),
    rect(3,3,CV_32F)
{
  rect = Mat::eye(3,3,CV_32F);
  R = Mat::eye(3,3,CV_32F);
}

RectificationProcessor::~RectificationProcessor()
{
}

void RectificationProcessor::run()
{
  forever {
    if(abort) return;
    loadCalibrationResults();
    emit progress(10);
    rectify();
    emit progress(100);
    emit updated();
    if(once) return;

    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}

void RectificationProcessor::loadCalibrationResults()
{
  mutex.lock();
  QString filename = calibration_results.canonicalFilePath();
  bool valid = calibration_results.exists();

  if(!valid) return;

  Mat Rl(3,3,CV_32F),Rr(3,3,CV_32F),Tl(3,1,CV_32F),Tr(3,1,CV_32F);
  QFile file(filename);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  QString flString = file.readLine();
  bool ok;
  float f1,f2;
  f1 = flString.toFloat(&ok);
  if(!ok) return;
  if(!Util::read_matrix(Rl,&file)) return;
  if(!Util::read_matrix(Tl,&file)) return;
  flString = file.readLine();
  f2 = flString.toFloat(&ok);
  if(!ok) return;
  if(!Util::read_matrix(Rr,&file)) return;
  if(!Util::read_matrix(Tr,&file)) return;

  qDebug() << "Focal lengths:" << f1 << f2;

  R = Rl.inv(DECOMP_SVD)*Rr;
  T = Tr-Tl;
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

  Mat dz = Mat::zeros(3,1,CV_32F);
  dz.at<float>(2,0) = 1;
  Mat e2;
  normalize(e1.cross(dz), e2);
  Mat e3;
  normalize(e1.cross(e2), e3);


  mutex.lock();
  rect.row(0) = -e1.t();
  rect.row(1) = e2.t();
  rect.row(2) = e3.t();

  qDebug() << "Matrix rect:";
  qDebug() << rect.at<float>(0,0) << rect.at<float>(0,1) << rect.at<float>(0,2);
  qDebug() << rect.at<float>(1,0) << rect.at<float>(1,1) << rect.at<float>(1,2);
  qDebug() << rect.at<float>(2,0) << rect.at<float>(2,1) << rect.at<float>(2,2);
  mutex.unlock();
}

void RectificationProcessor::rectify()
{
  if(input_image.empty() || right_image.empty()) return;
  if(input_image.rows != right_image.rows || input_image.cols != right_image.cols) {
    qWarning() << "Left and right images differ in size. Not rectifying.";
    return;
  }
  mutex.lock();
  Mat Rl = rect.inv(DECOMP_SVD);
  Mat Rr = (R * rect).inv(DECOMP_SVD);

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

  qDebug() << "Focal length:" << focal_length;

  Mat map_left_x(left_img.rows, left_img.cols, CV_32F);
  Mat map_left_y(left_img.rows, left_img.cols, CV_32F);
  Mat map_right_x(left_img.rows, left_img.cols, CV_32F);
  Mat map_right_y(left_img.rows, left_img.cols, CV_32F);

  Mat left_rectified = Mat::zeros(left_img.rows, left_img.cols, left_img.type());
  Mat right_rectified = Mat::zeros(right_img.rows, right_img.cols, right_img.type());

  float x_offset = left_img.cols/2;
  float y_offset = left_img.rows/2;
  qDebug() << "Offsets:" << x_offset << y_offset;

  for(int x = 0; x < left_img.cols; x++) {
    for(int y = 0; y < left_img.rows; y++) {
      Mat dest(3,1,CV_32F);
      float rx,ry;
      rx = x_offset-x;
      ry = y_offset-y;
      dest.at<float>(0) = rx;
      dest.at<float>(1) = ry;
      dest.at<float>(2) = focal_length;
      Mat left(Rl*dest);
      float rect_l = left.at<float>(2);
      left *= focal_length/rect_l;
      Mat right(Rr*dest);
      float rect_r = right.at<float>(2);
      right *= focal_length/rect_r;

      Point src(x,y);
      map_left_x.at<float>(src) = x_offset - left.at<float>(0);
      map_left_y.at<float>(src) = y_offset - left.at<float>(1);

      map_right_x.at<float>(src) = x_offset - right.at<float>(0);
      map_right_y.at<float>(src) = y_offset - right.at<float>(1);
      if(rx==ry && qAbs(rx) < 10) {
        qDebug("Dest: (%f,%f,%f)", dest.at<float>(0), dest.at<float>(1), dest.at<float>(2));
        qDebug("Left: (%f,%f,%f)", left.at<float>(0), left.at<float>(1), left.at<float>(2));
        qDebug("Right: (%f,%f,%f)", right.at<float>(0), right.at<float>(1), right.at<float>(2));

      }
    }
  }

  remap(left_img, left_rectified, map_left_x, map_left_y, INTER_LINEAR);
  emit progress(55);
  remap(right_img, right_rectified, map_right_x, map_right_y, INTER_LINEAR);

  mutex.lock();
  output_image = Util::combine(left_rectified, right_rectified);
  mutex.unlock();
}

void RectificationProcessor::setCalibrationResults(QFileInfo path)
{
  QMutexLocker locker(&mutex);
  if(path.canonicalFilePath() == calibration_results.canonicalFilePath()) return;
  calibration_results = path;
  mutex.unlock();
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
