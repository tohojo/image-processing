#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QImage>
#include <QtGui/QImageWriter>
#include <QtGui/QPixmap>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QBitmap>
#include <QtGui/QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTime>
#include <QtCore/QSettings>
#include <QtCore/QMetaProperty>
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
  connect(action_Draw_lines, SIGNAL(toggled(bool)), current_image, SLOT(setPOILines(bool)));

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
    connect(output_view, SIGNAL(left()),
            processor_model->get_processor(i), SLOT(left()));
    connect(output_view, SIGNAL(right()),
            processor_model->get_processor(i), SLOT(right()));
  }

  // Start out by selection first index.
  processor_selection->setCurrentIndex(processor_model->index(0), QItemSelectionModel::SelectCurrent);

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

  // Saving and loading images & POIs
  connect(action_open_image, SIGNAL(activated()), this, SLOT(open_image()));
  connect(actionSaveOutput, SIGNAL(activated()), this, SLOT(save_output()));
  connect(actionSavePOIs, SIGNAL(activated()), this, SLOT(save_POIs()));
  connect(actionLoadPOIs, SIGNAL(activated()), this, SLOT(load_POIs()));

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
      int idx = current_processor->metaObject()->indexOfProperty(i.key().toLocal8Bit().data());
      if(idx == -1) {
        qWarning() << "Non-existing property:" << i.key();
      } else {
        QMetaProperty property = current_processor->metaObject()->property(idx);
        if(QString(property.typeName()) == "QFileInfo") {
          QFileInfo info(i.value().toString());
          if(!current_processor->setProperty(i.key().toLocal8Bit(), QVariant::fromValue(info))) {
            qWarning() << "Error setting QFileInfo property: " << i.key() << info.filePath();
          }
        } else if(!current_processor->setProperty(i.key().toLocal8Bit(), i.value())) {
          qWarning() << "Error setting property: " << i.key() << i.value();
        }
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
  input_filename = QFileDialog::getOpenFileName(this, tr("Select image"),
                                                  open_directory,
                                                  tr("Images (*.png *.jpg *.jpeg *.tif)"));
  if(!input_filename.isNull()) {
    load_image(input_filename);
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

  input_image = Util::load_image_colour(filename);
  current_processor->set_input_name(filename); // The PCA processor requires the filename to be set first
  current_processor->set_input(input_image);
  QImage qImg = Util::mat_to_qimage(input_image);
  input_view->setImage(qImg);
  input_label->setText(QString("%1 - %2x%3px").arg(fileinfo.fileName()).arg(qImg.width()).arg(qImg.height()));
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

void ProcessingGUI::save_output()
{
  if(current_processor->get_output().empty()) {
    QMessageBox msgbox(QMessageBox::Critical, tr("No output to save"),
                       tr("Currently no processor output, so nothing so save."),
                       QMessageBox::Ok, this);
    msgbox.exec();
    return;
  }
  QString filename = QFileDialog::getSaveFileName(this, tr("Select file name"),
                                          open_directory,
                                          tr("Images (*.png *.jpg *.jpeg *.tif)"));
  if(!filename.isNull()) {
    save_image(filename);
  }
}

void ProcessingGUI::save_image(QString filename)
{
  QImage img = Util::mat_to_qimage(current_processor->get_output());
  QFileInfo info(filename);

  QImageWriter writer(filename);
  if(!writer.write(img)) {
    if(m_batch) {
      qFatal("Unable to save output to '%s': %s.",
             filename.toLocal8Bit().data(), writer.errorString().toLocal8Bit().data());
      return;
    }
    QMessageBox msgbox(QMessageBox::Critical, tr("Unable to save image"),
                       tr("The output image could not be saved to '%1':\n%2.").arg(filename).arg(writer.errorString()),
                       QMessageBox::Ok, this);
    msgbox.exec();
  } else{
    qDebug() << "Output image saved to:" << filename;
  }
}

void ProcessingGUI::save_POIs()
{
  if(current_processor->poiCount() == 0) {
    QMessageBox msgbox(QMessageBox::Critical, tr("No POIs to save"),
                       tr("No POIs selected, so nothing so save."),
                       QMessageBox::Ok, this);
    msgbox.exec();
    return;
  }
  QString filename = QFileDialog::getSaveFileName(this, tr("Select file name"),
                                               QString("%1/%2.txt").arg(open_directory).arg(QFileInfo(input_filename).baseName()),
                                               tr("Text files (*.txt)"));
  if(!filename.isNull()) {
    write_POIs(filename);
  }
}

void ProcessingGUI::load_POIs()
{
  if(current_processor->get_output().empty()) {
    QMessageBox msgbox(QMessageBox::Critical, tr("No image loaded"),
                       tr("Unable to load POIs: No processor output."),
                       QMessageBox::Ok, this);
    msgbox.exec();
    return;
  }

  if(current_processor->poiCount() > 0) {
    QMessageBox msgbox(QMessageBox::Question, tr("Replace existing POIs?"),
                       tr("POIs already exist. Replace with loaded, or keep them (and load additional POIs from file)?"),
                       QMessageBox::NoButton, this);
    QPushButton* btn = msgbox.addButton("&Replace", QMessageBox::AcceptRole);
    msgbox.setDefaultButton(btn);
    msgbox.addButton("&Keep", QMessageBox::RejectRole);
    msgbox.exec();
    if(msgbox.clickedButton() == btn) {
      current_image->clearPOIs();
    }
  }

  QString filename = QFileDialog::getOpenFileName(this, tr("Select POI file"),
                                                  open_directory,
                                                  tr("Text files (*.txt)"));
  if(!filename.isNull()) {
    read_POIs(filename);
  }
}

void ProcessingGUI::write_POIs(QString filename)
{
  QFile file(filename);
  if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    if(m_batch) {
      qFatal("Unable to save POIs to '%s': %s.",
             filename.toLocal8Bit().data(), file.errorString().toLocal8Bit().data());
      return;
    }
    QMessageBox msgbox(QMessageBox::Critical, tr("Unable to save POIs"),
                       tr("The POIs could not be saved to '%1':\n%2.").arg(filename).arg(file.errorString()),
                       QMessageBox::Ok, this);
    msgbox.exec();
  } else {
    QTextStream stream(&file);
    QList<Point> POIs = current_processor->getPOIs();
    foreach(Point p, POIs) {
      stream << p.x << "," << p.y << "\n";
    }
    qDebug() << "POIs saved to file:" << filename;
  }
}

void ProcessingGUI::read_POIs(QString filename)
{
  QFile file(filename);
  if(!file.open(QIODevice::ReadOnly)) {
    if(m_batch) {
      qFatal("Unable to read POIs from '%s': %s.",
             filename.toLocal8Bit().data(), file.errorString().toLocal8Bit().data());
      return;
    }
    QMessageBox msgbox(QMessageBox::Critical, tr("Unable to load POIs"),
                       tr("The POIs could not be read from '%1':\n%2.").arg(filename).arg(file.errorString()),
                       QMessageBox::Ok, this);
    msgbox.exec();
  } else {
    QTextStream stream(&file);
    int x,y;
    bool x_ok, y_ok;
    QRegExp r("[,\\s]+");
    int c = 0;
    int height = current_processor->get_output().rows;
    int width = current_processor->get_output().cols;
    QList<Point> POIs = current_processor->getPOIs();
    QList<Point> newPOIs = Util::read_POIs(&file);
    foreach(Point p, newPOIs) {
      if(POIs.contains(p)) {
        qDebug("POI already exists: (%d,%d)", p.x, p.y);
      } else if(p.x > width-1 || p.y > height-1) {
        qDebug("POI out of bounds: (%d,%d)", p.x, p.y);
      } else {
        current_image->addPOI(QPoint(p.x,p.y));
        c++;
      }
    }
    qDebug() << "Added" << c << "POIs from file:" << filename;
  }
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
      current_processor->set_input_name(input_filename);
      current_processor->run_once();
      return;
    }
    m_properties->clear();
    m_properties->addObject(current_processor);

    current_processor->set_input(input_image);
    current_processor->set_input_name(input_filename);
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
