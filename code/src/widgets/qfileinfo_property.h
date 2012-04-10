/**
 * qfileinfo_property.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-10
 */

#ifndef QFILEINFO_PROPERTY_H
#define QFILEINFO_PROPERTY_H
#include "Property.h"
#include <QtCore/QFileInfo>

class QFileInfoProperty : public Property
{
  Q_OBJECT

  public:
  QFileInfoProperty(const QString& name = QString(), QObject* propertyObject = 0, QObject* parent = 0);

  QVariant value(int role = Qt::UserRole) const;
  virtual void setValue(const QVariant& value);
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option);
  virtual QVariant editorData(QWidget *editor);
  virtual bool setEditorData(QWidget *editor, const QVariant& data);

public slots:
  void editorFinished();

};

Q_DECLARE_METATYPE(QFileInfo)
#endif
