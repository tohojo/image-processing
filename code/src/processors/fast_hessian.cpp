/**
 * fast_hessian.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-06
 */

#include "fast_hessian.h"

FastHessian::FastHessian(Mat &img, int octaves, int intervals, float threshold)
{
  m_img = img;
  m_octaves = octaves;
  m_intervals = intervals;
  m_threshold = threshold;
  m_integral = new IntegralImage(img);
  m_scales = new Mat[octaves*intervals];
  for(int i = 0; i < octaves*intervals; i++) {
    m_scales[i] = Mat::zeros(img.rows, img.cols, CV_32F);
  }
}

FastHessian::~FastHessian()
{
  delete m_integral;
  delete [] m_scales;
}

void FastHessian::compute()
{
  if(m_computed) return;

  // formula for filter size: 3(2^octave*interval+1) - from opensurf pg 15

  int o,i,x,y;

  for(o = 0; o < m_octaves; o++) {
    int border = ((3 * pow(2, o+1)*(m_intervals)+1)+1)/2;
    for(i = 0; i < m_intervals; i++) {
      int lobe_s = pow(2,o+1)*(i+1)+1;
      int lobe_l = pow(2,o+1)*2*(i+1)+1;

      int area = 9*lobe_s*lobe_s; // side length = 3*lobe short

      for(y = border; y<m_img.rows -border; y++) {
        for(x = border; x<m_img.cols-border; x++) {
          Point p(x,y);
          float Dxx = filterX(p, lobe_s, lobe_l) / area;
          float Dyy = filterY(p, lobe_s, lobe_l) / area;
          float Dxy = filterXY(p, lobe_s) / area;

          float detHess = (Dxx*Dyy - 0.9f*0.9f*Dxy*Dxy);
          m_scales[o*m_intervals+i].at<float>(p) = detHess;
        }
      }
    }
  }

  // compute maxima
  for(i = 1; i < m_octaves*m_intervals-1; i++) {
    int border = ((3 * pow(2, i/m_intervals+1)*(i%m_intervals)+1)+1)/2;
    for(y = border; y<m_img.rows -border; y++) {
      for(x = border; x<m_img.cols-border; x++) {
        Point pt(x,y);
        if(maximal(pt, i)) addPoint(pt, i);
      }
    }
  }
  m_computed = true;
}

/**
 * Check whether a point is maximal, given its coordinates and the
 * scale space plane it is located in.
 *
 * Checks all neighbouring pixels in this, the previous and next
 * layers. If any have greater values, return false, otherwise return
 * true. Also returns false if the value is less than the threshold.
 */
bool FastHessian::maximal(Point pt, int i)
{
  if(i < 1 || i > m_octaves * m_intervals -1) return false;
  bool maximal = true;
  float val = m_scales[i].at<float>(pt);
  if(val < m_threshold) return false;
  Mat cur = m_scales[i];
  Mat prev = m_scales[i-1];
  Mat next = m_scales[i+1];
  for(int j = 0; j < 9; j++) {
    Point p(pt.x+j%3-1, pt.y+j/3-1);
    if(prev.at<float>(p) > val) { maximal = false; break;}
    if(next.at<float>(p) > val) { maximal = false; break;}
    if(j !=4 && cur.at<float>(p) > val) { maximal = false; break;}
  }
  return maximal;
}

/**
 * Interpolate real point position and add it to the interesting points if it converges.
 *
 * TODO: Implement interpolation.
 */
void FastHessian::addPoint(Point pt, int /*i*/)
{
  m_ipoints.append(pt);
}

/**
 * Compute the filter in the Y direction.
 */
float FastHessian::filterY(Point p, int size_s, int size_l)
{
  int x_l = p.x-(size_l-1)/2-1;
  int x_r = p.x+(size_l-1)/2;
  int y_t = p.y-(size_s-1)/2-size_s-1;
  int y_b = p.y+(size_s-1)/2+size_s;
  float filter_top = m_integral->area(Point(x_l, y_t),
                                      Point(x_r, y_t+size_s));

  float filter_mid = m_integral->area(Point(x_l, p.y-(size_s-1)/2-1),
                                      Point(x_r, p.y+(size_s-1)/2));

  float filter_btm = m_integral->area(Point(x_l, p.y+(size_s-1)/2),
                                      Point(x_r, y_b));

  return filter_top*-1+filter_mid*2+filter_btm*-1;
}

/**
 * Compute the filter in the X direction.
 */
float FastHessian::filterX(Point p, int size_s, int size_l)
{
  int y_t = p.y-(size_l-1)/2-1;
  int y_b = p.y+(size_l-1)/2;
  int x_l = p.x-(size_s-1)/2-size_s-1;
  int x_r = p.x+(size_s-1)/2+size_s;

  float filter_left  = m_integral->area(Point(x_l, y_t),
                                        Point(x_l+size_s, y_b));

  float filter_mid   = m_integral->area(Point(p.x-(size_s-1)/2-1, y_t),
                                        Point(p.x+(size_s-1)/2, y_b));

  float filter_right = m_integral->area(Point(p.x+(size_s-1)/2+1, y_t),
                                        Point(x_r, y_b));

  return filter_left*-1+filter_mid*2+filter_right*-1;
}

/**
 * Compute the filter in the XY direction.
 */
float FastHessian::filterXY(Point p, int size)
{
  float filter_tl = m_integral->area(Point(p.x-size-1, p.y-size-1),
                                     Point(p.x-1, p.y-1));
  float filter_tr = m_integral->area(Point(p.x, p.y-size-1),
                                     Point(p.x+size, p.y-1));
  float filter_bl = m_integral->area(Point(p.x-size-1, p.y),
                                     Point(p.x-1, p.y+size));
  float filter_br = m_integral->area(p,
                                     Point(p.x+size, p.x+size));

  return -1*(filter_tl+filter_br)+ 2*(filter_bl+filter_tr);
}

