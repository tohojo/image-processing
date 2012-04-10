/**
 * threshold_segmenter.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-10
 */

#include "threshold_segmenter.h"
#include <QDebug>

ThresholdSegmenter::ThresholdSegmenter(Mat img, bool dark_bg)
{
  m_input = img;
  img.copyTo(m_output);
  m_threshold = 0;
  m_dark_bg = dark_bg;
}

ThresholdSegmenter::~ThresholdSegmenter()
{
}

void ThresholdSegmenter::compute(bool adapt)
{
  Scalar m = mean(m_input);
  m_threshold = cvRound(m[0]);

  if(adapt) {
    qDebug("Threshold before: %d", m_threshold);
    adaptThreshold();
    qDebug("Threshold after: %d", m_threshold);
  }


  if(m_dark_bg)
    m_output = m_input >= m_threshold;
  else
    m_output = m_input < m_threshold;
}

void ThresholdSegmenter::adaptThreshold()
{
  int old_threshold;
  do {
    old_threshold = m_threshold;
    Mat above = m_input >= m_threshold;
    Mat below = m_input < m_threshold;
    Scalar mean_above = mean(m_input, above);
    Scalar mean_below = mean(m_input, below);
    m_threshold = cvRound(((mean_above[0]+mean_below[0])/2));

  } while(old_threshold != m_threshold);
}
