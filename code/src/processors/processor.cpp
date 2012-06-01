/**
 * processor.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-03-29
 */

#include "processor.h"
#include "util.h"
#include <QMetaProperty>

Processor::Processor(QObject *parent)
  :QThread(parent)
{
  abort = false;
  restart = false;
  once = false;
  uses_colour = false;
  connect(this, SIGNAL(updated()), this, SLOT(saveOutput()));
}

Processor::~Processor()
{
  mutex.lock();
  abort = true;
  condition.wakeOne();
  mutex.unlock();

  wait();
}

void Processor::process()
{
  QMutexLocker locker(&mutex);
  abort = false;

  if(once) {
    mutex.unlock();
    run();
  } else if(!isRunning()) {
    start(LowPriority);
  } else {
    restart = true;
    condition.wakeOne();
  }
}

void Processor::cancel()
{
  QMutexLocker locker(&mutex);
  abort = true;
  condition.wakeOne();
}


void Processor::run()
{
  forever {
    if(abort) return;
    if(once) return;
    mutex.lock();
    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}

void Processor::run_once()
{
  once = true;
  run();
}

void Processor::set_input(const Mat img)
{
  QMutexLocker locker(&mutex);
  if(!uses_colour && !img.empty()) {
    cvtColor(img,input_image,CV_RGB2GRAY);
  } else {
    input_image = img;
  }
}

void Processor::set_input_name(QString filename)
{
  QMutexLocker locker(&mutex);
  input_image_filename = filename;
}

Mat Processor::get_output()
{
  QMutexLocker locker(&mutex);
  return output_image;
}

void Processor::addPOI(QPoint p)
{
  QMutexLocker locker(&mutex);
  Point pt(p.x(), p.y());
  POIs.append(pt);
  emit poiCountUpdated();
}

void Processor::deletePOI(QPoint p)
{
  QMutexLocker locker(&mutex);
  Point pt(p.x(), p.y());
  POIs.removeOne(pt);
  emit poiCountUpdated();
}

QFileInfo Processor::imageOutput()
{
  QMutexLocker l(&mutex);
  return image_output_file;
}

void Processor::setImageOutput(QFileInfo path)
{
  QMutexLocker l(&mutex);
  if(path.filePath() == image_output_file.filePath()) return;
  image_output_file = path;
}

void Processor::saveOutput()
{
  QMutexLocker l(&mutex);
  QString filename = image_output_file.filePath();
  if(!filename.isEmpty()) {
	 // if (uses_colour){
	//	  Util::save_image_colour(output_image, filename);
	//  } else {
		  Util::save_image(output_image, filename);
	 // }
  }
}

// Add extra dynamic properties from a meta object (used when replicating
// another processors functionality within a processor)
void Processor::addPropertiesFrom(Processor *other)
{
  const QMetaObject *thisMeta = metaObject();
  const QMetaObject *otherMeta = other->metaObject();
  for(int i = otherMeta->propertyOffset(); i < otherMeta->propertyCount(); i++) {
    QMetaProperty prop = otherMeta->property(i);
    if(prop.isUser()) { // The property editor only uses user properties, so restrict to this
      if(thisMeta->indexOfProperty(prop.name()) == -1) {
        setProperty(prop.name(), other->property(prop.name()));
      }
    }
  }
}

// Sets compatible properties from another processor object
void Processor::setPropertiesFrom(Processor *other)
{
  const QMetaObject *thisMeta = metaObject();
  const QMetaObject *otherMeta = other->metaObject();
  QList<QByteArray> dynProps = dynamicPropertyNames();
  QList<QByteArray> otherDynProps = other->dynamicPropertyNames();

  // Regular properties are stored in QMetaObject
  for(int i = otherMeta->propertyOffset(); i < otherMeta->propertyCount(); i++) {
    QMetaProperty prop = otherMeta->property(i);
    if(prop.isUser()) { // The property editor only uses user properties, so restrict to this
      if(thisMeta->indexOfProperty(prop.name()) > -1 || dynProps.contains(prop.name())) {
        setProperty(prop.name(), other->property(prop.name()));
      }
    }
  }

  // Dynamic properties are not, so copy them separately
  foreach(QByteArray dynProp, otherDynProps) {
    if(thisMeta->indexOfProperty(dynProp) > -1 || dynProps.contains(dynProp)) {
      setProperty(dynProp, other->property(dynProp));
    }
  }
}
