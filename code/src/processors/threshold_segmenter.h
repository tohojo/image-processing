/**
 * threshold_segmenter.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-10
 */

#ifndef THRESHOLD_SEGMENTER_H
#define THRESHOLD_SEGMENTER_H

#include <cv.h>

using namespace cv;

class ThresholdSegmenter
{
public:
  ThresholdSegmenter(Mat img, bool dark_bg);
  ~ThresholdSegmenter();

  void compute(bool adapt);
  Mat output() {return m_output;}
  int threshold() {return m_threshold;}

private:
  void adaptThreshold();
  Mat m_input;
  Mat m_output;
  int m_threshold;
  bool m_dark_bg;
};

#endif
