#ifndef SEGMENTING_H
#define SEGMENTING_H

#include <QtCore/QList>
#include "processor.h"
#include "region.h"
#include "rpoint.h"

using namespace cv;
namespace IP = ImageProcessing;

class Segmenting : public Processor
{
  Q_OBJECT

  Q_PROPERTY(Mode SegmentingMode READ mode WRITE setMode DESIGNABLE true USER true)
  Q_PROPERTY(bool DarkBackground READ darkBG WRITE setDarkBG DESIGNABLE true USER true)
  Q_PROPERTY(int Delta READ delta WRITE setDelta USER true)
  Q_PROPERTY(int Threshold READ threshold USER true)
  Q_ENUMS(Mode)

public:
  enum Mode { GLOBAL_THRESHOLD, ADAPTIVE_THRESHOLD, SPLIT_MERGE };
  Segmenting(QObject *parent = 0);
  ~Segmenting();

  QString name() {return "Segmenting";}

  bool darkBG() {QMutexLocker locker(&mutex); return m_dark_bg;}
  void setDarkBG(const bool bg);

  Mode mode() {QMutexLocker locker(&mutex); return m_mode;}
  void setMode(const Mode mode);

  int threshold() {QMutexLocker locker(&mutex); return m_threshold;}

  int delta() {QMutexLocker locker(&mutex); return m_delta;}
  void setDelta(const int delta);

private:
  void run();
  void thresholdSegment(bool adapt);
  void adaptThreshold();
  void splitMerge();
  QList<IP::Region> splitRegions(Mat image, bool topLevel = false) const;
  QList<IP::Region> mergeRegions(QList<IP::Region> regions, Mat img) const;
  QList<IP::Region> filterRegions(QList<IP::Region> regions, Rect rect) const;
  void colourRegions(QList<IP::Region> regions, Mat img) const;
  bool isHomogeneous(const IP::Region region, const Mat img) const;
  bool isHomogeneous(const Mat mat) const;
  int m_threshold;
  bool m_dark_bg;
  Mode m_mode;
  int m_delta;
};

#endif
