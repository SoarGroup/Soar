#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// EventManager class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This class manages a list of connections which are interested
// in being notified when a particular event occurs in the kernel.
//
/////////////////////////////////////////////////////////////////

#include "sml_EventManager.h"

using namespace sml ;

EventManager::~EventManager()
{
	Clear() ;
}

void EventManager::Clear()
{
	for (EventMapIter mapIter = m_EventMap.begin() ; mapIter != m_EventMap.end() ; mapIter++)
	{
		egSKIEventId eventID = mapIter->first ;
		ConnectionList* pList = mapIter->second ;

		// Can't walk through with a normal iterator because we're deleting
		// the items from the list as we go.  We do all this so that ultimately
		// the listener into the kernel will be removed (if RemoveListener for the specific
		// event class wishes to do so).
		while (pList->size() > 0)
		{
			Connection* pConnection = pList->front() ;
			RemoveListener(eventID, pConnection) ;
		}

		delete pList ;
	}

	m_EventMap.clear() ;
}

// Record that a particular connection wants to listen in on this event.
bool EventManager::AddListener(egSKIEventId eventID, Connection* pConnection)
{
	EventMapIter mapIter = m_EventMap.find(eventID) ;

	ConnectionList* pList = NULL ;

	// Either create a new list or retrieve the existing list of listeners
	if (mapIter == m_EventMap.end())
	{
		pList = new ConnectionList() ;
		m_EventMap[eventID] = pList ;
	}
	else
	{
		pList = mapIter->second ;
	}

	pList->push_back(pConnection) ;

	// Return true if this is the first listener for this event
	return (pList->size() == 1) ;
}

// Find the list of connections listening to this event (or NULL)
// The returned list can be empty.
ConnectionList*	EventManager::GetListeners(egSKIEventId eventID)
{
	EventMapIter mapIter = m_EventMap.find(eventID) ;

	if (mapIter == m_EventMap.end())
		return NULL ;

	return mapIter->second ;
}

// Returns true if just removed the last listener
bool EventManager::RemoveListener(egSKIEventId eventID, Connection* pConnection)
{
	ConnectionList* pList = GetListeners(eventID) ;

	// We have no record of anyone listening for this event
	// That's not an error -- it's OK to call this for all events in turn
	// to make sure a connection is removed completely.
	if (pList == NULL || pList->size() == 0)
		return false ;

	pList->remove(pConnection) ;

	// Return true if the list is now empty
	return pList->empty() ;
}

// Remove all listeners that this connection has
void EventManager::RemoveAllListeners(Connection* pConnection)
{
	// Remove any listeners for this connection
	// We do this for all possible events even though only some will
	// be valid for this particular event manager.
	// We could make this more efficient by defining the set for each handler
	// but the cost of removing all should be minimal as this is a rare event.
	for (int i = 1 ; i < gSKIEVENT_LAST ; i++)
	{
		egSKIEventId id = (egSKIEventId)i ;
		RemoveListener(id, pConnection) ;
	}
}

ConnectionListIter EventManager::GetBegin(egSKIEventId eventID)
{
	ConnectionList* pList = GetListeners(eventID) ;

	// If nobody is listening return NULL.
	// Key is that this must match the value returned by GetEnd()
	// in the same situation.
	if (!pList)
		return NULL ;

	return pList->begin() ;
}

ConnectionListIter EventManager::GetEnd(egSKIEventId eventID)
{
	ConnectionList* pList = GetListeners(eventID) ;

	if (!pList)
		return NULL ;

	return pList->end() ;
}

