/**
 * region.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-03-25
 */

#include "region.h"
#include <stdio.h>

using namespace ImageProcessing;

Region::Region()
{
}

Region::Region(const Mat &m)
{
  Size s; Point p;
  // Get the position in the parent matrix if one exists, and use that
  // as an offset for computing the actual points. Allows to use a
  // sub-region matrix to create a matrix from.
  m.locateROI(s,p);
  s = m.size();

  // We don't want reallocations as we're adding stuff, so reserve
  // space for a rectangle going all the way around the region.
  points.reserve((s.width+s.height)*2);
  bound_min = RPoint(p.x, p.y);
  bound_max = RPoint(p.x+s.width-1, p.y+s.height-1);

  // do first column, then middle ones, then last column, so
  // preserve sorted order.
  int i;
  for(i = 0; i < s.width; i++) {
    points.insert(RPoint(p.x+i,p.y));
  }
  for(i = 1; i < s.height-1; i++) {
    points.insert(RPoint(p.x, p.y+i));
    points.insert(RPoint(p.x+s.width-1, p.y+i));
  }
  for(i = 0; i < s.width; i++) {
    points.insert(RPoint(p.x+i,p.y+s.height-1));
  }
}

Region::Region(const Region &r)
{
  bound_min = RPoint(r.bound_min);
  bound_max = RPoint(r.bound_max);
  points = QSet<RPoint>(r.points);
}

Region::~Region()
{
}

Region& Region::operator=(const Region &other)
{
  bound_min = RPoint(other.bound_min);
  bound_max = RPoint(other.bound_max);
  points = QSet<RPoint>(other.points);
  return *this;
}

void Region::add(const Region &other)
{
  if(other.isEmpty()) return;
  if(isEmpty()) {
    operator=(other);
    return;
  }
  if(!adjacentTo(other)) {
    qWarning("Attempt to add non-adjacent region");
    return;
  }
  if(contains(other) ) {
    qWarning("Attempt to add contained region");
    return;
  }
  if (other.contains(*this)) {
    qWarning("Attempt to add containing region");
    return;
  }
  // Keep track of points that might become interior, and check them
  // afterwards.
  QSet<RPoint>::const_iterator i;
  QList<RPoint> checkPoints;
  QList<RPoint>::const_iterator j;
  checkPoints.reserve(points.size()+other.points.size());

  // Go through each region and find the points that are adjacent to
  // the other region; i.e. the points that are on the shared border.
  // There are the points that may become interior points after the
  // merge, so we need to check them when we have merged the regions.
  for(i = other.points.constBegin(); i != other.points.constEnd(); ++i) {
    if(adjacentPoint(*i)) checkPoints.append(*i);
  }
  for(i = points.constBegin(); i != points.constEnd(); ++i) {
    if(other.adjacentPoint(*i)) checkPoints.append(*i);
  }

  for(i = other.points.constBegin(); i != other.points.constEnd(); ++i) {
    insert(*i);
  }

  for(j = checkPoints.constBegin(); j != checkPoints.constEnd(); ++j) {
    removeInterior(*j);
  }
}

bool Region::isEmpty() const
{
  return points.size() == 0;
}

Mat Region::toMask(Mat img) const
{
  // Create a new matrix large enough to hold the rectangle up to the
  // max of the bounds.
  Size s = img.size();
  Mat m = Mat::zeros(s, img.type());
  // Build up the array line by line, by going through all possible
  // points in the bounding rectangle. Keep an array that for each
  // x-value keeps track of whether or not, on the last row, this
  // x-value was in the bounding set, and whether or not this x-value
  // is currently inside the region.

  char *xmap = new char[bound_max.x()-bound_min.x()+1];
  for(int i = bound_min.y(); i <= bound_max.y(); i++) {
    for(int j = bound_min.x(); j <= bound_max.x(); j++) {
      int idx = j-bound_min.x(); // index for xmap
      if(i == bound_min.y()) xmap[idx]=0; // init vars
      RPoint p(j,i);
      if(inBoundary(p)) {
        xmap[idx] |= 1;
      } else {
        if((xmap[idx] & 1) && contains(p)) {
          xmap[idx] |= 2;
        }
        xmap[idx] &= (~1);
      }
      if(xmap[idx] & 3) m.at<uchar>(i, j) = 255;
    }
  }
  delete[] xmap;

  return m;
}

/**
 * Check whether another region is adjacent to this one.
 *
 * Region b is adjacent to region a if, for at least one point p in
 * the boundary of a, the boundary of b contains a point that is a
 * 4-neighbour of p. Since regions contain only boundary points, it is
 * straight-forward to check for this.
 *
 * As an optimisation, if the bounding regions are entirely disjoint,
 * to not check all the points.
 **/
