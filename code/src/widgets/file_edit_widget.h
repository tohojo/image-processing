/**
 * file_edit_widget.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-10
 */

#ifndef FILE_EDIT_WIDGET_H
#define FILE_EDIT_WIDGET_H

#include <QtGui/QWidget>
#include "ui_file-edit-widget.h"

class FileEditWidget : public QWidget, private Ui::FileEditWidget
{
  Q_OBJECT
  Q_PROPERTY(QString text READ text WRITE setText USER true)
  Q_PROPERTY(QString filetype READ filetype WRITE setFiletype)
  Q_PROPERTY(OpenType opentype READ opentype WRITE setOpentype)
  Q_ENUMS(OpenType)
public:
    enum OpenType { READ, WRITE, DIR };
  FileEditWidget(QWidget *parent =0);
  FileEditWidget(const QString &contents, QWidget *parent =0);
  ~FileEditWidget();

  QString text();
  QString filetype() {return file_type;}
  OpenType opentype() {return open_type;}

public slots:
  void setText(const QString &text);
  void setFiletype(const QString &fileType);
  void setOpentype(const OpenType type) {open_type = type;}

private slots:
  void lineeditFinished();
  void lineeditChanged(const QString &text);
  void lineeditEdited(const QString &text);
  void selectFile();

private:
  void init();
  QString file_filter;
  QString file_type;
  OpenType open_type;

signals:
  void editingFinished();
  void textChanged(const QString & text);
  void textEdited(const QString &text);
};
#endif
