#ifndef NULL_PROCESSOR_H
#define NULL_PROCESSOR_H

#include "processor.h"

using namespace cv;

class NullProcessor : public Processor
{
  Q_OBJECT

  Q_PROPERTY(QString Name READ name DESIGNABLE true USER true)

public:
  NullProcessor(QObject *parent = 0);
  ~NullProcessor();

  static const QString name() {return "No processing";}

public slots:
  void process();
};

#endif
