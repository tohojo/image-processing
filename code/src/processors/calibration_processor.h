#ifndef CALIBRATION_PROCESSOR_H
#define CALIBRATION_PROCESSOR_H

#include <QtCore/QFileInfo>
#include "processor.h"
#include "camera_calibrator.h"
#include <cv.h>

using namespace cv;

class CalibrationProcessor : public Processor
{
  Q_OBJECT

  Q_PROPERTY(QFileInfo Points3d READ points3d WRITE setPoints3d USER true)
  Q_PROPERTY(ProcessingStage Stage READ stage WRITE setStage USER true)
  Q_PROPERTY(double FeatureThreshold READ threshold WRITE setThreshold USER true)
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

  double threshold() {QMutexLocker locker(&mutex); return m_threshold;}
  void setThreshold(const double threshold);

public slots:
  void addPOI(QPoint);
  void deletePOI(QPoint);

private:
  void run();
  void loadPoints3d();
  void findPOIs();
  void adjustPOIs();
  void calibrate();
  Point findCentre(Mat img);
  bool poiExists(QPoint p);
  bool parsePoint(QString line, Point3d *p);
  QFileInfo m_points3d_file;
  QList<Point3d> m_points3d;
  QVector<point_correspondence> m_corr;
  ProcessingStage m_stage;
  double m_threshold;

};

#endif
