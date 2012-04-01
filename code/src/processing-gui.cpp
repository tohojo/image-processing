#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QBitmap>
#include <QtGui/QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include "processing-gui.h"
#include "util.h"

ProcessingGUI::ProcessingGUI(QWidget *parent)
  : QMainWindow(parent)
{
  setupUi(this);

  current_processor = NULL;
  m_inprogress = false;

  open_directory = QDir::currentPath();

  output_scene = new QGraphicsScene(this);
  output_view->setScene(output_scene);

  processor_model = new ProcessorModel();
  processor_selection = new QItemSelectionModel(processor_model);
  processor_view->setModel(processor_model);
  processor_view->setSelectionModel(processor_selection);
  connect(processor_selection, SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(new_processor(const QModelIndex&)));
  connect(processButton, SIGNAL(clicked()), this, SLOT(process_button_clicked()));

  // Start out by selection first index.
  processor_selection->setCurrentIndex(processor_model->index(0), QItemSelectionModel::SelectCurrent);

  connect(action_open_image, SIGNAL(activated()), this, SLOT(open_image()));
  connect(output_zoom, SIGNAL(sliderMoved(int)), this, SLOT(zoom_output(int)));
}

void ProcessingGUI::set_args(QMap<QString, QVariant> arguments) {
  args = arguments;

  if(args.contains("input")) {
    load_image(args.value("input").toString());
  }

  if(args.contains("processor")) {
    QString processor = args.value("processor").toString();
    int idx = processor_model->index_for(processor);
    if(idx == -1) {
      QMessageBox msgbox(QMessageBox::Critical, tr("Processor not found"),
                         tr("The processor '%1' was not found.").arg(processor),
                         QMessageBox::Ok, this);
      msgbox.exec();
    } else {
      processor_selection->setCurrentIndex(processor_model->index(idx),
                                           QItemSelectionModel::SelectCurrent);
    }
  }
}

ProcessingGUI::~ProcessingGUI()
{
  delete output_scene;
  delete processor_selection;
  delete processor_model;
  if(current_processor != NULL)
    delete current_processor;
}

void ProcessingGUI::zoom_output(int value)
{
  qreal scale = (qreal)value/100.0;
  QTransform transform = QTransform::fromScale(scale, scale);
  output_view->setTransform(transform);
}

void ProcessingGUI::update_output()
{
  output_scene->clear();
  QImage img = Util::mat_to_qimage(current_processor->get_output());
  QGraphicsPixmapItem *image = output_scene->addPixmap(QPixmap::fromImage(img));
  output_scene->setSceneRect(image->boundingRect());
  output_view->ensureVisible(image->boundingRect());
}


void ProcessingGUI::open_image()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Select image"),
                                                  open_directory,
                                                  tr("Images (*.png *.jpg *.jpeg *.tif)"));
  if(!filename.isNull()) {
    load_image(filename);
  }
}

void ProcessingGUI::load_image(QString filename)
{
  QFileInfo fileinfo = QFileInfo(filename);
  if(fileinfo.dir().exists(".")) {
    open_directory = fileinfo.dir().path();
  }

  if(!fileinfo.exists()) {
    QMessageBox msgbox(QMessageBox::Critical, tr("File not found"),
                       tr("File '%1' was not found.").arg(filename),
                       QMessageBox::Ok, this);
    msgbox.exec();
    return;
  }

  input_image = Util::load_image(filename);
  current_processor->set_input(input_image);
  QImage qImg = Util::mat_to_qimage(input_image);
  input_view->setImage(qImg);
  input_filename->setText(fileinfo.fileName());
  emit image_changed();
}

void ProcessingGUI::set_processor(Processor *proc)
{
  if(current_processor != NULL) {
    current_processor->cancel();
    current_processor->disconnect();
    disconnect(this, 0, current_processor, 0);
  }

  current_processor = proc;
  connect(this, SIGNAL(image_changed()), current_processor, SLOT(process()));
  connect(current_processor, SIGNAL(updated()), this, SLOT(update_output()));
  connect(current_processor, SIGNAL(progress(int)), progressBar, SLOT(setValue(int)));
  connect(current_processor, SIGNAL(progress(int)), this, SLOT(setProgress(int)));
  m_properties->clear();
  m_properties->addObject(current_processor);

  current_processor->set_input(input_image);
  current_processor->process();
}

void ProcessingGUI::new_processor(const QModelIndex & current)
{
  int row = current.row();
  set_processor(processor_model->get_processor(row));
}

void ProcessingGUI::setProgress(int value)
{
  if(value < 100) {
    m_inprogress = true;
    processButton->setText(tr("Cancel processing"));
  } else {
    m_inprogress = false;
    processButton->setText(tr("Re-process"));
  }
}

void ProcessingGUI::process_button_clicked()
{
  if(m_inprogress) {
    current_processor->cancel();
    setProgress(100);
    progressBar->setValue(100);
  } else {
    current_processor->process();
  }
}
