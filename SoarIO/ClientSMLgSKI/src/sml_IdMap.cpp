/////////////////////////////////////////////////////////////////
// IdMap class
//
// Author: Douglas Pearson	www.threepenny.net
// Date  : August 2004
//
// Simple class used to record a list of objects
// that we own.  When this map is destroyed, we delete
// the objects in the map.
//
// If you'd like to have a map, but the objects shouldn't
// be deleted, then make sure to clear it before the destructor is called.
//
/////////////////////////////////////////////////////////////////

#include "sml_IdMap.h"
#include "sml_ClientObject.h"
#include "sml_ClientRelease.h"

using namespace sml ;

IdMap::IdMap()
{
}

IdMap::~IdMap(void)
{
	clear(true) ;
}

void IdMap::clear(bool deleteObjects)
{
	if (deleteObjects)
	{
		// Delete the contents of the map
		for(idMapItr_t mapIter = m_Map.begin(); mapIter != m_Map.end(); ++mapIter)
		{
			ClientObject* pObj = mapIter->second;

			// We only delete the object.  The id (or name) is a pointer
			// into the object structure, so it's deleted when the object is deleted.
			delete pObj ;
		}
	}

	// Delete the map itself
	m_Map.clear();
}

// Go through all objects in the map and call IRelease::Release on them.
// Then optionally delete the entire map.
void IdMap::releaseAll(bool deleteObjects)
{
	// Delete the contents of the map
	for(idMapItr_t mapIter = m_Map.begin(); mapIter != m_Map.end(); ++mapIter)
	{
		IRelease* pObject = (IRelease*)mapIter->second;
		Release::ReleaseObject(pObject) ;
	}

	if (deleteObjects)
		clear(deleteObjects) ;
}

void IdMap::add(ClientObject* pObject)
{
	// BUGBUG?  I think we should check if pObject->GetId() is already in the map.
	// If so, we should delete the old object before adding the new one or this might leak.
	m_Map[pObject->GetId()] = pObject ;
}

// Remove an object from the map (optionally deleting it)
bool IdMap::remove(char const* pID, bool deleteObject)
{
	idMapItr_t mapItr = m_Map.find(pID);

	if (mapItr == m_Map.end())
		return false ;

	ClientObject* pObject = (*mapItr).second ;

	if (deleteObject)
		delete pObject ;

	m_Map.erase(mapItr) ;

	return true ;
}

ClientObject* IdMap::getFirst()
{
	if (m_Map.size() == 0)
		return NULL ;

	return (*m_Map.begin()).second ;
}

ClientObject* IdMap::lookup(char const* pID) const
{
	idMapItrConst_t mapItr = m_Map.find(pID);

	if (mapItr == m_Map.end())
		return NULL ;

	return (*mapItr).second ;
}

