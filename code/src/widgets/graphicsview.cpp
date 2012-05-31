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
  QPointF before = mapToScene(event->pos());
  QPointF center = mapToScene(QPoint(viewport()->width()/2, viewport()->height()/2));
  scale(s,s);
  ensureVisible(QRectF(before, QSize(1,1)));
  QPointF after = mapToScene(event->pos());
  centerOn(center-(after-before));

  event->accept();
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
  if(event->key() == Qt::Key_Left)
    emit left();
  else if(event->key() == Qt::Key_Right)
    emit right();
  else
    QGraphicsView::keyPressEvent(event);
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
  int textSize = 16; // Height of text bounding box, in pixels
  QBrush bg = QBrush(QColor::fromRgb(0, 0, 0, 200));
  QPen text = QPen(QColor::fromRgb(220,220,0));
  QFont font("Sans", 10);
  QString string("Drag to move. Scroll to zoom. Double click to add a POI. Right click on a POI to remove it.");


  // We want to draw in viewport coordinates, so remove the painter
  // scaling, and move it into position height pixels above the
  // bottom.
  QRect vp = painter->viewport();
  painter->setWorldTransform(QTransform(1.0, // scale x
                                        0, 0,
                                        1.0, // scale y
                                        0, // x pos
                                        vp.height()-height)); // y pos


  // Draw the translucent rectangle in the bottom of the screen. Make
  // sure the border in the sides are outside the view (so there's a
  // border at the top only.
  QRect rect = QRect(-1, 0, vp.width()+2, height);
  painter->setBrush(bg);
  painter->drawPolygon(rect);

  painter->setPen(text);
  painter->setFont(font);
  int ypos = (height-textSize)/2;
  QRectF textPos(QPoint(10,ypos),
                 QSize(vp.width(), height-ypos));
  painter->drawText(textPos, Qt::AlignLeft, string);
}
