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
          points.insert(pt, 0);
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
        points.insert(RPoint(i,j), 0);
      }
    }
  }

  if(min < bound_min) bound_min = min;
  if(bound_max < max) bound_max = max;
}

void Region::add(const Region &other)
{
  points.unite(other.points);
  if(other.bound_min < bound_min) bound_min = other.bound_min;
  if(bound_max < other.bound_max) bound_max = other.bound_max;
}

void Region::add(RPoint p)
{
  points.insert(p, 0);
  if(p < bound_min) bound_min = p;
  if(bound_max < p) bound_max = p;
}

Mat Region::toMask()
{
  // Create a new matrix large enough to hold the rectangle up to the
  // max of the bounds.
  Mat m = Mat::zeros(bound_max.y()-1, bound_max.x()-1, CV_8UC1);
  QMap<RPoint, char>::iterator i;
  for(i = points.begin(); i != points.end(); ++i) {
    m.at<uchar>(i.key().x(), i.key().y()) = 255;
  }

  return m;
}

/**
 * Check whether another region is adjacent to this one.
 *
 * Region b is adjacent to region a if, for at least one point p in a,
 * b contains a point that is a 4-neighbour of p. This definition
 * unfortunately makes it nontrivial to check adjacency. The strategy
 * is as follows:
 *
 * 1. If the bounding boxes of the regions are entirely disjoint,
 * return false immediately.
 *
 * 2. Otherwise, go through each possible x coordinate in the region
 * and find the maximum and minimum y coordinates for this x
 * coordinate. Check each neighbour of these points for membership in
 * the other region. Return true as soon as a match is found.
 *
 * 3. If still not match is found, check the minimum and maximum x
 * coordinates and check if any points with (x_min-1) respectively
 * (x_max+1) exist in the other region. If they do, check these for
 * adjacency, and return true if they are.
 *
 * 4. If the above steps do not find an adjacency, non exists, and so
 * return false.
 *
 * This relies on the fact that the points in the region are sorted
 * (by the operator< of RPoint), and so it is easy to get at the
 * points closest to a specific point. Most of this comes from the use
 * of QMap.
 **/
bool Region::adjacentTo(const Region &other) const
{
  // If the bounding rectangles are entirely disjoint, the regions
  // cannot be adjacent. Note that a < comparison on the points
  // themselves are not enough, because two points can have equal x
  // coordinates and still be < each other.
  //
  // Return false immediately as an optimisation.
  if((other.bound_max.x() < bound_min.x() && other.bound_max.y() < bound_min.y()) ||
     (bound_max.x() < other.bound_min.x() && bound_max.y() < other.bound_min.y()))
    return false;

  

  return false;
}

bool Region::contains(const RPoint p) const
{
  if(p < bound_min || bound_max < p) return false;
  return points.contains(p);
}
