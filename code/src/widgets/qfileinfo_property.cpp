/**
 * qfileinfo_property.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-10
 */

#include "qfileinfo_property.h"
#include <QtGui/QMessageBox>

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
    if(!info.isFile()) {
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
