/**
 * fast_hessian.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-06
 */

#include "fast_hessian.h"
#include <QDebug>

FastHessian::FastHessian(Mat &img, int octaves, int intervals, int init_sample, float threshold)
{
  m_img = img;
  m_octaves = (octaves > OCTAVES) ? OCTAVES : ((octaves < 0) ? 0 : octaves);
  m_intervals = (intervals > INTERVALS) ? INTERVALS : ((intervals < 3) ? 3 : intervals);
  m_init_sample = (init_sample > 0) ? init_sample : 1;
  m_threshold = threshold;
  m_integral = new IntegralImage(img);
}

FastHessian::~FastHessian()
{
  delete m_integral;
  foreach(ResponseLayer *l, m_layers)
    delete l;
}

void FastHessian::compute()
{
  // For each octave, only the filter sizes not already used for
  // smaller scales are added. This means that the (up to) four
  // interval layers for each octave will be at different points of
  // the response map. This mapping gives those indexes, corresponding
  // to the buildup in buildResponseMap().
  //
  // (Taken from the openSURF code).
  static const int filter_map [OCTAVES][INTERVALS] = {{0,1,2,3},
                                                      {1,3,4,5},
                                                      {3,5,6,7},
                                                      {5,7,8,9},
                                                      {7,9,10,11}};

  // start by clearing out any existing interest points and building
  // the Hessian response map.
  m_ipoints.clear();
  buildResponseMap();

  // Compute maxima by comparing points to the level above and below
  // them. The SURF article is quite vague on whether or not
  // comparison is made between different octaves. The SIFT article
  // does not mention this either. However, the opencv and the
  // opensurf implementations seem to agree that comparison is only
  // done within octaves, so this is the approach we take here.

  ResponseLayer *b,*m,*t;
  int o,i,x,y;

  // Loop through all octaves, and for octave, select m_interval-2
  // sets of top/middle/bottom. This does in practice mean that only 3
  // or 4 intervals pr octave make sense.
  for(o = 0; o < m_octaves; o++) {
    for(i = 0; i < m_intervals-2; i++) {
      b = m_layers.at(filter_map[o][i]);
      m = m_layers.at(filter_map[o][i+1]);
      t = m_layers.at(filter_map[o][i+2]);

      // Loop at the scale of the top-most (most sparse) layer
      for(y = 0; y < t->height(); y++) {
        for(x = 0; x < t->width(); x++) {
          Point pt(x,y);
          if(maximal(pt, b, m, t)) addPoint(pt, b, m, t);
        }
      }
    }
  }
}

void FastHessian::buildResponseMap()
{
  // Filter sizes (from [SURF]):
  // Octave 1:  9,  15,  21,  27
  // Octave 2: 15,  27,  39,  51
  // Octave 3: 27,  51,  75,  99
  // Octave 4: 51,  99, 147, 195
  // Octave 5: 99, 195, 291, 387 (from opensurf code)

  // Each response map is built using the scale of the lowest octave
  // that needs it (values at coarser scales can be directly read form
  // the finer scale representation, since scales are powers of two).
  foreach(ResponseLayer * layer, m_layers)
    delete layer;
  m_layers.clear();

  int w = (m_img.cols / m_init_sample);
  int h = (m_img.rows / m_init_sample);
  int s = m_init_sample;

  // For each octave, add the layers for filter sizes not included in
  // a smaller octave. Scale each octave by a factor 2.
  if(m_octaves >= 1) {
    m_layers.append(new ResponseLayer(w, h, s, 9));
    m_layers.append(new ResponseLayer(w, h, s, 15));
    m_layers.append(new ResponseLayer(w, h, s, 21));
    m_layers.append(new ResponseLayer(w, h, s, 27));
  }

  if(m_octaves >= 2) {
    m_layers.append(new ResponseLayer(w/2, h/2, s*2, 39));
    m_layers.append(new ResponseLayer(w/2, h/2, s*2, 51));
  }

  if(m_octaves >= 3) {
    m_layers.append(new ResponseLayer(w/4, h/4, s*4, 75));
    m_layers.append(new ResponseLayer(w/4, h/4, s*4, 99));
  }

  if(m_octaves >= 4) {
    m_layers.append(new ResponseLayer(w/8, h/8, s*8, 147));
    m_layers.append(new ResponseLayer(w/8, h/8, s*8, 195));
  }

  if(m_octaves >= 5) {
    m_layers.append(new ResponseLayer(w/16, h/16, s*8, 291));
    m_layers.append(new ResponseLayer(w/16, h/16, s*8, 387));
  }

  foreach(ResponseLayer *layer, m_layers)
    buildResponseLayer(layer);
}

