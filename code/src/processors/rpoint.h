/**
 * rpoint.h
 *
 * Toke Høiland-Jørgensen
 * 2012-03-25
 */

#ifndef RPOINT_H
#define RPOINT_H

namespace ImageProcessing {

  class RPoint
  {
  public:
    RPoint();
    RPoint(int x, int y);
    RPoint(const RPoint &other);
    ~RPoint();

    bool operator<(const RPoint& other) const;
    RPoint operator+(const RPoint& other) const;
    RPoint& operator=(const RPoint &other);
    inline bool operator==(const RPoint& other) const
    {
      return (m_x == other.m_x && m_y == other.m_y);
    }

    int x() const {return m_x;}
    int y() const {return m_y;}

    void print() const;

  private:
    int m_x;
    int m_y;
  };

  inline unsigned int qHash(const RPoint &key)
  {
    return key.x() ^ key.y();
  }
}
#endif
