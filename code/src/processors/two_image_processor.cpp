/**
 * two_image_processor.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-05-08
 */

#include "two_image_processor.h"
#include "util.h"

TwoImageProcessor::TwoImageProcessor(QObject *parent)
  : Processor(parent)
{
}

TwoImageProcessor::~TwoImageProcessor()
{
}

QFileInfo TwoImageProcessor::rightImage()
{
  QMutexLocker l(&mutex);
  return right_image_file;
}

void TwoImageProcessor::setRightImage(QFileInfo path)
{
  QMutexLocker l(&mutex);
  if(path.canonicalFilePath() == right_image_file.canonicalFilePath()) return;
  right_image_file = path;
  right_image = Util::load_image(right_image_file.canonicalFilePath());
  mutex.unlock();
  process();
}
