#ifndef RESIZINGIMAGELABEL_H
#define RESIZINGIMAGELABEL_H

#include <QtGui/QLabel>

class ResizingImageLabel : public QLabel
{
Q_OBJECT

public:
  ResizingImageLabel(QWidget *parent =0);
  ResizingImageLabel(QImage img, QWidget *parent = 0);
  ~ResizingImageLabel();

public slots:
  void setImage(QImage img);
  void updatePixmap();

protected:
  void resizeEvent(QResizeEvent *event);

private:
  QImage image;

};

#endif
