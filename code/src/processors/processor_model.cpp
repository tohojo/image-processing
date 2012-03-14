#include "processor_model.h"
#include "null_processor.h"
#include "adaptive_segment.h"

ProcessorModel::ProcessorModel() : QAbstractListModel()
{
}

ProcessorModel::~ProcessorModel()
{
}

int ProcessorModel::rowCount(  const QModelIndex & parent) const
{
  if(!parent.isValid())
    return 2;
  return 0;
}

QVariant ProcessorModel::data(const QModelIndex & index, int role) const
{
  if(role == Qt::DisplayRole) {
    if(index.row() == 0) {
      return QVariant(NullProcessor::name());
    } else if(index.row() == 1) {
      return QVariant(AdaptiveSegment::name());
    }
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

Processor * ProcessorModel::get_processor(int index)
{
  switch(index) {
  case 1:
    return new AdaptiveSegment();
  default:
    return new NullProcessor();
  }
}
