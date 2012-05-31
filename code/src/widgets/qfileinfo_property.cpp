/**
 * qfileinfo_property.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-10
 */

#include "qfileinfo_property.h"
#include <QtGui/QMessageBox>
#include <QDebug>

QFileInfoProperty::QFileInfoProperty(const QString& name /*= QString()*/,
                                     QObject* propertyObject /*= 0*/,
                                     QObject* parent /*= 0*/)
  : Property(name, propertyObject, parent)
{
  open_type = FileEditWidget::READ;
}

QVariant QFileInfoProperty::value(int role) const
{
  QVariant data = Property::value();
  if (data.isValid() && role != Qt::UserRole) {
    return data.value<QFileInfo>().filePath();
  }
  return data;
}

void QFileInfoProperty::setValue(const QVariant& value)
{
  if (value.type() == QVariant::String) {
    QString v = value.toString();
    QFileInfo info(v);
    if(open_type == FileEditWidget::READ && !v.isEmpty() && !info.isFile()) {
      qWarning() << tr("The file '%1' was not found.").arg(v);
    } else {
      Property::setValue(QVariant::fromValue(info));
    }
  }
  else
    Property::setValue(value);
}

QWidget * QFileInfoProperty::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/)
{
  FileEditWidget *editor = new FileEditWidget(parent);
  //  connect(editor, SIGNAL(editingFinished()), this, SLOT(editorFinished()));
  return editor;
}

bool QFileInfoProperty::setEditorData(QWidget *editor, const QVariant &data)
{
  static_cast<FileEditWidget*>(editor)->setText(data.toString());
  return true;
}

QVariant QFileInfoProperty::editorData(QWidget *editor)
{
  return QVariant(static_cast<FileEditWidget*>(editor)->text());
}


void QFileInfoProperty::editorFinished()
{
  QVariant value_editor = static_cast<FileEditWidget*>(QObject::sender())->text();
  setValue(value_editor);
}

void QFileInfoProperty::setEditorHints(const QString& hints)
{
  QRegExp rx("(.*)(=\\s*)(.*)(;{1})");
  rx.setMinimal(true);
  int pos = 0;
  while ((pos = rx.indexIn(hints, pos)) != -1) {
    QString name = rx.cap(1).trimmed();
    QString value = rx.cap(3).trimmed();
    if(name == "opentype") {
      if(value == "WRITE") {
        open_type = FileEditWidget::WRITE;
      } else if (value == "DIR") {
        open_type = FileEditWidget::DIR;
      }
    }
    pos += rx.matchedLength();
  }
  Property::setEditorHints(hints);
}
