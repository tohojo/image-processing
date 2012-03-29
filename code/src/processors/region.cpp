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

Region::Region(const Mat &m, bool mask)
{
  Size s; Point p;
  RPoint min, max;
  // Get the position in the parent matrix if one exists, and use that
  // as an offset for computing the actual points. Allows to use a
  // sub-region matrix to create a matrix from.
  m.locateROI(s,p);
  s = m.size();

  // We don't want reallocations as we're adding stuff, so reserve
  // space for a rectangle going all the way around the region, plus
  // some extra for odd paths.
  points.reserve(qRound((s.width+s.height)*2.5));
  if(mask) {
    for(int i = 0; i < s.height; i++) {
      const uchar *ptr = m.ptr<uchar>(i);
      for(int j = 0; j < s.width; j++) {
        if(ptr[j] && // if the point is set
           // and it is an edge point
           (i == 0 || // edges of regions
            j == 0 ||
            i == s.height-1 ||
            j == s.width-1 ||
            (!ptr[j-1]) || (!ptr[j+1]) || // before or after x-value not set
            (!m.at<int>(j,i-1)) || (!m.at<int>(j,i+1)) // before or after y-value not set
            )
           ) {
          RPoint pt(j+p.x,i+p.y);
          if(isEmpty()) {
            bound_min = pt;
            bound_max = pt;
          } else {
            updateBounds(pt);
          }
          points.append(pt);
        }
      }
    }
  } else {
    bound_min = RPoint(p.x, p.y);
    bound_max = RPoint(p.x+s.width, p.y+s.height);

    // do first column, then middle ones, then last column, so
    // preserve sorted order.
    int i;
    for(i = 0; i < s.width; i++) {
      points.append(RPoint(p.x+i,p.y));
    }
    for(i = 1; i < s.height-1; i++) {
      points.append(RPoint(p.x, p.y+i));
      points.append(RPoint(p.x+s.width-1, p.y+i));
    }
    for(i = 0; i < s.width; i++) {
      points.append(RPoint(p.x+i,p.y+s.height-1));
    }
  }

  buildYMap();
}

Region::Region(const Region &r)
{
  bound_min = RPoint(r.bound_min);
  bound_max = RPoint(r.bound_max);
  points = QList<RPoint>(r.points);
  ycoords = QMap<int, int>(r.ycoords);
}

Region::~Region()
{
}

Region& Region::operator=(const Region &other)
{
  bound_min = RPoint(other.bound_min);
  bound_max = RPoint(other.bound_max);
  points = QList<RPoint>(other.points);
  ycoords = QMap<int, int>(other.ycoords);

  return *this;
}

void Region::add(const Region &other)
{
  if(other.isEmpty()) return;
  if(isEmpty()) {
    operator=(other);
  }
  if(!adjacentTo(other)) return;
  // Keep track of points that might become interior, and check them
  // afterwards.
  QList<RPoint> checkPoints;
  int i;
  checkPoints.reserve(points.size()+other.points.size());

  // Go through each region and find the points that are adjacent to
  // the other region; i.e. the points that are on the shared border.
  // There are the points that may become interior points after the
  // merge, so we need to check them when we have merged the regions.
  for(i = 0; i < other.points.size(); i++) {
    if(adjacentPoint(other.points[i])) checkPoints.append(RPoint(other.points[i]));
  }
  for(i = 0; i < points.size(); i++) {
    if(other.adjacentPoint(points[i])) checkPoints.append(RPoint(points[i]));
  }

  for(i = 0; i < other.points.size(); i++) {
    RPoint p(other.points[i]);
    insert(p);
  }

  for(i = 0; i < checkPoints.size(); i++) {
    removeInterior(checkPoints[i]);
  }
}

/**
 * Add a single point to the region. The point must be adjacent to the
 * region.
 *
 * Works by first adding the point to the boundary, then checking if
 * each of its 8-neighbours has become interior from this addition and
 * removes them if they have.
 */
