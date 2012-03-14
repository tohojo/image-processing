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
}

#endif
