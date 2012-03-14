#ifndef PROCESSOR_MODEL_H
#define PROCESSOR_MODEL_H

#include <QAbstractListModel>
#include "processor.h"

class ProcessorModel : public QAbstractListModel
{
  Q_OBJECT
public:
  ProcessorModel();
  ~ProcessorModel();

  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  Processor * get_processor(int index);
};
#endif
