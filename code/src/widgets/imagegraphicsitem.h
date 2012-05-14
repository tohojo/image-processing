/**
 * imagegraphicsitem.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-05
 */

#ifndef IMAGEGRAPHICSITEM_H
#define IMAGEGRAPHICSITEM_H

#include <QtGui/QGraphicsPixmapItem>
#include "poiitem.h"

class ImageGraphicsItem : public QObject, public QGraphicsPixmapItem
{
  Q_OBJECT

public:
  ImageGraphicsItem(QGraphicsItem *parent = 0);
  ImageGraphicsItem(const QPixmap &pixmap, QGraphicsItem *parent = 0);
  ~ImageGraphicsItem();
  void setPixmap(const QPixmap &pixmap);
  void removePOI(POIItem * poi);

public slots:
  void clearPOIs();
  void addPOI(QPoint p);
  void removePOI(QPoint p);
  void setPOILines(bool v);

signals:
  void newPOI(QPoint p);
  void POIRemoved(QPoint p);

private:
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void init();
  bool poi_lines;
};

#endif
