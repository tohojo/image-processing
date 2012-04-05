/**
 * signalingdockwidget.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-05
 */

#ifndef SIGNALINGDOCKWIDGET_H
#define SIGNALINGDOCKWIDGET_H
#include <QtGui/QDockWidget>

class SignalingDockWidget : public QDockWidget
{
  Q_OBJECT

public:
  SignalingDockWidget(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = 0);
  SignalingDockWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);
  ~SignalingDockWidget();

signals:
  void closed(bool b);

private:
  void closeEvent(QCloseEvent *event);
};
#endif
