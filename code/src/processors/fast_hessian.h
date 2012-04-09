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
#include "response_layer.h"
#include <QtCore/QList>

using namespace cv;

static const int OCTAVES = 5;
static const int INTERVALS = 4;

class FastHessian
{
public:
  FastHessian(Mat &img, int octaves, int intervals, int init_sample, float threshold);
  ~FastHessian();

  QList<KeyPoint> interestPoints() { return m_ipoints; };
  void compute();

private:
  void buildResponseMap();
  void buildResponseLayer(ResponseLayer *layer);
  inline float filterX(Point p, int lobe);
  inline float filterY(Point p, int lobe);
  inline float filterXY(Point p, int lobe);
  bool maximal(Point pt, ResponseLayer *b, ResponseLayer *m, ResponseLayer *t);
  void addPoint(Point pt, ResponseLayer *b, ResponseLayer *m, ResponseLayer *t);
  void interpolate(Point p, ResponseLayer *b, ResponseLayer *m, ResponseLayer *t,
                   double *dx, double *dy, double *ds);
  Mat hessian3D(Point p, ResponseLayer *b, ResponseLayer *m, ResponseLayer *t);
  Mat deriv3D(Point p, ResponseLayer *b, ResponseLayer *m, ResponseLayer *t);
  Mat m_img;
  IntegralImage *m_integral;
  int m_octaves;
  int m_intervals;
  int m_init_sample;
  float m_threshold;
  QList<ResponseLayer *> m_layers;
  QList<KeyPoint> m_ipoints;
};

#endif
