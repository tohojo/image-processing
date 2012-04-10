/**
 * qfileinfo_property.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-10
 */

#include "qfileinfo_property.h"
#include <QtGui/QMessageBox>
#include "file_edit_widget.h"

QFileInfoProperty::QFileInfoProperty(const QString& name /*= QString()*/,
                                     QObject* propertyObject /*= 0*/,
                                     QObject* parent /*= 0*/)
  : Property(name, propertyObject, parent)
{
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
    if(!v.isEmpty() && !info.isFile()) {
      QMessageBox msgbox(QMessageBox::Critical, tr("File not found"),
                         tr("The file '%1' was not found.").arg(v),
                         QMessageBox::Ok);
      msgbox.exec();
      Property::setValue(QVariant(""));
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
  connect(editor, SIGNAL(editingFinished()), this, SLOT(editorFinished()));
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
