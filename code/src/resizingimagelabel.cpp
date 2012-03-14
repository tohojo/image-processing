#include "resizingimagelabel.h"

ResizingImageLabel::ResizingImageLabel(QWidget *parent)
  : QLabel(parent)
{
}

ResizingImageLabel::ResizingImageLabel(QImage img, QWidget *parent)
  : QLabel(parent)
{
  setImage(img);
}

ResizingImageLabel::~ResizingImageLabel()
{
}

void ResizingImageLabel::setImage(QImage img)
{
  image = img;
  updatePixmap();
}

void ResizingImageLabel::updatePixmap()
{
  if(!image.isNull()) {
    QImage scaled = image.scaled(width(), height(), Qt::KeepAspectRatio);
    setPixmap(QPixmap::fromImage(scaled));
  }
}

void ResizingImageLabel::resizeEvent(QResizeEvent *event)
{
  updatePixmap();
  QLabel::resizeEvent(event);
}
