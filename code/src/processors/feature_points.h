/**
 * feature_points.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-04
 */

#ifndef FEATURE_POINTS_H
#define FEATURE_POINTS_H
#include "processor.h"

using namespace cv;

class FeaturePoints: public Processor
{
  Q_OBJECT

  Q_PROPERTY(int Octaves READ octaves WRITE setOctaves USER true)
  Q_PROPERTY(int Intervals READ intervals WRITE setIntervals USER true)
  Q_PROPERTY(int InitSample READ initSample WRITE setInitSample USER true)
  Q_PROPERTY(double Threshold READ threshold WRITE setThreshold USER true)
  Q_PROPERTY(Extractor Extractor READ extractor WRITE setExtractor USER true)
  Q_ENUMS(Extractor)

public:
    enum Extractor { SURF, SURF_OPENSURF, SURF_OPENCV };
  FeaturePoints( QObject *parent = 0 );
  ~FeaturePoints();

  QString name() { return "Feature point extraction"; }

  int octaves() {QMutexLocker locker(&mutex); return m_octaves;}
  void setOctaves(const int octaves);

  int intervals() {QMutexLocker locker(&mutex); return m_intervals;}
  void setIntervals(const int intervals);

  int initSample() {QMutexLocker locker(&mutex); return m_init_sample;}
  void setInitSample(const int init_sample);

  double threshold() {QMutexLocker locker(&mutex); return m_threshold;}
  void setThreshold(const double threshold);

  Extractor extractor() {QMutexLocker locker(&mutex); return m_extractor;}
  void setExtractor(Extractor extractor);

private:
  void run();
  void compute();
  void compute_opencv();
  void compute_opensurf();

  int m_octaves;
  int m_intervals;
  int m_init_sample;
  double m_threshold;

  Extractor m_extractor;
};

#endif
