/**
 * graphicsview.cpp
 *
 * Toke Høiland-Jansen
 * 2012-04-06
 */

#include "graphicsview.h"
#include <QBrush>
#include <QDebug>

GraphicsView::GraphicsView(QWidget *parent)
  :QGraphicsView(parent)
{
  setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
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

void GraphicsView::drawForeground(QPainter *painter, const QRectF &/*rect*/)
{
  int height = 30;
  int textSize = 14;
  QBrush bg = QBrush(QColor::fromRgb(0, 0, 0, 200));
  QPen text = QPen(QColor::fromRgb(220,220,0));
  QString string("Drag to move. Scroll to zoom. Double click to add a POI. Right click on a POI to remove it.");


  // Straight forward: Draw the translucent rectangle in the bottom of the screen.
  QRect vp = painter->viewport();
  QRect rect = QRect(vp.x()-1, vp.height()-height, vp.width()+2, height);
  painter->setBrush(bg);
  painter->drawPolygon(mapToScene(rect));

  // More involved: Change the transform of the painter to paint the
  // text correctly without scaling. This is found by a lot of
  // experimenting.
  //
  // The idea is to not scale, and to replace the scaling with a
  // corresponding translation (only in the x direction, for some
  // reason).
  painter->setPen(text);
  QTransform tf = painter->worldTransform();
  qreal scale = 1/tf.m11();
  int newx = tf.dx()*scale;
  painter->setWorldTransform(QTransform(1.0, tf.m12(), tf.m21(), 1.0, newx, rect.bottomLeft().y()/2));
  QRectF textPos(QPoint(-newx, rect.bottomLeft().y()/2) + QPoint(10,-(height-(height-textSize)/2)),
                 QSize(vp.width(), textSize));
  painter->drawText(textPos, Qt::AlignLeft, string);
}
