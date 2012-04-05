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
#include <QDebug>

ImageGraphicsItem::ImageGraphicsItem(QGraphicsItem *parent)
  :QGraphicsPixmapItem(parent)
{
}

ImageGraphicsItem::ImageGraphicsItem(const QPixmap &pixmap, QGraphicsItem *parent)
  :QGraphicsPixmapItem(parent)
{
  setPixmap(pixmap);
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
  foreach(QGraphicsView *view, scene()->views()) {
    // Scale the graphics view to leave 15 pixels of air on each side
    // of the image, and recenter it.
    QRect r = view->frameRect();
    float scale = qMin((r.width()-30.0f)/s.width(), (r.height()-30.0f)/s.height());
    scale = qMax(scale, 0.01f);
    scale = qMin(scale, 4.0f);
    QTransform transform = QTransform::fromScale(scale, scale);
    view->centerOn(this);
    view->setTransform(transform);
  }
}

void ImageGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/)
{
}
