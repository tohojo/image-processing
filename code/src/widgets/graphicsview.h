/**
 * graphicsview.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-06
 */

#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QtGui/QWheelEvent>
#include <QtGui/QTransform>
#include <QtGui/QPainter>

class GraphicsView : public QGraphicsView
{
  Q_OBJECT

public:
  GraphicsView(QWidget *parent =0);
  ~GraphicsView();
  void updateZoom();
  void setTransform(const QTransform &matrix, bool combine = false);
  void scale(qreal dx, qreal dy);

signals:
  void zoomUpdated(int);

private:
  void wheelEvent(QWheelEvent *event);
  void drawForeground(QPainter *painter, const QRectF &rect);
};

#endif