bool Region::adjacentTo(const Region &other) const
{
  // Step 1. Check if regions are entirely disjoint.
  //
  // Note that a < comparison on the points themselves are not enough,
  // because two points can have equal x coordinates and still be <
  // each other.
  if((other.bound_max.x() < bound_min.x() && other.bound_max.y() < bound_min.y()) ||
     (bound_max.x() < other.bound_min.x() && bound_max.y() < other.bound_min.y()))
    return false;

  QSet<RPoint>::const_iterator i;
  for(i = points.constBegin(); i != points.constEnd(); i++) {
    if(other.adjacentPoint(*i)) return true;
  }
  return false;
}

bool Region::adjacentPoint(const RPoint p) const
{
  return (inBoundary(p+RPoint(-1, 0)) ||
          inBoundary(p+RPoint(1, 0)) ||
          inBoundary(p+RPoint(0, -1)) ||
          inBoundary(p+RPoint(0, 1)));
}

int Region::boundSize() const
{
  return (bound_max.x()-bound_min.x())*(bound_max.y()-bound_min.y());
}

/**
 * Checks whether a given point is contained in the region.
 *
 * A point is in the region if it is in the boundary or the interior.
 * As an optimisation, check if the point is entirely outside the
 * bounding box first.
 *
 */
bool Region::contains(const RPoint p) const
{
  return inBoundary(p) || interior(p);
}

bool Region::contains(const Region &other) const
{
  if(!(other.bound_max.x() <= bound_max.x() &&
       other.bound_max.y() <= bound_max.y() &&
       other.bound_min.x() >= bound_min.x() &&
       other.bound_min.y() >= bound_min.y())) return false;
  foreach(RPoint p, other.points)
    if(!contains(p)) return false;
  return true;
}

/**
 * Checks whether a given point is part of the bounding set.
 *
 * This uses the sorted nature of the points to do a smarter lookup
 * than a naive points.contains().
 */
bool Region::inBoundary(const RPoint p) const
{
  return points.contains(p);
}

/**
 * Check if a point is in the interior of the region.
 *
 * Try extending a line from the points in each x and y direction.
 * These lines each has to hit a point in the boundary set. If they do
 * not (i.e. the lines cross the bounding rectangle before a match is
 * found), the point is not in the interior.
 *
 * It is important that the search for a point in the boundary does
 * not include the tested point itself, so it can be used to check if
 * a point in the boundary is also in the interior (which can happen
 * when points are added).
 */
bool Region::interior(const RPoint p) const
{
  // keep track of each direction
  bool x_plus = false, x_minus = false, y_plus = false, y_minus = false;

  // Loop until we've found a match in each direction
  for(int i = 1; !x_plus || !x_minus || !y_plus || !y_minus; i++) {
    if(!x_plus && inBoundary(p+RPoint(i,0))) x_plus = true;
    if(!x_minus && inBoundary(p+RPoint(-i,0))) x_minus = true;
    if(!y_plus && inBoundary(p+RPoint(0,i))) y_plus = true;
    if(!y_minus && inBoundary(p+RPoint(0,-i))) y_minus = true;

    // If we moved outside the bounding box, the point is not in the
    // interior.
    if(p.x()-i < bound_min.x() && p.x()+i > bound_max.x() &&
       p.y()-i < bound_min.y() && p.y()+i > bound_max.y()) return false;
  }
  return true;
}

/**
 * Insert point into region, preserving ordering of points list, and y
 * coord map.
 */
void Region::insert(RPoint p)
{
  points.insert(p);
  updateBounds(p);
}

void Region::updateBounds(RPoint p)
{
  if(p.x() < bound_min.x()) bound_min = RPoint(p.x(), bound_min.y());
  if(p.y() < bound_min.y()) bound_min = RPoint(bound_min.x(), p.y());
  if(p.x() > bound_max.x()) bound_max = RPoint(p.x(), bound_max.y());
  if(p.y() > bound_max.y()) bound_max = RPoint(bound_max.x(), p.y());
}

/**
 * Remove a point that is both in the interior and the boundary from
 * the boundary.
 */
void Region::removeInterior(RPoint p)
{
  if(inBoundary(p) && interior(p)) {
    points.remove(p);
  }
}

void Region::print() const
{
  std::cout << "Min/max bound: ";
  bound_min.print();
  std::cout << "/";
  bound_max.print();
  std::cout << "\n";
}
