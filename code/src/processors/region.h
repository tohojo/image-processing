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
#include <QtCore/QSet>

using namespace cv;

namespace ImageProcessing {

  /**
   * Class to represent a region of an image.
   *
   * Stores the points representing the outline of the region, i.e.
   * all points that have a (4-way) neighbouring point that is not
   * inside the region.
   *
   * This representation of regions only works for convex regions, and
   * so breaks down in quite a few cases (see region-tests for an
   * example). A better approach would have to represent all points in
   * a region (probably in an opencv matrix).
   **/
  class Region
  {
  public:
    Region();
    Region(const Mat &m);
    Region(const Region &r);
    ~Region();

    Region& operator=(const Region& other);

    bool adjacentPoint(const RPoint p) const;
    bool adjacentTo(const Region &other) const;
    Mat toMask(Mat img) const;
    void add(const Mat &m, bool mask = false);
    void add(const Region &other);

    bool isEmpty() const;

    bool contains(const Region &other) const;
    bool contains(const RPoint p) const;
    bool inBoundary(const RPoint p) const;
    bool interior(const RPoint p) const;

    int boundSize() const;

    RPoint minBound() const {return bound_min;}

    void print() const;

  private:
    void insert(RPoint p);
    void removeInterior(RPoint p);
    void updateBounds(RPoint p);
    RPoint bound_min;
    RPoint bound_max;
    QSet<RPoint> points;
  };

}
#endif
