#ifndef UTIL_H
#define UTIL_H

#include <QImage>
#include <cv.h>
#include <highgui.h>

using namespace cv;

class Util
{
public:
  static const QImage mat_to_qimage (Mat img);
  static Mat load_image(QString filename);
};

#endif
