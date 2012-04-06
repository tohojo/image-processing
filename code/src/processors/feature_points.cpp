/**
 * feature_points.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-04
 */

#include "feature_points.h"

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
    if(abort) return;
    mutex.lock();
    output_image = input_image;
    emit progress(100);
    emit updated();
    if(once) return;

    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}
