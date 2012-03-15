#ifndef ADAPTIVE_SEGMENT_H
#define ADAPTIVE_SEGMENT_H

#include "processor.h"

using namespace cv;

class AdaptiveSegment : public Processor
{
  Q_OBJECT

  Q_PROPERTY(bool AdaptThreshold READ adapt WRITE setAdapt DESIGNABLE true USER true)
  Q_PROPERTY(Background BackgroundColour READ background WRITE setBackground DESIGNABLE true USER true)
  Q_PROPERTY(int Threshold READ threshold USER true)
  Q_ENUMS(Background)

public:
  enum Background { DARK, LIGHT };
  AdaptiveSegment(QObject *parent = 0);
  ~AdaptiveSegment();

  QString name() {return "Adaptive segmenting";}

  bool adapt() const {return m_adapt;}
  void setAdapt(const bool adapt);

  Background background() const {return m_background;}
  void setBackground(const Background bg);

  int threshold() const {return m_threshold;}

public slots:
  void process();

private:
  void adaptThreshold();
  bool m_adapt;
  int m_threshold;
  Background m_background;
};

#endif
