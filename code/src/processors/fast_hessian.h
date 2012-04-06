/**
 * fast_hessian.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-06
 */

#ifndef FAST_HESSIAN_H
#define FAST_HESSIAN_H

#include <cv.h>
#include "integral_image.h"
#include <QtCore/QList>

using namespace cv;


class FastHessian
{
public:
  FastHessian(Mat &img, int octaves, int intervals, float threshold);
  ~FastHessian();

  QList<Point> interestPoints() { return m_ipoints; };
  void compute();

private:
  float filterX(Point p, int size_s, int size_l);
  float filterY(Point p, int size_s, int size_l);
  float filterXY(Point p, int size);
  bool maximal(Point pt, int i);
  void addPoint(Point pt, int i);
  Mat m_img;
  IntegralImage *m_integral;
  bool m_computed;
  int m_octaves;
  int m_intervals;
  float m_threshold;
  Mat *m_scales;
  QList<Point> m_ipoints;
};

#endif
