#ifndef STEREO_PROCESSOR_H
#define STEREO_PROCESSOR_H

#include "two_image_processor.h"

using namespace cv;

class StereoProcessor : public TwoImageProcessor
{
  Q_OBJECT

public:
  StereoProcessor(QObject *parent = 0);
  ~StereoProcessor();

  QString name() {return "Stereo matching";}

private:
	StereoProcessor::dynamicProgramming();
  void run();
  Mat leftDepthMap;
  Mat rightDepthMap;
};

#endif
