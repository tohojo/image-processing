/**
 * custom_types.h
 *
 * Toke Høiland-Jørgensen
 * 2012-04-10
 */

#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H
#include "Property.h"

namespace CustomTypes
{
	void registerTypes();
	Property* createCustomProperty(const QString& name, QObject* propertyObject, Property* parent);

}
#endif
