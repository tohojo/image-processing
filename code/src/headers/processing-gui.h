#ifndef PROCESSING_GUI_H
#define PROCESSING_GUI_H

#include <QApplication>
#include "ui_processing-gui.h"

class ProcessingGUI : public QMainWindow, private Ui::MainWindow
{
  Q_OBJECT

    public:
  ProcessingGUI(QWidget *parent =0);
  ~ProcessingGUI();
};

#endif
