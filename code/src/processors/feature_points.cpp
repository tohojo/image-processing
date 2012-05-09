/**
 * feature_points.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-04
 */

#include "feature_points.h"
#include "fast_hessian.h"
#include "surflib.h"
#include <QDebug>

#if OPENCV_VERSION == 24
  #include "opencv2/nonfree/features2d.hpp"
#endif


FeaturePoints::FeaturePoints(QObject *parent)
  : Processor(parent)
{
  m_threshold = 10.0;
  // Default values from SURF article, pg 5.
  m_intervals = 4;
  m_octaves = 4;
  m_init_sample = 2;

  m_extractor = SURF;
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

void FeaturePoints::setInitSample(int initSample)
{
  QMutexLocker locker(&mutex);
  if(m_init_sample == initSample) return;
  m_init_sample = initSample;
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

void FeaturePoints::setExtractor(Extractor extractor)
{
  QMutexLocker locker(&mutex);
  if(m_extractor == extractor) return;
  m_extractor = extractor;
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

    if(!isEmpty) {
      switch(m_extractor) {
      case SURF:
        compute();
        break;
      case SURF_OPENCV:
        compute_opencv();
        break;
      case SURF_OPENSURF:
        compute_opensurf();
        break;
      }
    }

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
  Mat input = input_image;
  mutex.unlock();

  FastHessian fh(input, m_octaves, m_intervals, m_init_sample, m_threshold);

  emit progress(10);
  fh.compute();

  std::vector<KeyPoint> pts = fh.interestPoints().toVector().toStdVector();
  qDebug() << "Got" << pts.size() << "interest points.";
  Mat output(input.rows, input.cols, input.type());
  drawKeypoints(input, pts, output);
  mutex.lock();
  output_image = output;
  mutex.unlock();
}

void FeaturePoints::compute_opencv()
{
  emit progress(0);
  mutex.lock();
  Mat input = input_image;
  mutex.unlock();

  cv::SURF filter(m_threshold, m_octaves, m_intervals, false, true);
  std::vector<KeyPoint> kps;
  filter(input, Mat(), kps);
  qDebug() << "Got" << kps.size() << "interest points.";
  emit progress(90);
  Mat output(input.rows, input.cols, input.type());
  drawKeypoints(input, kps, output);
  mutex.lock();
  output_image = output;
  mutex.unlock();
}

void FeaturePoints::compute_opensurf()
{
  emit progress(0);
  mutex.lock();
  Mat input = input_image;
  mutex.unlock();

  IplImage in = input;

  OpenSURF::IpVec ipts;
  OpenSURF::surfDetDes(&in, ipts, true,  m_octaves, m_intervals, m_init_sample, m_threshold);
  qDebug() << "Got" << ipts.size() << "interest points.";
  emit progress(90);

  std::vector<KeyPoint> kps;
  for(unsigned int i = 0; i < ipts.size(); i++) {
    OpenSURF::Ipoint ip = ipts.at(i);
    kps.push_back(KeyPoint(ip.x, ip.y, 1));
  }
  Mat output(input.rows, input.cols, input.type());
  drawKeypoints(input, kps, output);
  mutex.lock();
  output_image = output;
  mutex.unlock();
}
