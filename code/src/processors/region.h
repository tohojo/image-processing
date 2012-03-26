/**
 * region.h
 *
 * Toke Høiland-Jørgensen
 * 2012-03-25
 *
 * A class to represent a region (which may be arbitrarily shaped).
 *
 * The idea is to represent a region as a set of points, as well as a
 * rectangular bound to make answering membership queries and
 * adjacency queries faster.
 *
 * The region can be constructed from an OpenCV Mat object, either
 * using the whole matrix as one contiguous region, or by using the
 * matrix as a mask (where non-zero elements constitute enabled
 * points).
 */

#ifndef REGION_H
#define REGION_H

#include "rpoint.h"
#include <cv.h>
#include <QtCore/QMap>

using namespace cv;

class Region
{
public:
  Region();
  Region(const Mat &m, bool mask = false);
  Region(const Region &r);
  ~Region();

  bool adjacentTo(const Region &other) const;
  Mat toMask();
  void add(const Mat &m, bool mask = false);
  void add(const Region &other);
  void add(RPoint p);

  bool contains(const RPoint p) const;

private:
  bool adjacentPoint(const RPoint p, const Region &other) const;
  RPoint bound_min;
  RPoint bound_max;
  QMap<RPoint, char> points;
};
#endif
