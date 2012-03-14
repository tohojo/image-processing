#ifndef ADAPTIVE_SEGMENT_H
#define ADAPTIVE_SEGMENT_H

#include "processor.h"

using namespace cv;

class AdaptiveSegment : public Processor
{
  Q_OBJECT

public:
  AdaptiveSegment(QObject *parent);
  ~AdaptiveSegment();

public slots:
  void process();

private:
  void adaptThreshold(Mat I, int *threshold);
};

#endif
