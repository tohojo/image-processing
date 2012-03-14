#include "adaptive_segment.h"

AdaptiveSegment::AdaptiveSegment(QObject *parent)
  : Processor(parent)
{
}

AdaptiveSegment::~AdaptiveSegment()
{
}

void AdaptiveSegment::process()
{
  output_image = input_image;
}
