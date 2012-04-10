#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QBitmap>
#include <QtGui/QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTime>
#include <QtCore/QSettings>
#include <QDebug>

#include "processing-gui.h"
#include "custom_types.h"
#include "util.h"

ProcessingGUI::ProcessingGUI(QWidget *parent)
  : QMainWindow(parent)
{
  setupUi(this);

  CustomTypes::registerTypes();
  m_properties->registerCustomPropertyCB(CustomTypes::createCustomProperty);

  current_processor = NULL;
  m_inprogress = m_batch = false;

  open_directory = QDir::currentPath();

  output_scene = new QGraphicsScene(this);
  output_view->setScene(output_scene);
  current_image = new ImageGraphicsItem();
  output_scene->addItem(current_image);
  connect(this, SIGNAL(image_changed()), current_image, SLOT(clearPOIs()));
  connect(action_Clear_POIs, SIGNAL(activated()), current_image, SLOT(clearPOIs()));

  processor_model = new ProcessorModel();
  processor_selection = new QItemSelectionModel(processor_model);
  processor_view->setModel(processor_model);
  processor_view->setSelectionModel(processor_selection);
  connect(processor_selection, SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(new_processor(const QModelIndex&)));
  connect(processButton, SIGNAL(clicked()), this, SLOT(process_button_clicked()));

  // Connect the POI signals to all processors
  for(int i = 0; i < processor_model->rowCount(); i++) {
    connect(current_image, SIGNAL(newPOI(QPoint)),
            processor_model->get_processor(i), SLOT(addPOI(QPoint)));
    connect(current_image, SIGNAL(POIRemoved(QPoint)),
            processor_model->get_processor(i), SLOT(deletePOI(QPoint)));
  }

  // Start out by selection first index.
  processor_selection->setCurrentIndex(processor_model->index(0), QItemSelectionModel::SelectCurrent);

  connect(action_open_image, SIGNAL(activated()), this, SLOT(open_image()));

  connect(output_zoom, SIGNAL(valueChanged(int)), this, SLOT(zoom_output(int)));
  connect(output_view, SIGNAL(zoomUpdated(int)),
          output_zoom, SLOT(setValue(int)));

  // Dock widgets showing/hiding w/menu
  connect(textDock, SIGNAL(closed(bool)),
          action_Textual_output, SLOT(setChecked(bool)));
  connect(inputDock, SIGNAL(closed(bool)),
          actionInput_image, SLOT(setChecked(bool)));
  connect(processingDock, SIGNAL(closed(bool)),
          actionProcessors, SLOT(setChecked(bool)));
  connect(propertiesDock, SIGNAL(closed(bool)),
          action_Properties, SLOT(setChecked(bool)));

  readSettings();

}

ProcessingGUI::~ProcessingGUI()
{
  delete output_scene;
  delete processor_selection;
  delete processor_model;
  if(current_processor != NULL)
    delete current_processor;
}

void ProcessingGUI::show()
{
  QMainWindow::show();
  // Make sure the dock widget menu is updated correctly
  action_Textual_output->setChecked(textDock->isVisible());
  actionInput_image->setChecked(inputDock->isVisible());
  actionProcessors->setChecked(processingDock->isVisible());
  action_Properties->setChecked(propertiesDock->isVisible());
}

void ProcessingGUI::readSettings()
{
  QSettings settings("Ben & Toke Inc.", "Image Processing");
  restoreGeometry(settings.value("ProcessingGUI/geometry").toByteArray());
  restoreState(settings.value("ProcessingGUI/windowState").toByteArray());
}

void ProcessingGUI::closeEvent(QCloseEvent * event)
{
  QSettings settings("Ben & Toke Inc.", "Image Processing");
  settings.setValue("ProcessingGUI/geometry", saveGeometry());
  settings.setValue("ProcessingGUI/windowState", saveState());
  QMainWindow::closeEvent(event);
}

