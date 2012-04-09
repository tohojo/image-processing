/**
 * integral_image.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-04
 */

#ifndef INTEGRAL_IMAGE_H
#define INTEGRAL_IMAGE_H

#include <cv.h>

using namespace cv;

class IntegralImage
{
public:
  IntegralImage(Mat &img);
  void compute();
  float area(Point start, Point end);
  float area(Point start, int w, int h);
  void toCSV(const char* filename);
private:
  Mat m_img;
  Mat m_int;
};

#endif
