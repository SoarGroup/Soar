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

#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "gSKI_Enumerations.h"

#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <list>
#include <map>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif

namespace sml {

class KernelSML ;
class Connection ;

// The list of connections interested in an event
typedef std::list< Connection* >		ConnectionList ;
typedef ConnectionList::iterator	ConnectionListIter ;

// Mapping from the event to the list of connections listening to that event
typedef std::map< egSKIEventId, ConnectionList* >	EventMap ;
typedef EventMap::iterator						EventMapIter ;

class EventManager
{
protected:
	// Map from event id to list of connections listening to that event
	EventMap		m_EventMap ;

protected:
	ConnectionList*	GetListeners(egSKIEventId eventID) ;

public:
	virtual ~EventManager() ;

	// Clear the map and release all listeners
	virtual void Clear() ;

	// Returns true if this is the first connection listening for this event
	virtual bool BaseAddListener(egSKIEventId eventID, Connection* pConnection) ;

	// Returns true if this is the first connection listening for this event
	// (Generally calls BaseAddListener and then registers with the kernel for this event)
	virtual bool AddListener(egSKIEventId eventID, Connection* pConnection) = 0 ;

	// Returns true if just removed the last listener
	virtual bool BaseRemoveListener(egSKIEventId eventID, Connection* pConnection) ;

	// Returns true if just removed the last listener
	// (Generally calls BaseRemoveListener and then unregisters with the kernel for this event)
	virtual bool RemoveListener(egSKIEventId eventID, Connection* pConnection) = 0 ;

	// Remove all listeners that this connection has
	virtual void RemoveAllListeners(Connection* pConnection) ;

	virtual ConnectionListIter	GetBegin(egSKIEventId) ;
	virtual ConnectionListIter  GetEnd(egSKIEventId)	;
} ;

} // End of namespace

#endif	// End of header ifdef
