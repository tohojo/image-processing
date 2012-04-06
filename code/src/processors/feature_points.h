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
  Q_PROPERTY(double Threshold READ threshold WRITE setThreshold USER true)

public:
  FeaturePoints( QObject *parent = 0 );
  ~FeaturePoints();

  QString name() { return "Feature point extraction"; }

  int octaves() {QMutexLocker locker(&mutex); return m_octaves;}
  void setOctaves(const int octaves);

  int intervals() {QMutexLocker locker(&mutex); return m_intervals;}
  void setIntervals(const int intervals);

  double threshold() {QMutexLocker locker(&mutex); return m_threshold;}
  void setThreshold(const double threshold);

private:
  void run();
  void compute();

  int m_octaves;
  int m_intervals;
  double m_threshold;
};

#endif
