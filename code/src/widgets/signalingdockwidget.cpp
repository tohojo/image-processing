/**
 * signalingdockwidget.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-05
 */

#include "signalingdockwidget.h"

SignalingDockWidget::SignalingDockWidget(const QString &title, QWidget *parent, Qt::WindowFlags flags)
  : QDockWidget(title, parent, flags)
{
}

SignalingDockWidget::SignalingDockWidget(QWidget *parent, Qt::WindowFlags flags)
  : QDockWidget(parent, flags)
{
}

SignalingDockWidget::~SignalingDockWidget()
{
}

void SignalingDockWidget::closeEvent(QCloseEvent * event)
{
  emit closed(false);
  QDockWidget::closeEvent(event);
}
