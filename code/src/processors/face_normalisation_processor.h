/**
 * face_normalisation_processor.h
 *
 * Toke Høiland-Jørgensen
 * 2012-05-30
 */

#ifndef FACE_NORMALISATION_PROCESSOR_H
#define FACE_NORMALISATION_PROCESSOR_H

#include "processor.h"

using namespace cv;

class FaceNormalisationProcessor : public Processor
{
  Q_OBJECT
  Q_PROPERTY(QFileInfo FacePoints READ facePoints
             WRITE setFacePoints USER true)
  Q_PROPERTY(bool ReadDir READ readDir WRITE setReadDir USER true)
  Q_PROPERTY(bool UseColour READ useColour WRITE setUseColour USER true)
  Q_PROPERTY(int ShowIndex READ showIndex WRITE setShowIndex USER true NOTIFY showIdxUpdated)
  Q_PROPERTY(float CropX READ cropX WRITE setCropX USER true)
  Q_PROPERTY(float CropY READ cropY WRITE setCropY USER true)
  Q_PROPERTY(int ScaledWidth READ scaledWidth WRITE setScaledWidth USER true)
  Q_PROPERTY(QFileInfo OutputDir READ outputDir
             WRITE setOutputDir USER true)
  Q_PROPERTY(QFileInfo AverageFile READ averageFile
             WRITE setAverageFile USER true)

  Q_CLASSINFO("FacePoints", "filetype=text;")
  Q_CLASSINFO("ShowIndex", "minimum=0;")
  Q_CLASSINFO("CropX", "minimum=0;")
  Q_CLASSINFO("CropY", "minimum=0;")
  Q_CLASSINFO("ScaledWidth", "minimum=1;")
  Q_CLASSINFO("OutputDir", "opentype=DIR;")
  Q_CLASSINFO("AverageFile", "filetype=text;")

public:
  FaceNormalisationProcessor(QObject *parent = 0);
  ~FaceNormalisationProcessor();

  QString name() {return "Face normalisation";}

  QFileInfo facePoints() {QMutexLocker l(&mutex); return face_points;}
  void setFacePoints(QFileInfo path);

  bool readDir() {QMutexLocker l(&mutex); return read_dir;}
  void setReadDir(bool value);

  bool useColour() {QMutexLocker l(&mutex); return uses_colour;}
  void setUseColour(bool value);

  int showIndex() {QMutexLocker l(&mutex); return show_idx;}
  void setShowIndex(int value);

  float cropX() {QMutexLocker l(&mutex); return crop_x;}
  void setCropX(float value);

  float cropY() {QMutexLocker l(&mutex); return crop_y;}
  void setCropY(float value);

  int scaledWidth() {QMutexLocker l(&mutex); return scaled_width;}
  void setScaledWidth(int value);

  QFileInfo outputDir() {QMutexLocker l(&mutex); return output_dir;}
  void setOutputDir(QFileInfo path);

  QFileInfo averageFile() {QMutexLocker l(&mutex); return average_file;}
  void setAverageFile(QFileInfo path);

public slots:
  void left();
  void right();

signals:
  void showIdxUpdated();

private:
  void run();
  void normalise_faces();
  void updateOutput();
  QFileInfo face_points;
  bool read_dir;
  int show_idx;
  float crop_x;
  float crop_y;
  int scaled_width;
  QFileInfo output_dir;
  QFileInfo average_file;
  QList<Mat> normalised_imgs;
};

#endif
