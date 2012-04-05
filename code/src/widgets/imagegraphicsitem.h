/**
 * imagegraphicsitem.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-05
 */

#ifndef IMAGEGRAPHICSITEM_H
#define IMAGEGRAPHICSITEM_H

#include <QtGui/QGraphicsPixmapItem>

class ImageGraphicsItem : public QObject, public QGraphicsPixmapItem
{
  Q_OBJECT

public:
  ImageGraphicsItem(QGraphicsItem *parent = 0);
  ImageGraphicsItem(const QPixmap &pixmap, QGraphicsItem *parent = 0);
  ~ImageGraphicsItem();
  void setPixmap(const QPixmap &pixmap);

signals:
  void newPOI(QPoint p);
  void POIRemoved(QPoint p);

private:
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void init();
};

#endif
