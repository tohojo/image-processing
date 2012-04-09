/**
 * response_layer.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-07
 */

#ifndef RESPONSE_LAYER_H
#define RESPONSE_LAYER_H

#include <cv.h>
using namespace cv;

/**
 * Class to represent a response layer in the image filtering pyramid.
 *
 * Idea from OpenSURF implementation.
 */

class ResponseLayer
{
public:
  ResponseLayer(int width, int height, int step, int filter_size);
  ~ResponseLayer();

  int width() { return m_width; }
  int height() { return m_height; }
  int step() { return m_step; }
  int filter_size() { return m_filter_size; }

  float getResponse(int row, int col, ResponseLayer *other);
  float getResponse(Point p, ResponseLayer *other);
  float getResponse(int row, int col);
  float getResponse(Point p);
  void  setResponse(int row, int col, float val);
  void  setResponse(Point p, float val);
  void toCSV(const char* filename);

private:
  Mat responses;
  int m_width, m_height, m_step, m_filter_size;
};

#endif