void ProcessingGUI::set_args(QMap<QString, QVariant> arguments) {
  args = arguments;

  if(args.contains("batch"))
    m_batch = true;

  if(args.contains("processor")) {
    QString processor = args.value("processor").toString();
    int idx = processor_model->index_for(processor);
    if(idx == -1) {
      if(m_batch) {
        qFatal("Processor '%s' not found.", processor.toLocal8Bit().data());
        return;
      }
      QMessageBox msgbox(QMessageBox::Critical, tr("Processor not found"),
                         tr("The processor '%1' was not found.").arg(processor),
                         QMessageBox::Ok, this);
      msgbox.exec();
    } else {
      processor_selection->setCurrentIndex(processor_model->index(idx),
                                           QItemSelectionModel::SelectCurrent);
    }
  }
  QMap<QString, QVariant> properties = args.value("properties").toMap();
  if(!properties.empty()) {
    QMap<QString, QVariant>::iterator i;
    for(i = properties.begin(); i != properties.end(); ++i) {
      if(!current_processor->setProperty(i.key().toLocal8Bit(), i.value())) {
        qWarning() << "Error setting property: " << i.key();
      }
    }
  }

  if(args.contains("input")) {
    load_image(args.value("input").toString());
  } else if(m_batch) {
    qFatal("No input image given for batch mode.");
  }
}


void ProcessingGUI::zoom_output(int value)
{
  qreal scale = (qreal)value/100.0;
  QTransform transform = QTransform::fromScale(scale, scale);
  output_view->setTransform(transform);
}

void ProcessingGUI::update_output()
{
  if(m_batch) {
    qDebug("Batch processing complete.");
    return;
  }
  QImage img = Util::mat_to_qimage(current_processor->get_output());
  current_image->setPixmap(QPixmap::fromImage(img));
}


void ProcessingGUI::open_image()
{
  filename = QFileDialog::getOpenFileName(this, tr("Select image"),
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
    if(m_batch) {
      qFatal("File '%s' not found.", filename.toLocal8Bit().data());
      return;
    }
    QMessageBox msgbox(QMessageBox::Critical, tr("File not found"),
                       tr("File '%1' was not found.").arg(filename),
                       QMessageBox::Ok, this);
    msgbox.exec();
    return;
  }

  input_image = Util::load_image(filename);
  current_processor->set_input(input_image);
  current_processor->set_input_name(filename);
  QImage qImg = Util::mat_to_qimage(input_image);
  input_view->setImage(qImg);
  input_filename->setText(fileinfo.fileName());
  emit image_changed();

  // Scale the graphics view to leave 15 pixels of air on each side
  // of the image, and recenter it.
  QSize s = qImg.size();
  QRect r = output_view->frameRect();
  float scale = qMin((r.width()-30.0f)/s.width(), (r.height()-30.0f)/s.height());
  scale = qMax(scale, 0.01f);
  scale = qMin(scale, 4.0f);
  QTransform transform = QTransform::fromScale(scale, scale);
  output_view->centerOn(current_image);
  output_view->setTransform(transform);
}

void ProcessingGUI::set_processor(Processor *proc)
{
  if(current_processor != NULL) {
    current_processor->disconnect(this);
    current_processor->disconnect(current_image);
    this->disconnect(current_processor);
    current_processor->cancel();
  }

  current_processor = proc;
  connect(this, SIGNAL(image_changed()), current_processor, SLOT(process()));
  connect(current_processor, SIGNAL(updated()), this, SLOT(update_output()));
  connect(current_processor, SIGNAL(progress(int)), progressBar, SLOT(setValue(int)));
  connect(current_processor, SIGNAL(progress(int)), this, SLOT(setProgress(int)));
  connect(current_processor, SIGNAL(newMessage(QString)), SLOT(newMessage(QString)));
  connect(current_processor, SIGNAL(newPOI(QPoint)), current_image, SLOT(addPOI(QPoint)));
  connect(current_processor, SIGNAL(clearPOIs()), current_image, SLOT(clearPOIs()));
  if(m_batch) {
    current_processor->set_input(input_image);
	current_processor->set_input_name(filename);
    current_processor->run_once();
    return;
  }
  m_properties->clear();
  m_properties->addObject(current_processor);

  current_processor->set_input(input_image);
  current_processor->set_input_name(filename);
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

void ProcessingGUI::newMessage(QString msg)
{
  QString message = QString("%1: %2");
  message = message.arg(QTime::currentTime().toString("hh:mm:ss.zzz"), msg);
  textOutput->appendPlainText(message);
}
