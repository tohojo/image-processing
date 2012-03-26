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
 * they cannot be adjacent, and so return false immediately.
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
 * 4. If the above steps do not find an adjacency, none exists, and so
 * return false.
 *
 * This relies on the fact that the points in the region are sorted
 * (by the operator< of RPoint), and so it is easy to get at the
 * points closest to a specific point. Most of this comes from the use
 * of QMap.
 *
 * TODO: This case:
 * Maybe it *is* better to make a pixel matrix?
 *
 * +-+-+
 * |a|a|
 * +-+-+
 * |a|
 * +-+-+
 * |a|b|
 * +-+-+
 * |a|
 * +-+
 **/
bool Region::adjacentTo(const Region &other) const
{
  // We'll be using this for iterating over points in the region.
  QMap<RPoint, char>::const_iterator i; RPoint p;

  // Step 1. Check if regions are entirely disjoint.
  //
  // Note that a < comparison on the points themselves are not enough,
  // because two points can have equal x coordinates and still be <
  // each other.
  if((other.bound_max.x() < bound_min.x() && other.bound_max.y() < bound_min.y()) ||
     (bound_max.x() < other.bound_min.x() && bound_max.y() < other.bound_min.y()))
    return false;

  // Step 2. Check the min and max y coordinates for each x coordinate
  // in the region.
  QList<RPoint> possibleNeighbours;
  for(i = points.constBegin(); i != points.constEnd(); ) {
    RPoint minY = i.key(); // The first point for a given x coordinate
    RPoint nextX(minY.x()+1, 0);
    i = points.lowerBound(nextX); // If not found this will be
                                  // points.constEnd(), ending the
                                  // loop
    RPoint maxY = (i-1).key();
    if(adjacentPoint(minY, other) || adjacentPoint(maxY, other)) return true;
  }

  // Step 3. Check if any points with (x_min-1) or (x_max+1) exists in
  // the other region. If so, check all points with x_min and x_max
  // coordinates.

  // x_min
  if(other.bound_max.x() >= bound_min.x()-1) {
    // Start at the beginning, keep looping while the x coordinate
    // stays the same. The RPoint variable p is used to cut down on
    // the number of method calls.
    for(i = points.constBegin(), p = i.key();
        p.x() == bound_min.x() && i != points.constEnd();
        p = (++i).key()) {
      if(adjacentPoint(i.key(), other)) return true;
    }
  }

  // x_max
  if(other.bound_min.x() <= bound_max.x()+1) {
    for(i = points.constEnd(), p = i.key();
        p.x() == bound_max.x() && i != points.constBegin();
        p = (--i).key()) {
      if(adjacentPoint(p, other)) return true;
    }
  }

  return false;
}

bool Region::adjacentPoint(const RPoint p, const Region &other) const
{
  return (other.contains(RPoint(p.x()-1, p.y())) ||
          other.contains(RPoint(p.x()+1, p.y())) ||
          other.contains(RPoint(p.x(), p.y()-1)) ||
          other.contains(RPoint(p.x(), p.y()+1)));
}

bool Region::contains(const RPoint p) const
{
  if(p < bound_min || bound_max < p) return false;
  return points.contains(p);
}
