#ifndef NULL_PROCESSOR_H
#define NULL_PROCESSOR_H

#include "processor.h"

using namespace cv;

class NullProcessor : public Processor
{
  Q_OBJECT

public:
  NullProcessor(QObject *parent);
  ~NullProcessor();

public slots:
  void process();
};

#endif
