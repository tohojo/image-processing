#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QBitmap>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include "processing-gui.h"
#include "util.h"

ProcessingGUI::ProcessingGUI(QWidget *parent)
  : QMainWindow(parent)
{
  setupUi(this);

  connect(actionOpen_image, SIGNAL(activated()), this, SLOT(open_image()));
}

ProcessingGUI::~ProcessingGUI()
{
}


void ProcessingGUI::open_image()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Select image"),
                                                  QDir::currentPath(),
                                                  tr("Images (*.png *.jpg *.jpeg)"));
  if(!filename.isNull()) {
    QFileInfo fileinfo = QFileInfo(filename);
    currentImage = Util::load_image(filename);
    QImage qImg = Util::mat_to_qimage(currentImage);
    QImage scaled = qImg.scaled(inputImage->width(), inputImage->height(),
                                Qt::KeepAspectRatio);
    QPixmap pixmap = QBitmap::fromImage(scaled);
    inputImage->setPixmap(pixmap);
    //    inputImage->setText(fileinfo.fileName());
  }
}
