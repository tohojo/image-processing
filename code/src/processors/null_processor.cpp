#include "null_processor.h"

NullProcessor::NullProcessor(QObject *parent)
  : Processor(parent)
{
}

NullProcessor::~NullProcessor()
{
}

void NullProcessor::process()
{
  output_image = input_image;
}
