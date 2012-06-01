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
	crop_x(0.25),
    crop_y(1.2),
    scaled_width(256),
    output_dir()
{
  uses_colour = true;
}

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
  QString filename = face_points.fileName();
  QString dirname = face_points.absolutePath();
  float scale_width = (float) scaled_width;
  QString outdir = output_dir.absoluteFilePath();
  QString avgfilename = average_file.absoluteFilePath();
  mutex.unlock();

  if(filename.isEmpty()) return;

  QStringList files;
  QStringList img_files;
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
    if(abort || restart) return;
    QString txt_filename = QString("%1/%2").arg(dirname).arg(fname);
    QString img_filename(txt_filename);
    img_filename.replace(".txt", ".jpg");
    if(!QFileInfo(img_filename).exists()) continue;
    QFile file(txt_filename);
    if(file.open(QIODevice::ReadOnly)) {
      QList<Point> POIs = Util::read_POIs(&file);
      if(POIs.empty()) continue;
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
      img_files << img_filename;
    }
  }
  if(img_list.empty()) return;
  avg /= img_list.size();

  if(!avgfilename.isEmpty()) {
    qDebug() << "Overriding average from file" << avgfilename;
    QFile file(avgfilename);
    if(file.open(QIODevice::ReadOnly)) {
      QList<Point> POIs = Util::read_POIs(&file);
      int i = 0;
      foreach(Point pt, POIs) {
        avg.at<float>(i,0) = (float) pt.x;
        avg.at<float>(i,1) = (float) pt.y;
        i++;
      }
    }
  }

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

  qDebug() << min_x << max_x << min_y << max_y << range_x << range_y;

  int i = 0;
  foreach(Mat img_points, img_list) {
    if(abort || restart) return;
    emit progress(100*((float)i)/img_list.size());
    Mat transform;
    if(!solve(img_points, avg, transform, DECOMP_SVD)) {
      qDebug() << "Solve error";
    } else {
      QString img_filename = img_files[i];
      Mat img;
      if(uses_colour)
        img = Util::load_image_colour(img_filename);
      else
        img = Util::load_image(img_filename);
      Mat affine = transform.t();
      Size s = img.size();
      Mat dst;
      warpAffine(img, dst, Mat(affine, Rect(0,0,3,2)), s, INTER_CUBIC);

      // Crop image by selecting sub-region, make sure we do not crash by using
      // out-of-bounds values.
      Rect roi_clipped(roi.x,roi.y,qMin(roi.width, img.cols-roi.x),
                       qMin(roi.height, img.rows-roi.y));
      Mat cropped(dst, roi_clipped);

      // Draw an ellipse, by drawing the maximum ellipse in black onto a
      // non-black image of the same size, then use that image as a fill mask
      // for the real image
      Mat mask = Mat::ones(cropped.rows, cropped.cols, CV_8U);
      ellipse(mask, RotatedRect(Point2f(mask.cols/2, mask.rows/2), mask.size(), 0), Scalar(0), -1);
      cropped.setTo(Scalar(0), mask);

      // Rescale the image to a width of 256 pixels. This also dramatically
      // helps on memory use, since the original images are no longer kept in
      // memory.
      Mat rescaled;
      float scale_factor = scale_width/cropped.cols;
      resize(cropped, rescaled, Size(), scale_factor, scale_factor, INTER_AREA);
      normalised << rescaled;
      i++;
    }
  }

  if(!outdir.isEmpty() && output_dir.isDir()) {
    qDebug () << "Outputting to" << outdir;
    for(int i = 0; i < img_files.size(); i++) {
      QString output_path = QString("%1/%2.normal.png").arg(outdir).arg(QFileInfo(img_files[i]).baseName());
      Util::save_image(normalised[i], output_path);
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

void FaceNormalisationProcessor::setUseColour(bool value)
{
  QMutexLocker locker(&mutex);
  if(uses_colour == value) return;
  uses_colour = value;
  mutex.unlock();
  process();
}

void FaceNormalisationProcessor::setShowIndex(int value)
{
  QMutexLocker locker(&mutex);
  if(show_idx == value) return;
  show_idx = value;
  mutex.unlock();
  emit showIdxUpdated();
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


void FaceNormalisationProcessor::setScaledWidth(int value)
{
  QMutexLocker locker(&mutex);
  if(scaled_width == value) return;
  scaled_width = value;
  mutex.unlock();
  process();
}

void FaceNormalisationProcessor::left()
{
  QMutexLocker locker(&mutex);
  int idx = show_idx;
  if(idx > 0) {
    mutex.unlock();
    setShowIndex(idx-1);
  }
}

void FaceNormalisationProcessor::right()
{
  QMutexLocker locker(&mutex);
  int idx = show_idx;
  if(idx < normalised_imgs.size()-1) {
    mutex.unlock();
    setShowIndex(idx+1);
  }
}

void FaceNormalisationProcessor::setOutputDir(QFileInfo path)
{
  QMutexLocker locker(&mutex);
  if(path.absoluteFilePath() == output_dir.absoluteFilePath()) return;
  output_dir = path;
  mutex.unlock();
  process();
}

void FaceNormalisationProcessor::setAverageFile(QFileInfo path)
{
  QMutexLocker locker(&mutex);
  if(path.canonicalFilePath() == average_file.canonicalFilePath()) return;
  average_file = path;
  mutex.unlock();
  process();
}

