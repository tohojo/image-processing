/**
 * imagegraphicsitem.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-05
 */

#include "imagegraphicsitem.h"
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QTransform>
#include <QtGui/QMenu>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QDebug>

ImageGraphicsItem::ImageGraphicsItem(QGraphicsItem *parent)
  :QGraphicsPixmapItem(parent)
{
  init();
}

ImageGraphicsItem::ImageGraphicsItem(const QPixmap &pixmap, QGraphicsItem *parent)
  :QGraphicsPixmapItem(parent)
{
  init();
  setPixmap(pixmap);
}

void ImageGraphicsItem::init()
{
  setFlags(QGraphicsItem::ItemIsSelectable);
  setCursor(Qt::CrossCursor);
  poi_lines = false;
}

ImageGraphicsItem::~ImageGraphicsItem()
{
}

void ImageGraphicsItem::setPixmap(const QPixmap &pixmap)
{
  QGraphicsPixmapItem::setPixmap(pixmap);
  QSize s = pixmap.size();
  QPointF p(-s.width()/2.0f, -s.height()/2.0f);
  setPos(p);
  scene()->setSceneRect(sceneBoundingRect());
}

void ImageGraphicsItem::removePOI(POIItem *poi)
{
  QPoint p = poi->pos().toPoint();
  delete poi;
  emit POIRemoved(p);
}

void ImageGraphicsItem::clearPOIs()
{
  foreach(QGraphicsItem * item, childItems()) {
    POIItem * poi = static_cast<POIItem*>(item);
    removePOI(poi);
  }
}

void ImageGraphicsItem::addPOI(QPoint p)
{
  emit newPOI(p);

  POIItem * poi = new POIItem(this);
  poi->setPos(p);
  poi->setLine(poi_lines);
}

void ImageGraphicsItem::removePOI(QPoint p)
{
  foreach(QGraphicsItem * item, childItems()) {
    POIItem * poi = static_cast<POIItem*>(item);
    if(poi->pos().toPoint() == p) removePOI(poi);
  }
}

void ImageGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  QPoint p = event->pos().toPoint();
  addPOI(p);
}

void ImageGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  event->ignore();
}

void ImageGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  scene()->clearSelection();
  event->ignore();
}

void ImageGraphicsItem::setPOILines(bool v)
{
  poi_lines = v;
 foreach(QGraphicsItem * item, childItems()) {
    POIItem * poi = static_cast<POIItem*>(item);
    poi->setLine(poi_lines);
 }
}
