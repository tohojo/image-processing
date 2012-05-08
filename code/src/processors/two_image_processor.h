/**
 * two_image_processor.h
 *
 * Toke Høiland-Jørgensen
 * 2012-05-08
 */

#ifndef TWO_IMAGE_PROCESSOR_H
#define TWO_IMAGE_PROCESSOR_H

#include <QtCore/QFileInfo>
#include "processor.h"

using namespace cv;

class TwoImageProcessor : public Processor
{
  Q_OBJECT
  Q_PROPERTY(QFileInfo RightImage READ rightImage WRITE setRightImage USER true)
  Q_CLASSINFO("RightImage", "filetype=images;")

public:
  TwoImageProcessor(QObject *parent = 0);
  ~TwoImageProcessor();

  QString name() {return "Two image processor";}

  QFileInfo rightImage();
  void setRightImage(QFileInfo path);

protected:
  Mat right_image;
  QFileInfo right_image_file;
};

#endif
