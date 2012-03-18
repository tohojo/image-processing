#include "segmenting.h"
#include "util.h"

Segmenting::Segmenting(QObject *parent)
  : Processor(parent)
{
  m_threshold = 0;
  m_dark_bg = false;
  m_mode = GLOBAL_THRESHOLD;
}

Segmenting::~Segmenting()
{
}

void Segmenting::process()
{
  if(input_image.empty()) return;

  switch(m_mode) {
  case GLOBAL_THRESHOLD:
    qDebug("Global threshold mode");
    thresholdSegment(false);
    break;
  case ADAPTIVE_THRESHOLD:
    qDebug("Adaptive threshold mode");
    thresholdSegment(true);
    break;
  case SPLIT_MERGE:
    qDebug("Split and merge mode");
    splitMerge();
    break;
  }

  emit updated();
}

void Segmenting::thresholdSegment(bool adapt) {

  Scalar m = mean(input_image);
  m_threshold = cvRound(m[0]);

  if(adapt) {
    qDebug("Threshold before: %d", m_threshold);
    adaptThreshold();
    qDebug("Threshold after: %d", m_threshold);
  }


  if(m_dark_bg)
    output_image = input_image >= m_threshold;
  else
    output_image = input_image < m_threshold;

}

void Segmenting::adaptThreshold()
{
  int old_threshold;
  do {
    old_threshold = m_threshold;
    Mat above = input_image >= m_threshold;
    Mat below = input_image < m_threshold;
    Scalar mean_above = mean(input_image, above);
    Scalar mean_below = mean(input_image, below);
    m_threshold = cvRound(((mean_above[0]+mean_below[0])/2));

  } while(old_threshold != m_threshold);
}

void Segmenting::splitMerge()
{
  /**
   * We need to have image dimensions be a power of two, so we can
   * keep subdividing it when doing the split part. To ensure this,
   * create a new image that is the nearest larger power of two in
   * each dimension, and copy the original image into it. Use the old
   * image dimensions as an ROI when doing the segmentation
   * afterwards.
   */
  Size size = input_image.size();
  uint32_t new_h = Util::nearest_pow(size.height);
  uint32_t new_w = Util::nearest_pow(size.width);
  uint32_t new_x = (new_w-size.width)/2;
  uint32_t new_y = (new_h-size.height)/2;

  Mat resized = Mat::zeros(new_h, new_w, input_image.type());
  Mat roi(resized, Rect(new_x, new_y, size.width, size.height));
  input_image.copyTo(roi);

  output_image = resized;
}


void Segmenting::setMode(const Mode mode)
{
  m_mode = mode;
  process();
}

void Segmenting::setDarkBG(const bool bg)
{
  m_dark_bg = bg;
  process();
}
