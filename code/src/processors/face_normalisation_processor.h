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
  Q_CLASSINFO("FacePoints", "filetype=text;")
  Q_CLASSINFO("ShowIndex", "minimum=0;")
  Q_PROPERTY(bool ReadDir READ readDir WRITE setReadDir USER true)
  Q_PROPERTY(int ShowIndex READ showIndex WRITE setShowIndex USER true)

public:
  FaceNormalisationProcessor(QObject *parent = 0);
  ~FaceNormalisationProcessor();

  QString name() {return "Face normalisation";}

  QFileInfo facePoints() {QMutexLocker l(&mutex); return face_points;}
  void setFacePoints(QFileInfo path);

  bool readDir() {QMutexLocker l(&mutex); return read_dir;}
  void setReadDir(bool value);

  int showIndex() {QMutexLocker l(&mutex); return show_idx;}
  void setShowIndex(int value);

private:
  void run();
  void normalise_faces();
  void updateOutput();
  QFileInfo face_points;
  bool read_dir;
  int show_idx;
  QList<Mat> normalised_imgs;
};

#endif
