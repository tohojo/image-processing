/**
 * feature_points.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-04
 */

#include "feature_points.h"
#include "fast_hessian.h"

FeaturePoints::FeaturePoints(QObject *parent)
  : Processor(parent)
{
  m_threshold = 10.0;
  // Default values from SURF article, pg 5.
  m_intervals = 4;
  m_octaves = 4;
}

FeaturePoints::~FeaturePoints()
{
}

void FeaturePoints::setOctaves(int octaves)
{
  QMutexLocker locker(&mutex);
  if(m_octaves == octaves) return;
  m_octaves = octaves;
  mutex.unlock();
  process();
}

void FeaturePoints::setIntervals(int intervals)
{
  QMutexLocker locker(&mutex);
  if(m_intervals == intervals) return;
  m_intervals = intervals;
  mutex.unlock();
  process();
}

void FeaturePoints::setThreshold(double threshold)
{
  QMutexLocker locker(&mutex);
  if(m_threshold == threshold) return;
  m_threshold = threshold;
  mutex.unlock();
  process();
}


void FeaturePoints::run()
{
  forever {
    mutex.lock();
    bool isEmpty = input_image.empty();
    if(abort) {
      mutex.unlock();
      return;
    }
    mutex.unlock();

    if(!isEmpty)
      compute();

    mutex.lock();
    if(!restart && !isEmpty) {
      emit progress(100);
      emit updated();
    }
    if(once) {
      mutex.unlock();
      return;
    }

    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}

void FeaturePoints::compute()
{
  emit progress(0);
  mutex.lock();
  FastHessian fh(input_image, m_octaves, m_intervals, m_threshold);
  mutex.unlock();

  emit progress(10);
  fh.compute();

  QList<Point> pts = fh.interestPoints();
  qDebug("Got %d interest points.", pts.size());
  mutex.lock();
  input_image.copyTo(output_image);
  foreach(Point pt, pts) {
    circle(output_image, pt, 5, 255);
  }
  mutex.unlock();
}
