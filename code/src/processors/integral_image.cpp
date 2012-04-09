/**
 * integral_image.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-04
 */

#include "integral_image.h"
#include <QDebug>
#include <fstream>

IntegralImage::IntegralImage(Mat &img)
{
  m_img = img;
  m_int = Mat(m_img.rows, m_img.cols, CV_32F);
  compute();
}

void IntegralImage::compute()
{
  int i,j;
  float row_sum;
  for(i = 0; i < m_img.rows; i++) {
    row_sum = 0;
    for(j = 0; j < m_img.cols; j++) {
      row_sum += m_img.at<uchar>(i,j);
      if(i == 0)
        m_int.at<float>(i,j) = row_sum;
      else
        m_int.at<float>(i,j) = m_int.at<float>(i-1,j)+row_sum;
    }
  }
}

float IntegralImage::area(Point start, Point end)
{
  if(end.x>=m_int.cols || end.y>=m_int.rows) {
    qFatal("End value too large: (%d, %d) (max (%d,%d))", end.x, end.y, m_int.cols, m_int.rows);
  }

  float A(0.0f), B(0.0f), C(0.0f), D(0.0f);

  if(start.x >= 0 && start.y >= 0) A = m_int.at<float>(start);
  if(start.x >= 0 && end  .y >= 0) B = m_int.at<float>(Point(start.x,end.y));
  if(end  .x >= 0 && start.y >= 0) C = m_int.at<float>(Point(end.x,start.y));
  if(end  .x >= 0 && end  .y >= 0) D = m_int.at<float>(end);

  return A-B-C+D;
}

/**
 * Get the are for a rectangle by specifying the top-left point in the
 * rectangle along with its width and height.
 */
float IntegralImage::area(Point p, int width, int height)
{
  // Subtract 1 to get the exclusive point outside the rectangle
  Point start(qMin(p.x, m_img.cols)-1, qMin(p.y, m_img.rows)-1);
  Point end(qMin(p.x+width, m_img.cols)-1, qMin(p.y+height, m_img.rows)-1);
  return area(start,end);
}

void IntegralImage::toCSV(const char *filename)
{
  std::ofstream file;
  file.open(filename);
  for(int i = 0; i < m_int.rows; i++) {
    for(int j = 0; j < m_int.cols; j++) {
      file << m_int.at<float>(i,j);
      if(j < m_int.cols-1) file << ",";
    }
    file << "\n";
  }

  file.close();

}
