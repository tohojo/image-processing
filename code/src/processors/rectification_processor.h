#ifndef RECTIFICATION_PROCESSOR_H
#define RECTIFICATION_PROCESSOR_H

#include "two_image_processor.h"

using namespace cv;

class RectificationProcessor : public TwoImageProcessor
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
