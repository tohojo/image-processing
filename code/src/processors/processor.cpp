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
  once = false;
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
  abort = false;

  if(once) {
    mutex.unlock();
    run();
  } else if(!isRunning()) {
    start(LowPriority);
  } else {
    restart = true;
    condition.wakeOne();
  }
}

void Processor::cancel()
{
  QMutexLocker locker(&mutex);
  abort = true;
  condition.wakeOne();
}


void Processor::run()
{
  forever {
    if(abort) return;
    if(once) return;
    mutex.lock();
    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}

void Processor::run_once()
{
  once = true;
  run();
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
