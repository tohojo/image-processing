#ifndef UTIL_H
#define UTIL_H

#include <QImage>
#include <QIODevice>
#include <cv.h>
#include <highgui.h>

using namespace cv;

namespace Util
{
  const QImage mat_to_qimage (Mat img);
  Mat load_image(QString filename);
  Mat load_image_colour(QString filename);
  void save_image(Mat ing, QString filename);
  uint32_t nearest_pow (uint32_t num, bool smaller = false);
  void img_to_csv(const char *filename, Mat mat);
  void write_matrix(Mat m, QIODevice *dev);
  QString format_matrix_float(Mat m);
  bool read_matrix(Mat m, QIODevice *dev);
  Mat combine(Mat l, Mat r);
  QList<Point> read_POIs(QIODevice *dev);
  bool comparePointsX(const Point p1, const Point p2);
}

#endif
