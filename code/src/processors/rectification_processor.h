#ifndef RECTIFICATION_PROCESSOR_H
#define RECTIFICATION_PROCESSOR_H

#include "two_image_processor.h"

using namespace cv;

class RectificationProcessor : public TwoImageProcessor
{
  Q_OBJECT
  Q_PROPERTY(QFileInfo CalibrationResults READ calibrationResults
             WRITE setCalibrationResults USER true)
  Q_CLASSINFO("CalibrationResults", "filetype=text;")

public:
  RectificationProcessor(QObject *parent = 0);
  ~RectificationProcessor();

  QString name() {return "Rectification";}

  QFileInfo calibrationResults() {QMutexLocker l(&mutex); return calibration_results;}
  void setCalibrationResults(QFileInfo path);


private:
  void run();
  void loadCalibrationResults();
  QFileInfo calibration_results;
  Mat T;
  Mat R;
};

#endif
