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
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QDebug>
#include "imagegraphicsitem.h"

POIItem::POIItem(QGraphicsItem *parent)
  : QGraphicsEllipseItem(parent)
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
