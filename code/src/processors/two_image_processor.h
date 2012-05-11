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

  Q_PROPERTY(QFileInfo RightOutput READ rightOutput WRITE setRightOutput USER true)
  Q_CLASSINFO("RightOutput", "filetype=images;opentype=WRITE;")

public:
  TwoImageProcessor(QObject *parent = 0);
  ~TwoImageProcessor();

  QString name() {return "Two image processor";}

  QFileInfo rightImage();
  void setRightImage(QFileInfo path);

  QFileInfo rightOutput();
  void setRightOutput(QFileInfo path);

public slots:
  void saveRightOutput();

protected:
  Mat right_image;
  QFileInfo right_image_file;
  QFileInfo right_image_output;
};

#endif
