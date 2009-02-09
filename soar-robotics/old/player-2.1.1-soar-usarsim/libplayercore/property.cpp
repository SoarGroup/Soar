/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/********************************************************************
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************/

#include "property.h"
#include <libplayercore/playercommon.h>
#include <libplayercore/player.h>
#include <libplayercore/configfile.h>
#include <libplayercore/error.h>
#include <libplayercore/driver.h>

#include <stdlib.h>
#include <string.h>

Property::Property (void)
	: key (NULL), readonly(false)
{
}

Property::Property (const char *newKey, bool readOnly)
	: readonly(readOnly)
{
	if ((key = strdup (newKey)) == NULL)
	{
		PLAYER_ERROR1 ("Failed to allocate memory to store property key %s", newKey);
		key = NULL;
	}
}

Property::~Property (void)
{
	if (key != NULL)
		free (key);
}

void Property::SetKey (const char *newKey)
{
	if (key != NULL)
		free (key);

	if ((key = strdup (newKey)) == NULL)
	{
		PLAYER_ERROR1 ("Failed to allocate memory to store property key %s", newKey);
		key = NULL;
	}
}

const bool Property::KeyIsEqual (const char *rhs)
{
	if (!strcmp (key, rhs))
		return true;
	return false;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

IntProperty::IntProperty (const char *newKey, int newValue, bool readOnly)
	: Property (newKey, readOnly), value (newValue)
{
}

IntProperty::IntProperty (const char *newKey, int newValue, bool readOnly, Driver * driver, ConfigFile*cf, int section)
: Property (newKey, readOnly), value (newValue)
{
	driver->RegisterProperty(newKey, this, cf, section);
}


void IntProperty::SetValue (int newValue)
{
	value = newValue;
}

void IntProperty::GetValueToMessage (void *data) const
{
	reinterpret_cast<player_intprop_req_t*> (data)->value = value;
}

void IntProperty::SetValueFromMessage (const void *data)
{
	if (readonly)
	{
		PLAYER_WARN2 ("Property %s is read only, cannot change value %d", key, reinterpret_cast<const player_intprop_req_t*> (data)->value);
		return;
	}

	value = reinterpret_cast<const player_intprop_req_t*> (data)->value;
}

bool IntProperty::ReadConfig (ConfigFile *cf, int section)
{
	// Read an integer from the config file section, using the current prop value as the default
	value = cf->ReadInt (section, key, value);

	return true;
}

const IntProperty& IntProperty::operator= (const IntProperty &rhs)
{
	value = rhs.GetValue ();
	return *this;
}

int IntProperty::operator= (int rhs)
{
	value = rhs;
	return value;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DoubleProperty::DoubleProperty (const char *newKey, double newValue, bool readOnly)
	: Property (newKey, readOnly), value (newValue)
{
}

DoubleProperty::DoubleProperty (const char *newKey, double newValue, bool readOnly, Driver * driver, ConfigFile*cf, int section)
: Property (newKey, readOnly), value (newValue)
{
	driver->RegisterProperty(newKey, this, cf, section);
}


void DoubleProperty::SetValue (double newValue)
{
	value = newValue;
}

void DoubleProperty::GetValueToMessage (void *data) const
{
	reinterpret_cast<player_dblprop_req_t*> (data)->value = value;
}

void DoubleProperty::SetValueFromMessage (const void *data)
{
	if (readonly)
	{
		PLAYER_WARN2 ("Property %s is read only, cannot change value %f", key, reinterpret_cast<const player_dblprop_req_t*> (data)->value);
		return;
	}

	value = reinterpret_cast<const player_dblprop_req_t*> (data)->value;
}

bool DoubleProperty::ReadConfig (ConfigFile *cf, int section)
{
	// Read a double from the config file section, using the current prop value as the default
	value = cf->ReadFloat (section, key, value);

	return true;
}

const DoubleProperty& DoubleProperty::operator= (const DoubleProperty &rhs)
{
	value = rhs.GetValue ();
	return *this;
}

double DoubleProperty::operator= (double rhs)
{
	value = rhs;
	return value;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

StringProperty::StringProperty (const char *newKey, const char *newValue, bool readOnly)
	: Property (newKey, readOnly)
{
	if (newValue != NULL)
	{
		if ((value = strdup (newValue)) == NULL)
		{
			PLAYER_ERROR1 ("Failed to allocate memory to store property value %s", newValue);
			value = NULL;
		}
	}
	else
		value = NULL;
}

StringProperty::StringProperty (const char *newKey, const char * newValue,  bool readOnly, Driver * driver, ConfigFile*cf, int section)
: Property (newKey, readOnly)
{
	if (newValue != NULL)
	{
		if ((value = strdup (newValue)) == NULL)
		{
			PLAYER_ERROR1 ("Failed to allocate memory to store property value %s", newValue);
			value = NULL;
		}
	}
	else
		value = NULL;
	driver->RegisterProperty(newKey, this, cf, section);

}


StringProperty::~StringProperty (void)
{
	if (value != NULL)
		free (value);
}

void StringProperty::SetValue (const char *newValue)
{
	if (value != NULL)
		free (value);

	if (newValue != NULL)
	{
		if ((value = strdup (newValue)) == NULL)
		{
			PLAYER_ERROR1 ("Failed to allocate memory to store property value %s", newValue);
			value = NULL;
		}
	}
	else
		value = NULL;
}

void StringProperty::GetValueToMessage (void *data) const
{
	player_strprop_req_t *req = reinterpret_cast<player_strprop_req_t*> (data);

	if (value == NULL)
		req->value = NULL;
	else if ((req->value = strdup (value)) == NULL)
	{
		PLAYER_ERROR1 ("Failed to allocate memory to store property value %s", value);
		req->value = NULL;
		req->value_count = 0;
	}
	req->value_count = strlen (req->value) + 1;	// +1 to make sure the NULL byte is copied by pack/unpack functions
}

void StringProperty::SetValueFromMessage (const void *data)
{
	if (readonly)
	{
		PLAYER_WARN2 ("Property %s is read only, cannot change value %s", key, reinterpret_cast<const player_strprop_req_t*> (data)->value);
		return;
	}

	const player_strprop_req_t *req = reinterpret_cast<const player_strprop_req_t*> (data);
	if (value != NULL)
		free (value);
	if ((value = strdup (req->value)) == NULL)
	{
		PLAYER_ERROR1 ("Failed to allocate memory to store property value %s", req->value);
		value = NULL;
	}
}

bool StringProperty::ReadConfig (ConfigFile *cf, int section)
{
	const char *temp = cf->ReadString (section, key, NULL);
	if (temp != NULL)
	{
		if (value != NULL)
			free (value);

		if ((value = strdup (temp)) == NULL)
		{
			PLAYER_ERROR1 ("Failed to allocate memory to store property value %s", temp);
			value = NULL;
			return false;
		}
	}

	return true;
}

const StringProperty& StringProperty::operator= (const StringProperty &rhs)
{
	if (value != NULL)
		free (value);

	if (rhs.GetValue () == NULL)
		value = NULL;
	else if ((value = strdup (rhs.GetValue ())) == NULL)
	{
		PLAYER_ERROR1 ("Failed to allocate memory to store property value %s", rhs.GetValue ());
		value = NULL;
	}
	return *this;
}

const char* StringProperty::operator= (const char* rhs)
{
	if (readonly)
	{
		PLAYER_WARN2 ("Property %s is read only, cannot change value 50 %s", key, rhs);
		return value;
	}

	if (value != NULL)
		free (value);

	if (rhs == NULL)
		value = NULL;
	else if ((value = strdup (rhs)) == NULL)
	{
		PLAYER_ERROR1 ("Failed to allocate memory to store property value %s", rhs);
		value = NULL;
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

PropertyBag::PropertyBag (void)
	: firstProperty (NULL)
{
}

PropertyBag::~PropertyBag (void)
{
	if (firstProperty != NULL)
	{
		// Trash the linked list
		PropertyNode *currentProp = firstProperty;
		while (firstProperty != NULL)
		{
			firstProperty = currentProp->next;
			if (currentProp->key != NULL)
				free (currentProp->key);
			delete currentProp;
			currentProp = firstProperty;
		}
	}
}

bool PropertyBag::AddProperty (const char *key, Property *property)
{
	if (firstProperty == NULL)
	{
		if ((firstProperty = new PropertyNode) == NULL)
		{
			PLAYER_ERROR ("Failed to allocate memory for property node");
			return false;
		}
		if ((firstProperty->key = strdup (key)) == NULL)
		{
			PLAYER_ERROR1 ("Failed to allocate memory for property key: %s", key);
			delete firstProperty;
			firstProperty = NULL;
			return false;
		}
		firstProperty->property = property;
		firstProperty->next = NULL;
	}
	else
	{
		// Walk to the end of the list, checking for an existing property as we go
		PropertyNode *lastProperty = firstProperty;
		while (lastProperty != NULL)
		{
			if (strcmp (lastProperty->key, key) == 0)
			{
				PLAYER_ERROR1 ("Property already registered: %s", key);
				return false;
			}
			if (lastProperty->next == NULL)
			{
				// This is the end of the list, break before we lose the pointer
				// Note that the while loop doesn't do this check because then
				// it wouldn't check the last item on the list to see if it's the
				// same key as the one being registered
				break;
			}
			lastProperty = lastProperty->next;
		}

		// Add the new property at this position (which should be the end of the list)
		if ((lastProperty->next = new PropertyNode) == NULL)
		{
			PLAYER_ERROR ("Failed to allocate memory for property node");
			return false;
		}
		if ((lastProperty->next->key = strdup (key)) == NULL)
		{
			PLAYER_ERROR1 ("Failed to allocate memory for property key: %s", key);
			delete lastProperty->next;
			lastProperty->next = NULL;
			return false;
		}
		lastProperty = lastProperty->next;
		lastProperty->property = property;
		lastProperty->next = NULL;
	}

	return true;
}

Property* PropertyBag::GetProperty (const char *key)
{
	PropertyNode *currentProp = firstProperty;

	while (currentProp != NULL)
	{
		if (strcmp (currentProp->key, key) == 0)
			return currentProp->property;
		currentProp = currentProp->next;
	}
	// Property wasn't found
	PLAYER_ERROR1 ("Property not registered: %s", key);
	return NULL;
}
