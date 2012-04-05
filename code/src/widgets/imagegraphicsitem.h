/**
 * imagegraphicsitem.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-05
 */

#ifndef IMAGEGRAPHICSITEM_H
#define IMAGEGRAPHICSITEM_H

#include <QtGui/QGraphicsPixmapItem>

class ImageGraphicsItem : public QGraphicsPixmapItem
{

public:
  ImageGraphicsItem(QGraphicsItem *parent = 0);
  ImageGraphicsItem(const QPixmap &pixmap, QGraphicsItem *parent = 0);
  ~ImageGraphicsItem();
  void setPixmap(const QPixmap &pixmap);

private:
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
};

#endif
