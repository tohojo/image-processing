/**
 * graphicsview.cpp
 *
 * Toke Høiland-Jansen
 * 2012-04-06
 */

#include "graphicsview.h"
#include <QDebug>

GraphicsView::GraphicsView(QWidget *parent)
  :QGraphicsView(parent)
{
}

GraphicsView::GraphicsView(QGraphicsScene *scene, QWidget *parent)
  :QGraphicsView(scene,parent)
{
}

GraphicsView::~GraphicsView()
{
}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
  int delta = event->delta();
  qreal s;
  if(delta < 0)
    s = 1.0/(-delta/100.0);
  else
    s = delta/100.0;
  scale(s,s);
  event->accept();
}

void GraphicsView::updateZoom()
{
  int zoom = transform().m11()*100;
  if(zoom > 400) {
    QTransform t = QTransform::fromScale(4.0, 4.0);
    setTransform(t);
    return;
  }
  if(zoom < 1) {
    QTransform t = QTransform::fromScale(0.01, 0.01);
    setTransform(t);
    return;
  }
  qDebug("Updating zoom: %d", zoom);
  emit zoomUpdated(zoom);
}

void GraphicsView::scale(qreal dx, qreal dy)
{
  QGraphicsView::scale(dx,dy);
  updateZoom();
}

void GraphicsView::setTransform(const QTransform & matrix, bool combine)
{
  QGraphicsView::setTransform(matrix, combine);
  updateZoom();
}
