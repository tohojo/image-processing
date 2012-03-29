#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <cv.h>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QWaitCondition>

using namespace cv;

class Processor : public QThread
{
  Q_OBJECT

  Q_PROPERTY(QString Name READ name DESIGNABLE true USER true)

public:
  Processor(QObject *parent = 0);
  virtual ~Processor();


  void set_input(const Mat img);
  Mat get_output();

  virtual QString name() {return "Processor";}

public slots:
  void process();

signals:
  void updated();
  void progress(int value) const;

protected:
  virtual void run();
  QMutex mutex;
  QWaitCondition condition;
  Mat input_image;
  Mat output_image;
  bool abort;
  bool restart;
};

#endif
