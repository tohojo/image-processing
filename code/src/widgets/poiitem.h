/**
 * poiitem.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-06
 */

#ifndef POIITEM_H
#define POIITEM_H

#include <QtGui/QGraphicsEllipseItem>
#include <QtGui/QGraphicsSceneContextMenuEvent>

class POIItem : public QGraphicsEllipseItem
{
public:
  POIItem(QGraphicsItem *parent = 0);
  POIItem(const QRectF &rect, QGraphicsItem *parent = 0);
  ~POIItem();

private:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

#endif
