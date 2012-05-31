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
  Q_PROPERTY(float FocalLength READ focalLength WRITE setFocalLength USER true)
  Q_PROPERTY(bool UseColour READ getuses_colour WRITE setuses_colour USER true)

public:
    enum Side {LEFT, RIGHT};
  RectificationProcessor(QObject *parent = 0);
  ~RectificationProcessor();

  QString name() {return "Rectification";}

  QFileInfo calibrationResults() {QMutexLocker l(&mutex); return calibration_results;}
  void setCalibrationResults(QFileInfo path);

  float focalLength() {QMutexLocker l(&mutex); return focal_length;}
  void setFocalLength(float length);
  
  float getuses_colour() {QMutexLocker l(&mutex); return uses_colour;}
  void setuses_colour(bool yn);

  Point mapPoint(Point p, Side side);

  bool canProcess();

private:
  void run();
  void loadCalibrationResults();
  void calculateRectMatrix();
  void rectify();
  QFileInfo calibration_results;
  float focal_length;
  Mat R;
  Mat T;
  Mat rect;
  int width, height;
  Mat left_rectified, right_rectified;
};

#endif
