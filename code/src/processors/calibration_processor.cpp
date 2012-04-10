#include "calibration_processor.h"
#include "util.h"

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
  if(f.absoluteFilePath() == m_points3d.absoluteFilePath()) return;
  m_points3d = f;
  mutex.unlock();
  process();
}
