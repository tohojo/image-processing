/**
 * region.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-03-25
 */

#include "region.h"

Region::Region()
{
}

Region::Region(const Mat &m, bool mask)
{
  add(m, mask);
}

Region::Region(const Region &r)
{
  add(r);
}

Region::~Region()
{
}

void Region::add(const Mat &m, bool mask)
{
  Size s; Point r;
  RPoint min, max;
  // Get the position in the parent matrix if one exists, and use that
  // as an offset for computing the actual points. Allows to use a
  // sub-region matrix to create a matrix from.
  m.locateROI(s,r);
  if(mask) {
    for(int i = 0; i < s.height; i++) {
      const uchar *ptr = m.ptr<uchar>(i);
      for(int j = 0; j < s.width; j++) {
        if(ptr[j]) {
          RPoint pt(j,i);
          if(pt < min) min = pt;
          if(max < pt) max = pt;
          points.insert(pt);
        }
      }
    }
  } else {
    min = RPoint(r.x, r.y);
    max = RPoint(r.x+s.width, r.y+s.height);
    bound_min = RPoint(r.x, r.y);
    bound_max = RPoint(r.x+s.width, r.y+s.height);
    for(int i = 0; i < s.width; i++) {
      for(int j = 0; j < s.height; i++) {
        points.insert(RPoint(i,j));
      }
    }
  }

  if(min < bound_min) bound_min = min;
  if(bound_max < max) bound_max = max;
}

void Region::add(const Region &other)
{
  points += other.points;
  if(other.bound_min < bound_min) bound_min = other.bound_min;
  if(bound_max < other.bound_max) bound_max = other.bound_max;
}

void Region::add(RPoint p)
{
  points.insert(p);
  if(p < bound_min) bound_min = p;
  if(bound_max < p) bound_max = p;
}

Mat Region::toMask()
{
  // Create a new matrix large enough to hold the rectangle up to the
  // max of the bounds.
  Mat m = Mat::zeros(bound_max.y()-1, bound_max.x()-1, CV_8UC1);
  QSet<RPoint>::iterator i;
  for(i = points.begin(); i != points.end(); ++i) {
    m.at<uchar>(i->x(), i->y()) = 255;
  }

  return m;
}

bool Region::adjacentTo(const Region &other) const
{
  if(other.bound_max < bound_min || bound_max < other.bound_min) return false;
  return true;
}
