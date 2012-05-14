/**
 * poiitem.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-06
 */

#include "poiitem.h"
#include <QtGui/QCursor>
#include <QtGui/QColor>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QDebug>
#include "imagegraphicsitem.h"

POIItem::POIItem(QGraphicsItem *parent)
  : QGraphicsEllipseItem(parent),
    line(0)
{
  setFlags(QGraphicsItem::ItemIsSelectable);
  setCursor(Qt::PointingHandCursor);
  setBrush(QBrush(QColor::fromRgb(255, 0, 0)));
  setRect(-4,-4,9,9);
}

POIItem::~POIItem()
{
}

void POIItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  QMenu menu;
  QPoint p = pos().toPoint();
  QString s = QString("POI at (%1,%2)").arg(p.x()).arg(p.y());
  QAction *headingAction = menu.addAction(s);
  headingAction->setEnabled(false);
  menu.addSeparator();
  QAction *removeAction = menu.addAction("&Remove POI");
  QAction *selectedAction = menu.exec(event->screenPos());
  if(selectedAction == removeAction) {
    ImageGraphicsItem *parent = static_cast<ImageGraphicsItem *>(parentItem());
    parent->removePOI(this);
  }
}

void POIItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  event->ignore();
}

void POIItem::setLine(bool v)
{
  if(v) {
    if(line) return;
    QPointF p = pos();
    QRectF boundRect = parentItem()->boundingRect();
    QPointF start(0, p.y());
    QPointF end(boundRect.right(), p.y());
    line = new QGraphicsLineItem(QLineF(mapFromParent(start), mapFromParent(end)), this);
    QPen pen(QColor(255,0,0));
    line->setPen(pen);
    setPen(pen);
    setBrush(QBrush(QColor(0,0,0,0)));
  } else {
    if(!line) return;
    delete line;
    line = 0;
    setPen(QPen(QColor(0,0,0)));
    setBrush(QBrush(QColor(255,0,0)));
  }
}
