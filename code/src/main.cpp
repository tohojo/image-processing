#include <QtGui/QApplication>
#include <iostream>
#include "processing-gui.h"



int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  ProcessingGUI ui;
  ui.show();
  return app.exec();
}
