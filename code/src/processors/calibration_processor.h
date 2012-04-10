#ifndef CALIBRATION_PROCESSOR_H
#define CALIBRATION_PROCESSOR_H

#include <QtCore/QFileInfo>
#include "processor.h"
#include "region.h"
#include "rpoint.h"

using namespace cv;
namespace IP = ImageProcessing;

class CalibrationProcessor : public Processor
{
  Q_OBJECT

  Q_PROPERTY(QFileInfo Points3d READ points3d WRITE setPoints3d USER true)

public:
  CalibrationProcessor(QObject *parent = 0);
  ~CalibrationProcessor();

  QString name() {return "Calibration";}

  QFileInfo points3d() {QMutexLocker l(&mutex); return m_points3d;}
  void setPoints3d(QFileInfo f);

private:
  void run();
  QFileInfo m_points3d;
};

#endif
