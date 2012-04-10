#include "calibration_processor.h"
#include "util.h"
#include <QDebug>

CalibrationProcessor::CalibrationProcessor(QObject *parent)
  : Processor(parent)
{
}

CalibrationProcessor::~CalibrationProcessor()
{
}

void CalibrationProcessor::run()
{
  forever {
    if(abort) return;
    mutex.lock();
    bool isEmpty = input_image.empty();
    mutex.unlock();
    if(!isEmpty) {
      emit progress(0);
      qDebug()<< m_points3d.canonicalFilePath();
      if(abort) return;
      if(!restart) {
        emit progress(100);
        emit updated();
      }
    }
    if(once) return;
    mutex.lock();
    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}


void CalibrationProcessor::setPoints3d(const QFileInfo f)
{
  QMutexLocker locker(&mutex);
  if(f.canonicalFilePath() == m_points3d.canonicalFilePath()) return;
  m_points3d = f;
  mutex.unlock();
  process();
}
