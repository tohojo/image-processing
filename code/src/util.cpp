/**
 * Utility functions for conversion between opencv and qt formats etc.
 */
#include "util.h"
#include <QTextStream>
#include <QRegExp>
#include <QDebug>
#include <fstream>

namespace Util {

  const QImage mat_to_qimage(Mat img)
  {
    if(img.empty()) return QImage();

    if(img.depth() == CV_8U && img.channels() == 1) {
      // Convert grey-scale picture into 8-bit indexed colour.
      QVector<QRgb> colorTable;
      for(int i=0; i < 256; i++)
        colorTable.append(qRgb(i,i,i));

      QImage qImage;

      uchar *img_buffer = (uchar*) malloc(img.rows * img.cols);
      uchar *img_pointer = img_buffer;
      for (int i=0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {
          *img_pointer++ = img.at<uchar>(i,j);
        }
      }
      // Use variant of QImage constructor that specifies the number
      // of bytes pr line. Otherwise the image gets all weird.
      qImage = QImage(img_buffer, img.cols, img.rows, img.cols, QImage::Format_Indexed8);
      qImage.setColorTable(colorTable);
      return qImage;
    } else if(img.depth() == CV_8U && img.channels() == 3) {
      Mat *channels = new Mat[3];
      split(img, channels);
      uchar *img_buffer = new uchar[img.rows * img.cols *4];
      uchar *img_pointer = img_buffer;
      for (int i=0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {
          *img_pointer++ = channels[0].at<uchar>(i,j);
          *img_pointer++ = channels[1].at<uchar>(i,j);
          *img_pointer++ = channels[2].at<uchar>(i,j);
          *img_pointer++ = 255; // alpha channel
        }
      }

      return QImage(img_buffer, img.cols, img.rows, QImage::Format_RGB32);
    } else {
      qFatal("Could not process image of depth %d with %d channels",
             img.depth(), img.channels());
      return QImage();
    }
  }


  Mat load_image(QString filename)
  {
    QByteArray bytes = filename.toLocal8Bit();
    qDebug("Loading file: %s", bytes.constData());
    return imread(bytes.constData(), 0);
  }

  Mat load_image_colour(QString filename)
  {
    QByteArray bytes = filename.toLocal8Bit();
    qDebug("Loading file (colour): %s", bytes.constData());
    return imread(bytes.constData(), 1);
  }

  void save_image(Mat img, QString filename)
  {
    QByteArray bytes = filename.toLocal8Bit();
    qDebug("Saving file: %s", bytes.constData());
    imwrite(bytes.constData(), img);
  }

  /**
   * Find the nearest power of two smaller than the number given.
   * Works by subtracting one, setting all bits to 1, then adding
   * 1 to make the number a power of two.
   */
  uint32_t nearest_pow (uint32_t num, bool smaller)
  {
    uint32_t n = num > 0 ? num - 1 : 0;

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return (n==num) ? n : ((smaller) ? n >> 1 : n);
  }

  void img_to_csv (const char *filename, Mat mat)
  {
    std::ofstream file;
    file.open(filename);
    for(int i = 0; i < mat.rows; i++) {
      for(int j = 0; j < mat.cols; j++) {
        file << (int) mat.at<uchar>(i,j);
        if(j < mat.cols-1) file << ",";
      }
      file << "\n";
    }

    file.close();

  }

  /**
   * Write a float-valued matrix to an IOdevice.
   */
  void write_matrix(Mat mat, QIODevice *dev)
  {
    QTextStream out(dev);
    out << QString("Matrix %1x%2:\n").arg(mat.rows).arg(mat.cols);
    for(int i = 0; i < mat.rows; i++) {
      for(int j = 0; j < mat.cols; j++) {
        out << mat.at<double>(i,j);
        if(j < mat.cols-1) out << ';';
      }
      out << '\n';
    }
  }

  QString format_matrix_float(Mat mat)
  {
    QString formatted;
    QTextStream stream(&formatted, QIODevice::WriteOnly);
    stream.setRealNumberNotation(QTextStream::FixedNotation);
    stream.setRealNumberPrecision(2);
    stream.setFieldAlignment(QTextStream::AlignRight);
    for(int i = 0; i < mat.rows; i++) {
      for(int j = 0; j < mat.cols; j++) {
        stream << qSetFieldWidth(4) << mat.at<float>(i,j) << " ";
      }
      stream << qSetFieldWidth(0) << endl;
    }

    return formatted;
  }

  /**
   * Read a float-valued matrix written by write_matrix.
   *
   * Assumes (and checks) that the matrix to write to is the same dimensionality
   * as the matrix in the file.
   *
   * Returns true if successful.
   */
  bool read_matrix(Mat mat, QIODevice *dev)
  {
    // Peek at the heading to see if we have the right dimensionality.
    char buffer[30];
    if(dev->peek(buffer, sizeof(buffer)) == -1) {
      qWarning() << "Unable to peek at input";
      return false;
    }
    QString header(buffer);
    QRegExp rx("^Matrix (\\d+)x(\\d+):");
    int rows = 0,cols = 0;
    if(rx.indexIn(header) == -1) {
      qWarning() << "RegExp mismatch in header.";
      return false;
    }
    rows = rx.cap(1).toInt();
    cols = rx.cap(2).toInt();
    if(rows != mat.rows || cols != mat.cols) {
      qWarning() << "Dimension mismatch. Matrix rows:" << mat.rows << "cols:" << mat.cols << "Input rows:" << rows << "cols:" << cols;
      return false;
    }
    Mat out(rows, cols, CV_32F);
    header = QString(dev->readLine());
    for(int i = 0; i < rows; i++) {
      QString line(dev->readLine());
      if(line.isEmpty()) {
        qWarning() << "Ran out of data prematurely.";
        return false;
      }
      for(int j = 0; j < cols; j++) {
        bool ok;
        float value = line.section(";", j, j).toFloat(&ok);
        out.at<float>(i,j) = value;
        if(!ok) {
          qWarning() << "Unable to parse float value:" << line.section(";", j, j);
          return false;
        }
      }
    }
    out.copyTo(mat);
    return true;
  }


  Mat combine(Mat l, Mat r)
  {
    const int gap = 5;
    int rows = qMax(l.rows, r.rows);
    assert(l.type() == r.type());
    Mat combined = Mat::ones(rows, l.cols+r.cols+gap, l.type());
    combined *= 255;

    Mat roi_l(combined, Rect(0, 0, l.cols, l.rows));
    Mat roi_r(combined, Rect(l.cols+gap, 0, r.cols, r.rows));

    l.copyTo(roi_l);
    r.copyTo(roi_r);

    return combined;
  }

  QList<Point> read_POIs(QIODevice *dev)
  {
    QList<Point> points;
    QTextStream stream(dev);
    int x,y;
    bool x_ok, y_ok;
    QRegExp r("[,\\s]+");
    int c = 0;
    while(!stream.atEnd()) {
      QString line = stream.readLine();
      x = line.section(r, 0, 0).toInt(&x_ok);
      y = line.section(r, 1, 1).toInt(&y_ok);
      if(x_ok && y_ok) {
        points << Point(x,y);
        c++;
      }
    }
    qDebug() << "Loaded" << c << "POIs from file";
    return points;

  }

  bool comparePointsX(const Point p1, const Point p2)
  {
    return p1.x < p2.x;
  }
}
