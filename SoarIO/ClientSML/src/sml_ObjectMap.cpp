/////////////////////////////////////////////////////////////////
// ObjectMap class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used to keep a map of objects which we own.
// When the map is deleted, so are the objects within the map.
//
/////////////////////////////////////////////////////////////////

#include "sml_ObjectMap.h"

using namespace sml ;

// Template implementation required to be in the header still.

/*
ObjectMap::ObjectMap(void)
{
}

ObjectMap::~ObjectMap(void)
{
	// Delete the contents of the map
	for(InternalMapIter mapIter = m_Map.begin(); mapIter != m_Map.end(); ++mapIter)
	{
		DataType pObject = mapIter->second ;

		// We only delete the object.  The name is a pointer
		// into the object structure, so it's deleted when the object is deleted.
		delete pObject ;
	}
}

// NOTE: The name of the object being added needs to be owned by the object (we just delete the object at the end)
// Also, if there's already an object with this name in the map, this could cause a leak.  The caller is currently
// in charge of checking for that.
void ObjectMap::add(char const* pName, void* pObject)
{
	m_Map[pName] = pObject ;
}

void* ObjectMap::find(char const* pName) const
{
	InternalMapConstIter mapIter = m_Map.find(pName);

	if (mapIter == m_Map.end())
		return NULL ;

	return mapIter->second ;
}

bool ObjectMap::remove(char const* pName, bool deleteObject)
{
	InternalMapIter mapIter = m_Map.find(pName);

	if (mapIter == m_Map.end())
		return false ;

	DataType pObject = mapIter->second ;

	if (deleteObject)
		delete pObject ;

	m_Map.erase(mapIter) ;

	return true ;
}
*/