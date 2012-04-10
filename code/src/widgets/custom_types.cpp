/**
 * custom_types.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-10
 */

#include "custom_types.h"
#include "qfileinfo_property.h"

namespace CustomTypes
{
  	void registerTypes()
	{
		static bool registered = false;
		if (!registered)
		{
			qRegisterMetaType<QFileInfo>("QFileInfo");
			registered = true;
		}
	}

	Property* createCustomProperty(const QString& name, QObject* propertyObject, Property* parent)
	{
		int userType = propertyObject->property(qPrintable(name)).userType();
		if (userType == QMetaType::type("QFileInfo"))
			return new QFileInfoProperty(name, propertyObject, parent);
		else
			return new Property(name, propertyObject, parent);
	}
}