void FastHessian::buildResponseLayer(ResponseLayer *layer)
{
  int step = layer->step();
  int size = layer->filter_size();
  int lobe = size/3;

  float inverse_area = 1.0f/(size*size);

  float Dxx, Dyy, Dxy;

  int x,y;

  for(y = 0; y<layer->height(); y++) {
    for(x = 0; x<layer->width(); x++) {
      Point p(x*step,y*step);

      Dxx = filterX(p, lobe) * inverse_area;
      Dyy = filterY(p, lobe) * inverse_area;
      Dxy = filterXY(p, lobe) * inverse_area;

      float detHess = (Dxx*Dyy - 0.81f*Dxy*Dxy); // (0.9Dxy)^2 == 0.81*Dxy*Dxy
      layer->setResponse(Point(x,y), detHess);
    }
  }
}

/**
 * Check whether a point is maximal, given its coordinates and the
 * scale space plane it is located in.
 *
 * Checks all neighbouring pixels in this, the previous and next
 * layers. If any have greater values, return false, otherwise return
 * true. Also returns false if the value is less than the threshold.
 */
bool FastHessian::maximal(Point pt, ResponseLayer *b, ResponseLayer *m, ResponseLayer *t)
{
  int border = (t->filter_size() +1) / (2 * t->step());
  if(pt.x <= border || pt.x >= t->width()-border ||
     pt.y <= border || pt.y >= t->height()-border)
    return false;

  float val = m->getResponse(pt, t);

  if(val < m_threshold) return false;

  for(int i = -1; i <= 1; i++) {
    for(int j = -1; j <= 1; j++) {
      Point p(pt.x+i, pt.y+j);
      if(t->getResponse(p) > val) return false;
      if(!(i==0&&j==0) && m->getResponse(p, t) > val) return false;
      if(b->getResponse(p, t) > val) return false;
    }
  }
  return true;
}

/**
 * Interpolate real point position and add it to the interesting points if it converges.
 *
 * TODO: Implement interpolation.
 */
void FastHessian::addPoint(Point pt, ResponseLayer *b, ResponseLayer *m, ResponseLayer *t)
{
  double dx, dy, ds;
  interpolate(pt, b, m, t, &dx, &dy, &ds);

  // Following the lead of opensurf and opencv, instead of doing
  // actual interpolation and checking for convergence, we just
  // discard points that do not correspond to the interpolated value.
  //
  // For opencv the cutoff value here is 1, for opensurf it's 0.5.
  if (qAbs(dx) < 0.5f && qAbs(dy) < 0.5f && qAbs(ds) < 0.5f) {
    m_ipoints.append(KeyPoint((pt.x+dx)*t->step(), (pt.y+dy)*t->step(), 1));
  }
}

/**
 * Interpolate the interest point to subpixel accuracy, by the method
 * from (Brown and Lowe, 2002).
 *
 * This comes down to solving the equation Ax=b where A is the
 * second-order derivative of the scale-space function shifted (i.e.
 * the 3D Hessian), and b = -(dx,dy,ds). The scale-space function is
 * assumed shifted so the interest point candidate is at the origin.
 * The solution to this equation yields the position (x,y,s) in
 * scale-space of the actual extremum.
 *
 * The values for the partial derivatives are approximated by point
 * value differences in the response layers.
 */
void FastHessian::interpolate(Point p, ResponseLayer *b, ResponseLayer *m, ResponseLayer *t,
                              double *dx, double *dy, double *ds)
{
  Mat X; // holds solution
  Mat h3d = hessian3D(p, b, m, t);
  Mat d3d = deriv3D(p, b, m, t);
  d3d *= -1;
  if(solve(h3d, d3d, X)) {
    *dx = X.at<double>(0,0);
    *dy = X.at<double>(1,0);
    *ds = X.at<double>(2,0);
  } else {
    *dx = *dy = *ds = 0.0f;
  }
}

