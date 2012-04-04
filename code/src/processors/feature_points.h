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
  Q_PROPERTY(int Segments READ segments WRITE setSegments USER true)
  Q_PROPERTY(double Threshold READ threshold WRITE setThreshold USER true)

public:
  FeaturePoints( QObject *parent = 0 );
  ~FeaturePoints();

  QString name() { return "Feature point extraction"; }

  int octaves() {QMutexLocker locker(&mutex); return m_octaves;}
  void setOctaves(const int octaves);

  int segments() {QMutexLocker locker(&mutex); return m_segments;}
  void setSegments(const int segments);

  double threshold() {QMutexLocker locker(&mutex); return m_threshold;}
  void setThreshold(const double threshold);

private:
  void run();

  int m_octaves;
  int m_segments;
  double m_threshold;
};

#endif
