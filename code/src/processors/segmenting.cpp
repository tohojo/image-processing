#include <QtCore/QMutableListIterator>
#include "segmenting.h"
#include "util.h"

Segmenting::Segmenting(QObject *parent)
  : Processor(parent)
{
  m_threshold = 0;
  m_dark_bg = false;
  m_mode = GLOBAL_THRESHOLD;
  m_delta = 10;
}

Segmenting::~Segmenting()
{
}

void Segmenting::process()
{
  if(input_image.empty()) return;
  emit progress(0);
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

  emit progress(100);
  emit updated();
}

void Segmenting::thresholdSegment(bool adapt) {

  Scalar m = mean(input_image);
  m_threshold = cvRound(m[0]);

  if(adapt) {
    emit progress(30);
    qDebug("Threshold before: %d", m_threshold);
    adaptThreshold();
    emit progress(70);
    qDebug("Threshold after: %d", m_threshold);
  } else {
    emit progress(50);
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
  emit progress(5);
  Rect imgRect = Rect(new_x, new_y, size.width, size.height);
  Mat roi(resized, imgRect);
  input_image.copyTo(roi);
  emit progress(10);


  QList<Mat> regionMap = splitRegions(resized, true);

  qDebug("Regions returned: %d", regionMap.size());

  QList<Mat> regions = mergeRegions(regionMap);

  qDebug("Regions returned: %d", regions.size());

  output_image = resized;
}

/**
 * Function to split a region into multiple homogeneous regions (no
 * item varies more from the region average than the delta parameter).
 */
QList<Mat> Segmenting::splitRegions(Mat region, bool topLevel) const
{
  QList<Mat> output;
  QList<Rect> rects;
  // Split into four regions.
  Size size = region.size();
  if(size.width == 1 && size.height == 1) {
    output.append(region);
    return output;
  }

  int mid_x = size.width/2;
  int mid_y = size.height/2;

  rects.append(Rect(0, 0, mid_x, mid_y));
  if(size.width > 1)
    rects.append(Rect(mid_x, 0, mid_x, mid_y));
  if(size.width > 1)
    rects.append(Rect(0, mid_y, mid_x, mid_y));
  if(size.width > 1 && size.height > 1)
    rects.append(Rect(mid_x, mid_y, mid_x, mid_y));

  int c_rects = rects.size();
  int c = 0;
  float progress_scale = 40;
  int progress_offset = 10;
  int value = 0;
  foreach(Rect rect, rects) {
    qDebug("Rect: %dx%d %d,%d", rect.width, rect.height, rect.x, rect.y);
    Mat newReg(region, rect);
    if(isHomogeneous(newReg)) {
      output.append(newReg);
      newReg.setTo(value);
      value += 50;
    } else {
      output += splitRegions(newReg);
    }
    if(topLevel)
      emit progress(progress_offset + qRound(progress_scale * (++c/(float)c_rects)));
  }

  qDebug("Returning %d regions", output.size());

  return mergeRegions(output);
}

QList<Mat> Segmenting::mergeRegions(QList<Mat> regions) const
{
  QList<Mat> output;

  QList<Mat>::iterator i;
  QMutableListIterator<Mat> j(regions);
  for(i = regions.begin(); i != regions.end(); ++i) {
    Mat newMat;
    Mat current = (*i);
    j.toFront();
    while(j.hasNext()) {
      Mat test = j.value();
      if(regionsAdjacent(current, test)) {
        newMat = merge(current, test);
        if(isHomogeneous(newMat)) {
          current = newMat;
          j.remove();
        }
      }
    }
    output.append(current);
  }
  return output;
}

bool Segmenting::regionsAdjacent(Mat one, Mat two) const
{
  Size s_one, s_two;
  Point ofs_one, ofs_two;
  one.locateROI(s_one, ofs_one);
  two.locateROI(s_two, ofs_two);
  return (ofs_two.x == ofs_one.x + s_one.width ||
          ofs_two.y == ofs_one.y + s_one.height ||
          ofs_one.x == ofs_two.x + s_two.width ||
          ofs_one.y == ofs_two.y + s_two.height);
}

Mat Segmenting::merge(Mat one, Mat two) const
{
  Size s_one, s_two;
  Point ofs_one, ofs_two;
  one.locateROI(s_one, ofs_one);
  two.locateROI(s_two, ofs_two);
  return one;
}

bool Segmenting::isHomogeneous(Mat region) const
{
  Size s = region.size();
  if(s.height == 1 && s.width == 1) return true;
  double r_min, r_max;
  Scalar r_mean = mean(region);
  double r_meanVal = r_mean[0];
  minMaxLoc(region, &r_min, &r_max);

  return (r_max-r_meanVal < m_delta && r_meanVal-r_min < m_delta);
}


void Segmenting::setMode(const Mode mode)
{
  if(m_mode == mode) return;
  m_mode = mode;
  process();
}

void Segmenting::setDarkBG(const bool bg)
{
  if(m_dark_bg == bg) return;
  m_dark_bg = bg;
  process();
}

void Segmenting::setDelta(const int delta)
{
  if(m_delta == delta) return;
  m_delta = delta;
  process();
}
