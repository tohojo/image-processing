/**
 * response_layer.cpp
 *
 * Toke Høiland-Jensen
 * 2012-04-07
 */

#include "response_layer.h"
#include <QDebug>
#include <fstream>

ResponseLayer::ResponseLayer(int width, int height, int step, int filter_size)
  : responses(height, width, CV_32F),
    m_width(width),
    m_height(height),
    m_step(step),
    m_filter_size(filter_size)
{
}

ResponseLayer::~ResponseLayer()
{
}


void ResponseLayer::setResponse(int row, int col, float val)
{
  responses.at<float>(row,col) = val;
}

void ResponseLayer::setResponse(Point p, float val)
{
  responses.at<float>(p) = val;
}

float ResponseLayer::getResponse(Point p)
{
  if(p.x >= responses.cols || p.y >= responses.rows) return 0.0f;
  return responses.at<float>(p);
}

float ResponseLayer::getResponse(int row, int col)
{
  return responses.at<float>(row,col);
}

/**
 * Get a response for corresponding to another layer (possibly with a
 * different scale).
 */
float ResponseLayer::getResponse(int row, int col, ResponseLayer *other)
{
  int scale = m_width/other->m_width;
  return getResponse(row*scale, col*scale);
}

float ResponseLayer::getResponse(Point p, ResponseLayer *other)
{
  int scale = m_width/other->m_width;
  return getResponse(Point(p.x*scale, p.y*scale));
}

void ResponseLayer::toCSV(const char *filename)
{
  std::ofstream file;
  file.open(filename);
  for(int i = 0; i < responses.rows; i++) {
    for(int j = 0; j < responses.cols; j++) {
      file << responses.at<float>(i,j);
      if(j < responses.cols-1) file << ",";
    }
    file << "\n";
  }

  file.close();

}
