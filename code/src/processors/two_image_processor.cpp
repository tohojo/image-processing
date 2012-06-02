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
  twoimage_output = false;
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
  if(uses_colour)
    right_image = Util::load_image_colour(right_image_file.canonicalFilePath());
  else
    right_image = Util::load_image(right_image_file.canonicalFilePath());
  mutex.unlock();
  process();
}

QFileInfo TwoImageProcessor::rightOutput()
{
  QMutexLocker l(&mutex);
  return right_image_output;
}

void TwoImageProcessor::setRightOutput(QFileInfo path)
{
  QMutexLocker l(&mutex);
  if(path.filePath() == right_image_output.filePath()) return;
  right_image_output = path;
}

void TwoImageProcessor::saveOutput()
{
  if(!twoimage_output) {
    Processor::saveOutput();
    return;
  }
  QMutexLocker l(&mutex);
  QString filename_l = image_output_file.filePath();
  if(!filename_l.isEmpty()) {
    Util::save_image(left_output, filename_l);
  }
  QString filename_r = right_image_output.filePath();
  if(!filename_r.isEmpty()) {
    Util::save_image(right_output, filename_r);
  }
}

void TwoImageProcessor::set_output_images(Mat l, Mat r)
{
  QMutexLocker lock(&mutex);
  output_image = Util::combine(l,r);
  left_output = l;
  right_output = r;
  twoimage_output = true;
}

Mat TwoImageProcessor::getRightOutput()
{
  QMutexLocker lock(&mutex);
  return right_output;
}

Mat TwoImageProcessor::getLeftOutput()
{
  QMutexLocker lock(&mutex);
  return left_output;
}