Mat FastHessian::hessian3D(Point p, ResponseLayer *b, ResponseLayer *m, ResponseLayer *t)
{
  double val = m->getResponse(p,t);
  double dxx = m->getResponse(Point(p.x+1, p.y), t) +m->getResponse(Point(p.x-1,p.y),t) - 2*val;
  double dyy = m->getResponse(Point(p.x, p.y+1), t) +m->getResponse(Point(p.x,p.y-1),t) - 2*val;
  double dss = b->getResponse(p, t) + t->getResponse(p) - 2*val;
  double dxy = ( m->getResponse(Point(p.x+1,p.y+1), t) - m->getResponse(Point(p.x-1, p.y+1), t) +
                m->getResponse(Point(p.x-1,p.y-1), t) - m->getResponse(Point(p.x+1, p.y-1), t) ) /4.0;
  double dxs = ( t->getResponse(Point(p.x+1, p.y)) - t->getResponse(Point(p.x-1, p.y)) +
                b->getResponse(Point(p.x-1, p.y), t) - b->getResponse(Point(p.x+1, p.y), t) ) /4.0;
  double dys = ( t->getResponse(Point(p.x, p.y+1)) - t->getResponse(Point(p.x, p.y-1)) +
                b->getResponse(Point(p.x, p.y-1), t) - b->getResponse(Point(p.x, p.y+1), t) ) /4.0;

  Mat h3d(3,3,CV_64F);

  h3d.at<double>(0,0) = dxx;
  h3d.at<double>(0,1) = dxy;
  h3d.at<double>(0,2) = dxs;
  h3d.at<double>(1,0) = dxy;
  h3d.at<double>(1,1) = dyy;
  h3d.at<double>(1,2) = dys;
  h3d.at<double>(2,0) = dxs;
  h3d.at<double>(2,1) = dys;
  h3d.at<double>(2,2) = dss;

  return h3d;
}

Mat FastHessian::deriv3D(Point p, ResponseLayer *b, ResponseLayer *m, ResponseLayer *t)
{
  Mat d3d(3, 1, CV_64F);
  d3d.at<double>(0,0) = (m->getResponse(Point(p.x+1, p.y), t) -
                        m->getResponse(Point(p.x-1, p.y), t))/2.0f;
  d3d.at<double>(1,0) = (m->getResponse(Point(p.x, p.y+1), t) -
                        m->getResponse(Point(p.x, p.y+1), t))/2.0f;
  d3d.at<double>(2,0) = (t->getResponse(p) - b->getResponse(p, t))/2.0f;
  return d3d;
}

/**
 * Compute the filter in the Y direction.
 */
inline float FastHessian::filterY(Point p, int lobe)
{
  // The long edge of the lobe is 2*lobe-1. The x left edge is half
  // this distance from p (integer division, so floored).
  int x = p.x-(2*lobe-1)/2;

  // Take the value of the area of the three lobes and subtract three
  // times the middle lobe to get the right weights as per the
  // article.
  return m_integral->area(Point(x, p.y-(lobe-1)/2-lobe), 2*lobe-1, lobe*3)
       - m_integral->area(Point(x, p.y-(lobe-1)/2), 2*lobe-1, lobe)*3;
}

/**
 * Compute the filter in the X direction. Corresponding to the Y
 * direction.
 */
inline float FastHessian::filterX(Point p, int lobe)
{
  int y = p.y-(2*lobe-1)/2;

  return m_integral->area(Point(p.x-(lobe-1)/2-lobe, y), lobe*3, 2*lobe-1)
       - m_integral->area(Point(p.x-(lobe-1)/2, y), lobe, 2*lobe-1)*3;
}

/**
 * Compute the filter in the XY direction.
 */
inline float FastHessian::filterXY(Point p, int lobe)
{
  return m_integral->area(Point(p.x-lobe, p.y-lobe), lobe, lobe)
    + m_integral->area(Point(p.x+1, p.y+1), lobe, lobe)
    - m_integral->area(Point(p.x-lobe, p.y+1), lobe, lobe)
    - m_integral->area(Point(p.x+1, p.y-lobe), lobe, lobe);
}

