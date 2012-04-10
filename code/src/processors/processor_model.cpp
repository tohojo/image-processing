#include "processor_model.h"
#include "null_processor.h"
#include "segmenting.h"
#include "feature_points.h"
#include "calibration_processor.h"
#include "distortion-removal.h"

ProcessorModel::ProcessorModel() : QAbstractListModel()
{
  create_processors();
}

ProcessorModel::~ProcessorModel()
{
}

int ProcessorModel::rowCount(  const QModelIndex & parent) const
{
  if(!parent.isValid())
    return m_processors.count();
  return 0;
}

QVariant ProcessorModel::data(const QModelIndex & index, int role) const
{
  if(role == Qt::DisplayRole) {
    return get_processor(index.row())->name();
  }
  return QVariant();
}

QVariant ProcessorModel::headerData ( int section, Qt::Orientation orientation,
                                      int role) const
{
  if(section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return QVariant(tr("Processor function"));
  return QVariant();
}

Processor * ProcessorModel::get_processor(int index) const
{
  return m_processors.at(index);
}

void ProcessorModel::create_processors()
{
  m_processors.append(new NullProcessor());
  m_processors.append(new Segmenting());
  m_processors.append(new FeaturePoints());
  m_processors.append(new CalibrationProcessor());
  m_processors.append(new DistortionRemoval());
}

int ProcessorModel::index_for(QString name)
{
  for(int i = 0; i < m_processors.size(); i++)
    if(m_processors[i]->name() == name) return i;
  return -1;
}
