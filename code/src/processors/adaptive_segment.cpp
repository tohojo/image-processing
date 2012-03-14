#include "adaptive_segment.h"

AdaptiveSegment::AdaptiveSegment(QObject *parent)
  : Processor(parent), m_adapt(true), m_background(LIGHT)
{
}

AdaptiveSegment::~AdaptiveSegment()
{
}

void AdaptiveSegment::process()
{
  if(input_image.empty()) return;
  
  Scalar m = mean(input_image);
  int threshold = cvRound(m[0]);

  if(m_adapt) {
    qDebug("Threshold before: %d", threshold);
    adaptThreshold(input_image, &threshold);
    qDebug("Threshold after: %d", threshold);
  }


  if(m_background == DARK)
    output_image = input_image < threshold;
  else
    output_image = input_image >= threshold;

  emit updated();
}

void AdaptiveSegment::adaptThreshold(Mat I, int* threshold)
{
  int old_threshold;
  do {
    old_threshold = *threshold;
    Mat above = I >= *threshold;
    Mat below = I < *threshold;
    Scalar mean_above = mean(I, above);
    Scalar mean_below = mean(I, below);
    *threshold = cvRound(((mean_above[0]+mean_below[0])/2));

} while(old_threshold != *threshold);
}


void AdaptiveSegment::setAdapt(const bool adapt)
{
  m_adapt = adapt;
  process();
}

void AdaptiveSegment::setBackground(const Background bg)
{
  m_background = bg;
  process();
}
