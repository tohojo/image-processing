/**
 * file_edit_widget.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-10
 */

#include <QtGui/QFileDialog>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include "file_edit_widget.h"

FileEditWidget::FileEditWidget(QWidget *parent)
  :QWidget(parent)
{
  init();
}

FileEditWidget::FileEditWidget(const QString & contents, QWidget *parent)
  :QWidget(parent)
{
  init();
  setText(contents);
}

void FileEditWidget::init()
{
  setupUi(this);
  setFocusProxy(lineEdit);
  open_type = READ;
  connect(lineEdit, SIGNAL(editingFinished()), SLOT(lineeditFinished()));
  connect(lineEdit, SIGNAL(textChanged(const QString&)),
          SLOT(lineeditChanged(const QString&)));
  connect(lineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(lineeditEdited(const QString&)));
  connect(pushButton, SIGNAL(clicked()), this, SLOT(selectFile()));
}

FileEditWidget::~FileEditWidget()
{
}

QString FileEditWidget::text()
{
  return lineEdit->text();
}

void FileEditWidget::setText(const QString &text)
{
  lineEdit->setText(text);
}

void FileEditWidget::lineeditFinished()
{
  if(!pushButton->hasFocus())
    emit editingFinished();
}

void FileEditWidget::lineeditChanged(const QString &text)
{
  emit textChanged(text);
}

void FileEditWidget::lineeditEdited(const QString &text)
{
  emit textEdited(text);
}

void FileEditWidget::selectFile()
{
  QString val = lineEdit->text();
  QString open_directory = QDir::currentPath();

  if(!val.isEmpty()) {
    QFileInfo info(val);
    if(info.exists()) {
      open_directory = info.dir().path();
    }
  }
  QString filename;
  if(open_type == READ) {
    filename = QFileDialog::getOpenFileName(this, tr("Select file"),
                                                    open_directory,
                                                    file_filter);
  } else if (open_type == WRITE) {
    filename = QFileDialog::getSaveFileName(this, tr("Select file"),
                                                    open_directory,
                                                    file_filter);
  } else if (open_type == DIR) {
    filename = QFileDialog::getExistingDirectory(this, tr("Select directory"),
                                                 open_directory,
                                                 QFileDialog::ShowDirsOnly);
  }
  if(!filename.isNull()) {
    lineEdit->setText(filename);
    emit editingFinished();
  }
  lineEdit->setFocus();
}

void FileEditWidget::setFiletype(const QString& type)
{
  file_type = type;
  if(type == "images") {
    file_filter = tr("Images (*.png *.jpg *.jpeg *.tif)");
  } else if(type == "text") {
    file_filter = tr("Text files (*.txt)");
  } else {
    file_filter = tr("Any file (*.*)");
  }
}
