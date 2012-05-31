#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <cv.h>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QWaitCondition>
#include <QtCore/QPoint>
#include <QtCore/QFileInfo>

using namespace cv;

class Processor : public QThread
{
  Q_OBJECT

  Q_PROPERTY(QString Name READ name DESIGNABLE true USER true)
  Q_PROPERTY(int POICount READ poiCount USER true NOTIFY poiCountUpdated)

  Q_PROPERTY(QFileInfo ImageOutput READ imageOutput WRITE setImageOutput USER true)
  Q_CLASSINFO("ImageOutput", "filetype=images;opentype=WRITE;")

public:
  Processor(QObject *parent = 0);
  virtual ~Processor();

  virtual void set_input(const Mat img);
  void set_input_name(QString filename);
  Mat get_output();

  QFileInfo imageOutput();
  void setImageOutput(QFileInfo path);

  virtual QString name() {return "Processor";}
  int poiCount() {QMutexLocker l(&mutex); return POIs.size();}
  QList<Point> getPOIs() { QMutexLocker l(&mutex); return POIs; }

  void setPropertiesFrom(Processor *other);

public slots:
  void process();
  void run_once();
  void cancel();
  virtual void saveOutput();
  virtual void addPOI(QPoint);
  virtual void deletePOI(QPoint);
  virtual void left() {}
  virtual void right() {}

signals:
  void updated();
  void progress(int value) const;
  void newMessage(QString msg) const;
  void newPOI(QPoint);
  void clearPOIs();
  void poiCountUpdated();

protected:
  void addPropertiesFrom(Processor *other);
  bool canProcess() {return true;}
  virtual void run();
  QMutex mutex;
  QWaitCondition condition;
  Mat input_image;
  Mat output_image;
  bool abort;
  bool restart;
  bool once;
  QList<Point> POIs;
  QString input_image_filename;
  QFileInfo image_output_file;
  bool uses_colour;
};

#endif
