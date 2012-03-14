#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QBitmap>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include "processing-gui.h"
#include "util.h"

ProcessingGUI::ProcessingGUI(QWidget *parent)
  : QMainWindow(parent)
{
  setupUi(this);

  output_scene = new QGraphicsScene(this);
  output_view->setScene(output_scene);

  connect(action_open_image, SIGNAL(activated()), this, SLOT(open_image()));
  connect(output_zoom, SIGNAL(sliderMoved(int)), this, SLOT(zoom_output(int)));
}

ProcessingGUI::~ProcessingGUI()
{
  delete output_scene;
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
  QImage img = Util::mat_to_qimage(output_image);
  QGraphicsPixmapItem *image = output_scene->addPixmap(QPixmap::fromImage(img));
  output_scene->setSceneRect(image->boundingRect());
  output_view->ensureVisible(image->boundingRect());
}


void ProcessingGUI::open_image()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Select image"),
                                                  QDir::currentPath(),
                                                  tr("Images (*.png *.jpg *.jpeg *.tif)"));
  if(!filename.isNull()) {
    QFileInfo fileinfo = QFileInfo(filename);
    output_image = input_image = Util::load_image(filename);
    QImage qImg = Util::mat_to_qimage(input_image);
    input_view->setImage(qImg);
    input_filename->setText(fileinfo.fileName());
    update_output();
  }
}
