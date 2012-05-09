#include "rectification_processor.h"
#include "util.h"
#include <QDebug>

RectificationProcessor::RectificationProcessor(QObject *parent)
  : TwoImageProcessor(parent)
{
}

RectificationProcessor::~RectificationProcessor()
{
}

void RectificationProcessor::run()
{
  forever {
    if(abort) return;
    loadCalibrationResults();
    mutex.lock();
    output_image = right_image;
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
  bool valid = calibration_results.isFile();
  mutex.unlock();

  Mat Rl(3,3,CV_64F),Rr(3,3,CV_64F),Tl(3,1,CV_64F),Tr(3,1,CV_64F);
  QFile file(filename);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  if(!Util::read_matrix(Rl,&file)) return;
  if(!Util::read_matrix(Tl,&file)) return;
  if(!Util::read_matrix(Rr,&file)) return;
  if(!Util::read_matrix(Tr,&file)) return;

  R = Rl.inv()*Rr;
  T = -Tl+Tr;

  qDebug() << "Matrix R:";
  qDebug() << R.at<double>(0,0) << R.at<double>(0,1) << R.at<double>(0,2);
  qDebug() << R.at<double>(1,0) << R.at<double>(1,1) << R.at<double>(1,2);
  qDebug() << R.at<double>(2,0) << R.at<double>(2,1) << R.at<double>(2,2);

  qDebug() << "Vector T:";
  qDebug() << T.at<double>(0,0);
  qDebug() << T.at<double>(1,0);
  qDebug() << T.at<double>(2,0);

}

void RectificationProcessor::setCalibrationResults(QFileInfo path)
{
  QMutexLocker locker(&mutex);
  if(path.canonicalFilePath() == calibration_results.canonicalFilePath()) return;
  calibration_results = path;
  mutex.unlock();
  process();
}
