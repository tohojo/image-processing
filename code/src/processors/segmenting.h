#ifndef SEGMENTING_H
#define SEGMENTING_H

#include "processor.h"

using namespace cv;

class Segmenting : public Processor
{
  Q_OBJECT

  Q_PROPERTY(Mode SegmentingMode READ mode WRITE setMode DESIGNABLE true USER true)
  Q_PROPERTY(Background BackgroundColour READ background WRITE setBackground DESIGNABLE true USER true)
  Q_PROPERTY(int Threshold READ threshold USER true)
  Q_ENUMS(Mode)
  Q_ENUMS(Background)

public:
  enum Background { DARK, LIGHT };
  enum Mode { GLOBAL_THRESHOLD, ADAPTIVE_THRESHOLD, SPLIT_MERGE };
  Segmenting(QObject *parent = 0);
  ~Segmenting();

  QString name() {return "Segmenting";}

  Background background() const {return m_background;}
  void setBackground(const Background bg);

  Mode mode() const {return m_mode;}
  void setMode(const Mode mode);

  int threshold() const {return m_threshold;}

public slots:
  void process();

private:
  void thresholdSegment(bool adapt);
  void adaptThreshold();
  void splitMerge();
  int m_threshold;
  Background m_background;
  Mode m_mode;
};

#endif
