/**
 * face_normalisation_processor.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-05-30
 */

#include <QStringList>
#include <QDir>
#include <QDebug>
#include "face_normalisation_processor.h"
#include "util.h"

using namespace cv;

FaceNormalisationProcessor::FaceNormalisationProcessor(QObject *parent)
  : Processor(parent),
    face_points(),
    read_dir(true),
    show_idx(0),
    crop_x(0.3),
    crop_y(1.8)
{}

FaceNormalisationProcessor::~FaceNormalisationProcessor()
{}

void FaceNormalisationProcessor::run()
{
  forever {
    if(abort) return;
    emit progress(10);
    normalise_faces();
    emit progress(100);
    emit updated();
    if(once) return;

    mutex.lock();
    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}

void FaceNormalisationProcessor::normalise_faces()
{
  mutex.lock();
  bool dir = read_dir;
  QString filename = face_points.absoluteFilePath();
  QString dirname = face_points.absolutePath();
  mutex.unlock();

  if(filename.isEmpty()) return;

  QStringList files;
  if(read_dir) {
    QDir dir(dirname);
    QStringList namefilters;
    namefilters << "*.txt";
    files = dir.entryList(namefilters, QDir::Files|QDir::Readable, QDir::Name);
  } else {
    files << filename;
  }

  if(files.empty()) return;

  Mat avg(3,3,CV_32F,Scalar::all(0));
  QList<Mat> img_list;

  foreach(QString fname, files) {
    QFile file(QString("%1/%2").arg(dirname).arg(fname));
    if(file.open(QIODevice::ReadOnly)) {
      QList<Point> POIs = Util::read_POIs(&file);
      Mat img = Mat::ones(POIs.size(), 3, CV_32F);
      int i = 0;
      qStableSort(POIs.begin(), POIs.end(), Util::comparePointsX);
      foreach(Point pt, POIs) {
        img.at<float>(i,0) = (float) pt.x;
        img.at<float>(i,1) = (float) pt.y;
        i++;
      }
      avg += img;
      img_list << img;
    }
  }
  if(img_list.empty()) return;
  avg /= img_list.size();

  qDebug() << "Avg" << endl << Util::format_matrix_float(avg);

  QList<Mat> normalised;

  double min_x,min_y,max_x,max_y;
  minMaxLoc(Mat(avg, Rect(0,0,1,3)), &min_x, &max_x, 0, 0);
  minMaxLoc(Mat(avg, Rect(1,0,1,3)), &min_y, &max_y, 0, 0);

  double range_x = max_x-min_x;
  double range_y = max_y-min_y;

  Rect roi(qMax(min_x-range_x*crop_x,0.0),
           qMax(min_y-range_y*crop_y,0.0),
           range_x*(1.0+2*crop_x),
           range_y*(1.0+2*crop_y));

  if(POIs.empty()) {
    for(int i = 0; i < avg.rows; i++) {
      emit newPOI(QPoint((int) avg.at<float>(i,0)-roi.x, (int) avg.at<float>(i,1)-roi.y));
    }
  }

  qDebug() << min_x << max_x << min_y << max_y << range_x << range_y;

  int i = 0;
  foreach(Mat img_points, img_list) {
    if(abort) return;
    emit progress(100*((float)i)/img_list.size());
    Mat transform;
    if(!solve(img_points, avg, transform, DECOMP_SVD)) {
      qDebug() << "Solve error";
    } else {
      QString img_filename = QString("%1/%2.jpg").arg(dirname).arg(QFileInfo(files[i]).baseName());
      Mat img = Util::load_image(img_filename);
      Mat affine = transform.t();
      Mat dst(img.rows, img.cols, img.type());
      Size s = img.size();
      warpAffine(img, dst, Mat(affine, Rect(0,0,3,2)), s, INTER_CUBIC);
      Rect roi_clipped(roi.x,roi.y,qMin(roi.width, img.cols-roi.x), qMin(roi.height, img.rows-roi.y));
      Mat cropped(dst, roi_clipped);
      normalised << cropped;
      i++;
    }
  }

  mutex.lock();
  normalised_imgs = normalised;
  mutex.unlock();

  updateOutput();

}

void FaceNormalisationProcessor::updateOutput()
{
  QMutexLocker l(&mutex);
  if(show_idx < normalised_imgs.size()) {
    output_image = normalised_imgs[show_idx];
  } else {
    output_image = Mat();
  }
  mutex.unlock();
  emit updated();
}

void FaceNormalisationProcessor::setFacePoints(QFileInfo path)
{
  QMutexLocker locker(&mutex);
  if(path.canonicalFilePath() == face_points.canonicalFilePath()) return;
  face_points = path;
  mutex.unlock();
  process();
}

void FaceNormalisationProcessor::setReadDir(bool value)
{
  QMutexLocker locker(&mutex);
  if(read_dir == value) return;
  read_dir = value;
  mutex.unlock();
  process();
}

void FaceNormalisationProcessor::setShowIndex(int value)
{
  QMutexLocker locker(&mutex);
  if(show_idx == value) return;
  show_idx = value;
  mutex.unlock();
  updateOutput();
}

void FaceNormalisationProcessor::setCropX(float value)
{
  QMutexLocker locker(&mutex);
  if(crop_x == value) return;
  crop_x = value;
  mutex.unlock();
  process();
}

void FaceNormalisationProcessor::setCropY(float value)
{
  QMutexLocker locker(&mutex);
  if(crop_y == value) return;
  crop_y = value;
  mutex.unlock();
  process();
}
