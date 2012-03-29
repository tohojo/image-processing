#include "null_processor.h"

NullProcessor::NullProcessor(QObject *parent)
  : Processor(parent)
{
}

NullProcessor::~NullProcessor()
{
}

void NullProcessor::run()
{
  forever {
    if(abort) return;
    mutex.lock();
    output_image = input_image;
    emit progress(100);
    emit updated();

    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}
