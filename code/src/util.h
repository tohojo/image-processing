#ifndef UTIL_H
#define UTIL_H

#include <QImage>
#include <cv.h>
#include <highgui.h>

using namespace cv;

namespace Util
{
  const QImage mat_to_qimage (Mat img);
  Mat load_image(QString filename);
  uint32_t nearest_pow (uint32_t num, bool smaller = false);
  void img_to_csv(const char *filename, Mat mat);
}

#endif
