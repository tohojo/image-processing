/**
 * rpoint.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-03-25
 */

#include "rpoint.h"

using namespace ImageProcessing;

RPoint::RPoint()
{
  m_x = m_y = 0;
}

RPoint::RPoint(int x, int y)
{
  m_x = x;
  m_y = y;
}

RPoint::~RPoint()
{
}

bool RPoint::operator <(const RPoint& other) const
{
  if(m_x < other.x())
    return true;
  if(m_x == other.x() && m_y < other.y())
    return true;
  return false;
}

