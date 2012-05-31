#include "processor_model.h"
#include "null_processor.h"
#ifdef SEGMENTING
  #include "segmenting.h"
#endif

#ifdef FEATURE_POINTS
  #include "feature_points.h"
#endif

#ifdef CALIBRATION
  #include "calibration_processor.h"
#endif

#ifdef DISTORTION
  #include "distortion-removal.h"
#endif

#ifdef RECTIFICATION
  #include "rectification_processor.h"
#endif

#ifdef RESIZING
#include "resizing_processor.h"
#endif

#ifdef STEREO
  #include "stereo_processor.h"
#endif

#ifdef PCA_773
  #include "pca_training_processor.h"
#endif

#ifdef FACE_NORMAL
  #include "face_normalisation_processor.h"
#endif

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
#ifdef SEGMENTING
  m_processors.append(new Segmenting());
#endif
#ifdef FEATURE_POINTS
  m_processors.append(new FeaturePoints());
#endif
#ifdef CALIBRATION
  m_processors.append(new CalibrationProcessor());
#endif
#ifdef DISTORTION
  m_processors.append(new DistortionRemoval());
#endif
#ifdef RECTIFICATION
  m_processors.append(new RectificationProcessor());
#endif
#ifdef RESIZING
  m_processors.append(new ResizingProcessor());
#endif
#ifdef STEREO
  m_processors.append(new StereoProcessor());
#endif
#ifdef PCA_773
  m_processors.append(new PcaTrainingProcessor());
#endif
#ifdef FACE_NORMAL
  m_processors.append(new FaceNormalisationProcessor());
#endif
}

int ProcessorModel::index_for(QString name)
{
  for(int i = 0; i < m_processors.size(); i++)
    if(m_processors[i]->name() == name) return i;
  return -1;
}