void Region::add(RPoint p)
{
  p = RPoint(p);
  if(isEmpty()) {
    bound_min = p;
    bound_max = p;
    insert(p);
    return;
  }
  if(contains(p) || !adjacentPoint(p)) return;
  insert(p);
  for(int i = -1; i <= 1; i++)
    for(int j = -1; j <= 1; j++)
      if(i != 0 || j != 0) removeInterior(p+RPoint(i,j));
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
      if(xmap[idx] & 3) m.at<uchar>(j, i) = 255;
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

  for(int i = 0; i < points.size(); i++) {
    if(other.adjacentPoint(points[i])) return true;
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
  if(p < bound_min || bound_max < p) {
    return false;}

  return inBoundary(p) || interior(p);
}

/**
 * Checks whether a given point is part of the bounding set.
 *
 * This uses the sorted nature of the points to do a smarter lookup
 * than a naive points.contains().
 */
bool Region::inBoundary(const RPoint p) const
{
  // If no points with this y coordinate are in the region, this point
  // is not.
  if(!ycoords.contains(p.y())) return false;
  for(int i = ycoords.value(p.y()); i < points.size() && points[i].y() == p.y(); i++) {
    if(points[i] == p) return true;
  }
  return false;
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
 * Build up the map of Y coordinates to points list positions.
 * The map is used for efficient lookup of points.
 */
void Region::buildYMap()
{
  ycoords.clear();
  if(points.size() == 0) return;
  int i, current;
  current = points[0].y();
  ycoords.insert(current, 0);
  for(i = 0; i < points.size(); i++) {
    if(current != points[i].y()) {
      current = points[i].y();
      ycoords.insert(current, i);
    }
  }
}

/**
 * Insert point into region, preserving ordering of points list, and y
 * coord map.
 */
void Region::insert(RPoint p)
{
  QMap<int, int>::iterator i;
  i = ycoords.lowerBound(p.y());
  if(i == ycoords.end()) {
    // p.y() is the new largest y, so insert at end
    ycoords.insert(p.y(), points.size());
    points.append(p);
  } else if (i.key() > p.y()) {
    // No point with this y coordinate exists, but y-value greater
    // than p.y() found, insert just before this.
    int pos = i.value();
    points.insert(pos, p);
    shiftYMap(i, 1);
    ycoords.insert(p.y(), pos);
  } else {
    // Found point(s) with same y-value
    int pos = i.value();
    while(pos < points.size() && points[pos] < p) pos++;
    if(pos == points.size()) {
      points.append(p);
    } else {
      points.insert(pos,p);
      shiftYMap(++i, 1); // Shift only those after this y
    }
  }
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
    QMap<int, int>::iterator i, j;
    i = ycoords.lowerBound(p.y());
    j = i+1;
    int pos = i.value();
    while(points[pos] < p) pos++;
    if(pos == i.value() && (j == ycoords.end() || j.value() == pos+1)) {
      // This point is being pointed to in the YMap, and is the only
      // on with its y coordinate, so remove it from the map.
      //
      // Note that if this happens in the middle of the region, we
      // will probably end up with a disjoint region. We do not try to
      // detect this, or worry about it much here, but instead assumes
      // that regions are being created and extended in a sane way.
      ycoords.erase(i);
    }

    // We remove the point, and shift all positions in the map *after*
    // this y coordinate. This is sufficient, because if the point is
    // being pointed to by the map, removing it will automatically
    // make the pointed valid for the next point with the same y
    // coordinate. If no such next point exists, we already removed
    // this y coordinate from the map as above.
    points.removeAt(pos);
    shiftYMap(j, -1);
  }
}

/**
 * Shift the values of the ymap, starting at an iterator, by a value
 * (defaults to 1). Used after a point is inserted.
 */
void Region::shiftYMap(QMap<int, int>::iterator i, int shift)
{
  for(; i != ycoords.end(); ++i) {
    i.value() += shift;
  }
}

void Region::print()
{
  std::cout << "Min: "; bound_min.print();
  std::cout << " Max: "; bound_max.print();
  std::cout << "\n";
  std::cout << "Points:\n";
  foreach(const RPoint &p, points) {
    std::cout << "  ";
    p.print();
    std::cout << "\n";
  }
  std::cout << "YMap:\n";
  QMap<int, int>::const_iterator i;
  for(i = ycoords.constBegin(); i != ycoords.constEnd(); ++i)
    printf("  %d: %d\n", i.key(), i.value());

}
