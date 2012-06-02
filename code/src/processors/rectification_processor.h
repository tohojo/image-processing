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
  Q_PROPERTY(bool TestChessboard READ testChessboard WRITE setTestChessboard USER true)
  Q_PROPERTY(int ChessboardHoriz READ chessboardHoriz WRITE setChessboardHoriz USER true)
  Q_PROPERTY(int ChessboardVert READ chessboardVert WRITE setChessboardVert USER true)

public:
    enum Side {LEFT, RIGHT};
  RectificationProcessor(QObject *parent = 0);
  ~RectificationProcessor();

  QString name() {return "Rectification";}

  QFileInfo calibrationResults() {QMutexLocker l(&mutex); return calibration_results;}
  void setCalibrationResults(QFileInfo path);

  float focalLength() {QMutexLocker l(&mutex); return focal_length;}
  void setFocalLength(float length);

  bool testChessboard() {QMutexLocker l(&mutex); return test_chessboard;}
  void setTestChessboard(bool test);

  int chessboardHoriz() {QMutexLocker l(&mutex); return chessboard_horiz;}
  void setChessboardHoriz(int value);

  int chessboardVert() {QMutexLocker l(&mutex); return chessboard_vert;}
  void setChessboardVert(int value);

  Point mapPoint(Point p, Side side);

  bool canProcess();

private:
  void run();
  void loadCalibrationResults();
  void calculateRectMatrix();
  void rectify();
  void test();
  QFileInfo calibration_results;
  float focal_length;
  Mat R;
  Mat T;
  Mat rect;
  int width, height;
  bool test_chessboard;
  int chessboard_horiz, chessboard_vert;
};

#endif
