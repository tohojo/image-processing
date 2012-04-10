#ifndef CALIBRATION_PROCESSOR_H
#define CALIBRATION_PROCESSOR_H

#include <QtCore/QFileInfo>
#include "processor.h"
#include "region.h"
#include "rpoint.h"
#include <cv.h>

using namespace cv;
namespace IP = ImageProcessing;

class CalibrationProcessor : public Processor
{
  Q_OBJECT

  Q_PROPERTY(QFileInfo Points3d READ points3d WRITE setPoints3d USER true)
  Q_PROPERTY(ProcessingStage Stage READ stage WRITE setStage USER true)
  Q_ENUMS(ProcessingStage)

public:
  enum ProcessingStage { STAGE_1, STAGE_2 };
  CalibrationProcessor(QObject *parent = 0);
  ~CalibrationProcessor();

  QString name() {return "Calibration";}

  QFileInfo points3d() {QMutexLocker l(&mutex); return m_points3d_file;}
  void setPoints3d(QFileInfo f);

  ProcessingStage stage() {QMutexLocker l(&mutex); return m_stage; }
  void setStage(ProcessingStage s);

public slots:
  void addPOI(QPoint);

private:
  void run();
  void loadPoints3d();
  void findPOIs();
  void adjustPOIs();
  void calibrate();
  Point findCentre(Mat img);
  bool poiExists(QPoint p);
  bool parsePoint(QString line, Point3f *p);
  QFileInfo m_points3d_file;
  QList<Point3f> m_points3d;
  ProcessingStage m_stage;

};

#endif
