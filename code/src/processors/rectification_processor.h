#ifndef RECTIFICATION_PROCESSOR_H
#define RECTIFICATION_PROCESSOR_H

#include "processor.h"

using namespace cv;

class RectificationProcessor : public Processor
{
  Q_OBJECT

public:
  RectificationProcessor(QObject *parent = 0);
  ~RectificationProcessor();

  QString name() {return "Rectification";}

private:
  void run();
};

#endif
