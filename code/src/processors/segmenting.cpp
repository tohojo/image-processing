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
  Size size = input_image.size();
  uint32_t new_h = Util::nearest_smaller_pow(size.height);
  uint32_t new_w = Util::nearest_smaller_pow(size.width);
  uint32_t new_x = (size.width-new_w)/2;
  uint32_t new_y = (size.height-new_h)/2;
  qDebug("h,w,x,y = %d,%d,%d,%d", new_h, new_w, new_x, new_y);
  Mat roi(input_image, Rect(new_x, new_y, new_w, new_h));

  output_image = roi;
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
