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
    read_dir(true)
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
  QString filename = face_points.canonicalFilePath();
  QString dirname = face_points.canonicalPath();
  mutex.unlock();

  QStringList files;
  if(read_dir) {
    QDir dir(dirname);
    QStringList namefilters;
    namefilters << "*.txt";
    files = dir.entryList(namefilters, QDir::Files|QDir::Readable|QDir::NoSymLinks, QDir::Name);
  } else {
    files << filename;
  }

  Mat avg(3,3,CV_32F,Scalar::all(0));
  QList<Mat> img_list;

  foreach(QString fname, files) {
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly)) {
      QList<Point> POIs = Util::read_POIs(&file);
      Mat img(POIs.size(), 3, CV_32F);
      int i = 0;
      foreach(Point pt, POIs) {
        Mat p(1, 3, CV_32F, Scalar::all(1));
        p.at<float>(0,0) = pt.x;
        p.at<float>(0,1) = pt.y;
        img.row(i++) = p;
      }
      avg += img;
      img_list << img;
    }
  }
  avg /= img_list.size();

  int i = 0;
  foreach(Mat img_points, img_list) {
    Mat transform;
    if(!solve(img_points, avg, transform, DECOMP_SVD)) {
      qDebug() << "Solve error";
    } else {
      QString img_filename = QString("%1.jpg").arg(QFileInfo(files[i]).baseName());
      Mat img = Util::load_image(img_filename);

    }
  }

  if(filename.isEmpty()) return;
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
