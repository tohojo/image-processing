#include "calibration_processor.h"
#include "util.h"
#include "fast_hessian.h"
#include "threshold_segmenter.h"
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
}

void CalibrationProcessor::calibrate()
{
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
  } else {
    qWarning("Ignoring new POIs in stage 2");
  }
}

void CalibrationProcessor::loadPoints3d()
{
  mutex.lock();
  QString filename = m_points3d_file.canonicalFilePath();
  bool valid = m_points3d_file.isFile();
  mutex.unlock();
  if(!valid) return;
  QList<Point3f> points;

  QLocale::setDefault(QLocale::English);
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  QTextStream in(&file);

  while (!in.atEnd()) {
    QString line = in.readLine();
    Point3f p;
    if(parsePoint(line, &p)) points.append(p);
  }

  mutex.lock();
  m_points3d = points;
  mutex.unlock();
  qDebug("Loaded %d 3D points", points.size());
}

bool CalibrationProcessor::parsePoint(QString line, Point3f *p)
{
  bool ok_x, ok_y, ok_z;
  p->x = line.section(",", 0, 0).toFloat(&ok_x);
  p->y = line.section(",", 1, 1).toFloat(&ok_y);
  p->z = line.section(",", 2, 2).toFloat(&ok_z);
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
