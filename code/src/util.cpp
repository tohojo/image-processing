/**
 * Utility functions for conversion between opencv and qt formats etc.
 */
#include "util.h"

namespace Util {

  const QImage mat_to_qimage(Mat img)
  {
    if(img.depth() == CV_8U && img.channels() == 1) {
      // Convert grey-scale picture into 8-bit indexed colour.
      QVector<QRgb> colorTable;
      for(int i=0; i < 256; i++)
        colorTable.append(qRgb(i,i,i));

      QImage qImage;

      Size size = img.size();
      qDebug("Image size: %dx%d pixels", size.width, size.height);

      if(img.isContinuous()) {
        qDebug("Image is continuous, reusing buffer for QImage");
        const uchar *ptr = img.ptr<uchar>(0);
        qImage = QImage(ptr, size.width, size.height, QImage::Format_Indexed8);
      } else {
        qDebug("Image is not continuous, copying line-by-line");
        uchar *img_buffer = (uchar*) malloc(size.width * size.height);
        uchar *img_pointer = img_buffer;
        for (int i=0; i < size.height; i++) {
          const uchar *ptr = img.ptr<uchar>(i);
          memcpy(img_pointer, ptr, size.width);
          img_pointer += size.width;
        }
        qDebug("Three random pixels: %d, %d, %d",
               img_buffer[10], img_buffer[123], img_buffer[124234]);
        qDebug("QImage pixel values: %d, %d",
               qImage.pixelIndex(10, 0), qImage.pixelIndex(123,0));

        QImage qImage = QImage(img_buffer, size.width, size.height, QImage::Format_Indexed8);
      }
      qImage.setColorTable(colorTable);
      return qImage;
    } else {
      qFatal("Could not process image of depth %d with %d channels",
             img.depth(), img.channels());
      return QImage();
    }
  }


  Mat load_image(QString filename)
  {
    QByteArray bytes = filename.toUtf8();
    qDebug("Loading file: %s", bytes.constData());
    return imread(bytes.constData(), 0);
  }
}
