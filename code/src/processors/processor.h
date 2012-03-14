#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <cv.h>
#include <QtCore/QObject>

using namespace cv;

class Processor : public QObject
{
  Q_OBJECT

public:
  Processor(QObject *parent = 0) : QObject(parent) {};
  virtual ~Processor() {};


  void set_input(Mat img) {input_image = img;}
  Mat get_output() {return output_image;}

public slots:
  virtual void process() {};

signals:
  void updated();

protected:
  Mat input_image;
  Mat output_image;
};

#endif
