#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <cv.h>
#include <QtCore/QObject>

using namespace cv;

class Processor : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString Name READ name DESIGNABLE true USER true)

public:
  Processor(QObject *parent = 0) : QObject(parent) {};
  virtual ~Processor() {};


  void set_input(const Mat img) {input_image = img;}
  const Mat get_output() {return output_image;}

  virtual QString name() {return "Processor";}

public slots:
  virtual void process() {};

signals:
  void updated();
  void progress(int value) const;

protected:
  Mat input_image;
  Mat output_image;
};

#endif
