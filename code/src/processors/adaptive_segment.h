#ifndef ADAPTIVE_SEGMENT_H
#define ADAPTIVE_SEGMENT_H

#include "processor.h"

using namespace cv;

class AdaptiveSegment : public Processor
{
  Q_OBJECT

  Q_PROPERTY(QString Name READ name DESIGNABLE true USER true)

  Q_PROPERTY(bool AdaptThreshold READ adapt WRITE setAdapt DESIGNABLE true USER true)
  Q_PROPERTY(Background BackgroundColour READ background WRITE setBackground DESIGNABLE true USER true)
  Q_ENUMS(Background)

public:
  enum Background { DARK, LIGHT };
  AdaptiveSegment(QObject *parent = 0);
  ~AdaptiveSegment();

  static const QString name() {return "Adaptive segmenting";}

  bool adapt() const {return m_adapt;}
  void setAdapt(const bool adapt);

  Background background() const {return m_background;}
  void setBackground(const Background bg);

public slots:
  void process();

private:
  void adaptThreshold(Mat I, int *threshold);
  bool m_adapt;
  Background m_background;
};

#endif
