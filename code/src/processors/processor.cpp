/**
 * processor.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-03-29
 */

#include "processor.h"

Processor::Processor(QObject *parent)
  :QThread(parent)
{
  abort = false;
  restart = false;
}

Processor::~Processor()
{
  mutex.lock();
  abort = true;
  condition.wakeOne();
  mutex.unlock();

  wait();
}

void Processor::process()
{
  QMutexLocker locker(&mutex);

  if(!isRunning()) {
    start(LowPriority);
  } else {
    restart = true;
    condition.wakeOne();
  }
}


void Processor::run()
{
  forever {
    if(abort) return;
    mutex.lock();
    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}

void Processor::set_input(const Mat img)
{
  QMutexLocker locker(&mutex);
  input_image = img;
}

Mat Processor::get_output()
{
  QMutexLocker locker(&mutex);
  return output_image;
}
