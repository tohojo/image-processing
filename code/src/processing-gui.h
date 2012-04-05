#ifndef PROCESSING_GUI_H
#define PROCESSING_GUI_H

#include <QtGui/QApplication>
#include <cv.h>
#include "ui_processing-gui.h"
#include "processor.h"
#include "null_processor.h"
#include "processor_model.h"

using namespace cv;

class ProcessingGUI : public QMainWindow, private Ui::MainWindow
{
  Q_OBJECT

public:
  ProcessingGUI(QWidget *parent =0);
  ~ProcessingGUI();

  void set_args(QMap<QString, QVariant> arguments);
  void show();

private:
  Mat input_image;
  bool m_inprogress;
  bool m_batch;
  QString open_directory;
  QMap<QString, QVariant> args;
  Processor *current_processor;
  QGraphicsScene *input_scene;
  QGraphicsScene *output_scene;
  ProcessorModel *processor_model;
  QItemSelectionModel *processor_selection;

public slots:
  void set_processor(Processor *proc);
  void newMessage(QString msg);

private slots:
  void zoom_output(int value);
  void open_image();
  void load_image(QString filename);
  void update_output();
  void new_processor(const QModelIndex & current);
  void setProgress(int value);
  void process_button_clicked();

signals:
  void image_changed();
};

#endif
