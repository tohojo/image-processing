/**
 * response_layer.cpp
 *
 * Toke Høiland-Jensen
 * 2012-04-07
 */

#include "response_layer.h"

ResponseLayer::ResponseLayer(int width, int height, int step, int filter_size)
  : responses(width, height, CV_32F),
    m_width(width),
    m_height(height),
    m_step(step),
    m_filter_size(filter_size)
{
}

ResponseLayer::~ResponseLayer()
{
}


void ResponseLayer::set(int row, int col, float val)
{
  responses.at<float>(row,col) = val;
}

float ResponseLayer::get(int row, int col)
{
  return responses.at<float>(row,col);
}
