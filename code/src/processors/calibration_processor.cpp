#include "calibration_processor.h"
#include "util.h"
#include "fast_hessian.h"
#include "threshold_segmenter.h"
#include "camera_calibrator.h"
#include <QDebug>


CalibrationProcessor::CalibrationProcessor(QObject *parent)
  : Processor(parent)
{
  m_stage = STAGE_1;
}

CalibrationProcessor::~CalibrationProcessor()
{
}

void CalibrationProcessor::run()
{
  forever {
    if(abort) return;
    mutex.lock();
    bool isEmpty = input_image.empty();
    ProcessingStage stage = m_stage;
    mutex.unlock();
    if(!isEmpty) {
      emit progress(0);
      switch(stage) {
      case STAGE_1:
      findPOIs();
      break;
      case STAGE_2:
      loadPoints3d();
      adjustPOIs();
      calibrate();
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

void CalibrationProcessor::findPOIs()
{
  mutex.lock();
  Mat input = input_image;
  ThresholdSegmenter seg(input_image, false);
  seg.compute(true);
  output_image = seg.output();
  emit updated();
  mutex.unlock();

  FastHessian fh(input, 4, 4, 2, 500);
  fh.compute();
  QList<KeyPoint> kps = fh.interestPoints();
  qDebug("Found %d keypoints", kps.size());

  foreach(KeyPoint kp, kps) {
    QPoint p(qRound(kp.pt.x), qRound(kp.pt.y));
    if(!poiExists(p)) emit newPOI(p);
  }

}

void CalibrationProcessor::adjustPOIs()
{
  mutex.lock();
  Mat input = output_image; // threshold ed version from stage 1
  QList<Point> pois(POIs);
  emit clearPOIs();
  mutex.unlock();
  QList<Point> newPois;

  //bool printed = false;
  foreach(Point p, pois) {
    Mat mask = Mat::zeros(input.rows+2, input.cols+2, input.type());
    Rect r;
    floodFill(input, mask, p, 255, &r, 0, 0, FLOODFILL_MASK_ONLY);
    Mat roi(mask, r);
    Mat roi2(input, r);
    Point c = findCentre(roi);
    emit newPOI(QPoint(c.x, c.y));
    qDebug("Adjusting point (%d,%d) -> (%d,%d)", p.x,p.y,c.x,c.y);
    newPois.append(c);
  }

  mutex.lock();
  POIs = newPois;
  mutex.unlock();
}

Point CalibrationProcessor::findCentre(Mat img)
{
  int x_sum = 0, y_sum = 0, x_c = 0, y_c = 0;
  for(int x= 0; x<img.cols; x++) {
    for(int y = 0; y<img.rows; y++) {
      if(img.at<uchar>(y,x)) {
        x_sum += x;
        x_c++;
        y_sum += y;
        y_c++;
      }
    }
  }
  Size s; Point p;
  img.locateROI(s,p);
  if(!x_c || !y_c) return p;
  return Point(p.x+x_sum/x_c, p.y+y_sum/y_c);
}

void CalibrationProcessor::calibrate()
{
  mutex.lock();
  QList<Point3d> points3d(m_points3d);
  QList<Point> pois(POIs);
  int height = input_image.rows;
  int width = input_image.cols;
  mutex.unlock();

  if(points3d.size() != 63 || pois.size() != 63) {
    qWarning("Need 63 points (both 2D and 3D). Unable to continue (have %d/%d)",
             pois.size(), points3d.size());
    return;
  }
  CamCalibrator calib(pois.toStdList(), points3d.toStdList(), width, height);
  calib.mapPtsToCalibrationPts();
  calib.calibrate();
}

bool CalibrationProcessor::poiExists(QPoint p)
{
  foreach(Point pt, POIs) {
    QPoint d = QPoint(pt.x, pt.y)-p;
    if(d.manhattanLength() < 20) return true;
  }
  return false;
}

void CalibrationProcessor::addPOI(QPoint p)
{
  if(m_stage == STAGE_1) {
    Processor::addPOI(p);
  }
}

void CalibrationProcessor::loadPoints3d()
{
  mutex.lock();
  QString filename = m_points3d_file.canonicalFilePath();
  bool valid = m_points3d_file.isFile();
  mutex.unlock();
  if(!valid) return;
  QList<Point3d> points;

  QLocale::setDefault(QLocale::English);
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  QTextStream in(&file);

  while (!in.atEnd()) {
    QString line = in.readLine();
    Point3d p;
    if(parsePoint(line, &p)) points.append(p);
  }

  mutex.lock();
  m_points3d = points;
  mutex.unlock();
  qDebug("Loaded %d 3D points", points.size());
}

bool CalibrationProcessor::parsePoint(QString line, Point3d *p)
{
  bool ok_x, ok_y, ok_z;
  p->x = line.section(",", 0, 0).toDouble(&ok_x);
  p->y = line.section(",", 1, 1).toDouble(&ok_y);
  p->z = line.section(",", 2, 2).toDouble(&ok_z);
  return (ok_x && ok_y && ok_z);
}


void CalibrationProcessor::setStage(ProcessingStage s)
{
  QMutexLocker locker(&mutex);
  if(m_stage == s) return;
  m_stage = s;
  mutex.unlock();
  process();
}

void CalibrationProcessor::setPoints3d(const QFileInfo f)
{
  QMutexLocker locker(&mutex);
  if(f.canonicalFilePath() == m_points3d_file.canonicalFilePath()) return;
  m_points3d_file = f;
  mutex.unlock();
  process();
}
