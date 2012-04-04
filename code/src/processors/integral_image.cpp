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
  return m_int.at<float>(start) +
         m_int.at<float>(end) -
        (m_int.at<float>(Point(start.x,end.y))+
         m_int.at<float>(Point(end.x,start.y)));
}
