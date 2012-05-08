#include "rectification_processor.h"

RectificationProcessor::RectificationProcessor(QObject *parent)
  : Processor(parent)
{
}

RectificationProcessor::~RectificationProcessor()
{
}

void RectificationProcessor::run()
{
  forever {
    if(abort) return;
    mutex.lock();
    output_image = input_image;
    emit progress(100);
    emit updated();
    if(once) return;

    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}
