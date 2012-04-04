/**
 * integral_image.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-04
 */

#include "integral_image.h"

IntegralImage::IntegralImage(Mat img)
{
  m_img = img;
  compute();
}

void IntegralImage::compute()
{
  int i,j;
  double row_sum;
  m_int = Mat(m_img.rows, m_img.cols, CV_64F);
  for(i = 0; i < m_img.rows; i++) {
    row_sum = 0;
    for(j = 0; j < m_img.cols; j++) {
      row_sum += m_img.at<uchar>(i,j);
      if(i == 0)
        m_int.at<double>(i,j) = row_sum;
      else
        m_int.at<double>(i,j) = m_int.at<double>(i-1,j)+row_sum;
    }
  }
}

double IntegralImage::area(Point start, Point end)
{
  return m_int.at<double>(start) +
         m_int.at<double>(end) -
        (m_int.at<double>(Point(start.x,end.y))+
         m_int.at<double>(Point(end.x,start.y)));
}
