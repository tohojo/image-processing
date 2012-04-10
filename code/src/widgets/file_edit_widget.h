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
public:
  FileEditWidget(QWidget *parent =0);
  FileEditWidget(const QString &contents, QWidget *parent =0);
  ~FileEditWidget();

  QString text();

public slots:
  void setText(const QString &text);

private slots:
  void lineeditFinished();
  void lineeditChanged(const QString &text);
  void lineeditEdited(const QString &text);
  void selectFile();

private:
  void init();

signals:
  void editingFinished();
  void textChanged(const QString & text);
  void textEdited(const QString &text);
};
#endif
