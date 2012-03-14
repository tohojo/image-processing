#ifndef PROCESSING_GUI_H
#define PROCESSING_GUI_H

#include <QtGui/QApplication>
#include <cv.h>
#include "ui_processing-gui.h"
#include "processor.h"
#include "null_processor.h"

using namespace cv;

class ProcessingGUI : public QMainWindow, private Ui::MainWindow
{
  Q_OBJECT

public:
  ProcessingGUI(QWidget *parent =0);
  ~ProcessingGUI();

private:
  Mat input_image;
  Processor *current_processor;
  QGraphicsScene *input_scene;
  QGraphicsScene *output_scene;

private slots:
  void zoom_output(int value);
  void open_image();
  void update_output();

signals:
  void image_changed();
};

#endif
