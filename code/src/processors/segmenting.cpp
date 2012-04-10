#include <QtCore/QMutableListIterator>
#include "segmenting.h"
#include "util.h"
#include "threshold_segmenter.h"

Segmenting::Segmenting(QObject *parent)
  : Processor(parent)
{
  m_threshold = 0;
  m_dark_bg = false;
  m_mode = GLOBAL_THRESHOLD;
  m_delta = 50;
}

Segmenting::~Segmenting()
{
}

void Segmenting::run()
{
  forever {
    if(abort) return;
    mutex.lock();
    Mode m = m_mode;
    bool isEmpty = input_image.empty();
    mutex.unlock();
    if(!isEmpty) {
      emit progress(0);
      switch(m) {
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
      if(abort) return;
      if(!restart) {
        emit progress(100);
        emit updated();
      }
    }
    if(once) return;
    mutex.lock();
    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}

void Segmenting::thresholdSegment(bool adapt) {
  QMutexLocker locker(&mutex);

  ThresholdSegmenter seg(input_image, m_dark_bg);
  seg.compute(adapt);
  m_threshold = seg.threshold();
  output_image = seg.output();
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
  mutex.lock();
  Size size = input_image.size();
  uint32_t new_h = Util::nearest_pow(size.height);
  uint32_t new_w = Util::nearest_pow(size.width);

  Mat resized = Mat::zeros(new_h, new_w, input_image.type());
  emit progress(5);
  Rect imgRect = Rect(0, 0, size.width, size.height);
  Mat roi(resized, imgRect);
  input_image.copyTo(roi);
  mutex.unlock();
  emit progress(5);

  if(abort || restart) return;

  QList<IP::Region> regionMap = splitRegions(resized, true);

  if(abort || restart) return;
  qDebug("Regions returned: %d", regionMap.size());

  if(roi.rows != resized.rows || roi.cols != resized.cols) {
    regionMap = filterRegions(regionMap, imgRect);
    qDebug("Filtered: %d", regionMap.size());
  }

  QList<IP::Region> regions = mergeRegions(regionMap, resized);

  if(abort || restart) return;
  qDebug("Regions returned: %d", regions.size());

  colourRegions(regions, resized);
  if(abort || restart) return;

  mutex.lock();
  output_image = Mat(resized, imgRect);
  mutex.unlock();
}

/**
 * Filter out all regions that are completely outside the region of
 * the original image (such regions occur when an image is resized to
 * a power of two for split/merge segmentation).
 */
QList<IP::Region> Segmenting::filterRegions(QList<IP::Region> regions, Rect rect) const
{
  QList<IP::Region> output;
  QList<IP::Region>::iterator i;
  for(i = regions.begin(); i != regions.end(); ++i) {
    IP::RPoint bm = i->minBound();
    if(bm.x() < rect.width && bm.y() < rect.height) {
      output.append(*i);
    }
  }
  return output;
}

/**
 * Function to split a region into multiple homogeneous regions (no
 * item varies more from the region average than the delta parameter).
 */
QList<IP::Region> Segmenting::splitRegions(Mat image, bool topLevel) const
{
  QList<IP::Region> output;
  QList<Rect> rects;
  // Split into four regions.
  Size size = image.size();
  if(size.width == 1 && size.height == 1) {
    output.append(IP::Region(image));
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
  float progress_scale = 20;
  int progress_offset = 5;
  foreach(Rect rect, rects) {
    Mat newReg(image, rect);
    if(isHomogeneous(newReg)) {
      output.append(IP::Region(newReg));
    } else {
      output += splitRegions(newReg);
    }
    if(abort || restart) break;
    if(topLevel)
      emit progress(progress_offset + qRound(progress_scale * (++c/(float)c_rects)));
  }

  return output;
}



/**
 * Merge the regions into as few possible homogeneous regions as
 * possible.
 *
 * The merge strategy consists of taking one region from the input,
 * and iteratively try merging it with every other region until no
 * more regions can be merged in. Merged regions are also removed from
 * the input.
 *
 * This process continues until no more regions are left in the input.
 *
 * This merge process is horribly inefficient, and takes forever to
 * run on a large number of regions (more than a few thousands). Given
 * a better region representation, the search for possible regions to
 * merge with could be limited to regions that are actually adjacent
 * to the current region (i.e. by only searching around the border).
 */
QList<IP::Region> Segmenting::mergeRegions(QList<IP::Region> regions, Mat img) const
{
  QList<IP::Region> output;
  QList<IP::Region> input(regions);
  int i, current_size;
  int input_size = input.size();
  float progress_scale = 70;
  int progress_offset = 25;

  while(!input.empty()) {
    IP::Region current = input.takeFirst();
    do {
      current_size = input.size();
      for(i = 0; i < input.size(); i++) {
        if(abort || restart) return output;
        IP::Region test(input[i]);
        if(current.contains(test)) {
          input.removeAt(i);
          i--;
        } else if(current.adjacentTo(test)) {
          IP::Region newReg(current);
          newReg.add(test);
          if(isHomogeneous(newReg, img)) {
            current = newReg;
            input.removeAt(i);
            i--;
          }
        }
      }
      emit progress(progress_offset +
                    qRound(progress_scale * ((input_size-input.size())/(float)input_size)));
    } while(current_size != input.size()); // Keep looping until no more regions are merged
    output.append(current);
  }
  return output;
}

void Segmenting::colourRegions(QList<IP::Region> regions, Mat img) const
{
  img.setTo(255);
  int colour = 0;
  float progress_scale = 5;
  int progress_offset = 95;
  for(int i = 0; i<regions.size(); i++) {
    Mat mask = regions[i].toMask(img);
    img.setTo(colour, mask);
    colour += 30;
    if(colour > 200) colour = 0;
    emit progress(progress_offset + qRound(progress_scale * (i/(float)regions.size())));
  }
}

bool Segmenting::isHomogeneous(const IP::Region region, const Mat img) const
{
  double r_min, r_max;
  if(!region.isEmpty()) {
    Mat mask = region.toMask(img);
    minMaxLoc(img, &r_min, &r_max, 0, 0, mask);
  } else {
    minMaxLoc(img, &r_min, &r_max);
  }

  bool res = (qRound(r_max-r_min) <= m_delta);
  return res;
}

bool Segmenting::isHomogeneous(const Mat mat) const
{
  return isHomogeneous(IP::Region(), mat);
}


void Segmenting::setMode(const Mode mode)
{
  QMutexLocker locker(&mutex);
  if(m_mode == mode) return;
  m_mode = mode;
  mutex.unlock();
  process();
}

void Segmenting::setDarkBG(const bool bg)
{
  QMutexLocker locker(&mutex);
  if(m_dark_bg == bg) return;
  m_dark_bg = bg;
  mutex.unlock();
  process();
}

void Segmenting::setDelta(const int delta)
{
  QMutexLocker locker(&mutex);
  if(m_delta == delta) return;
  m_delta = delta;
  mutex.unlock();
  process();
}
