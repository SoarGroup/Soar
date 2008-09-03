/////////////////////////////////////////////////////////////////
// RhsListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This class's HandleEvent method is called when
// specific events occur within the kernel:
/*
*      gSKIEVENT_RHS_USER_FUNCTION
*/
/////////////////////////////////////////////////////////////////

#ifndef RHS_LISTENER_H
#define RHS_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "sml_EventManager.h"

#include <string>
#include <map>

namespace sml {

class Connection ;

// Mapping from a rhs function name to the list of connections implementing that function
typedef std::map< std::string, ConnectionList* >	RhsMap ;
typedef RhsMap::iterator						RhsMapIter ;

//
// NOTE: this class differs from the other listener classes in that it does not inherit from EventManager
// The reason is because it's signatures for some functions are different (i.e. Add/RemoveListener).
// To emphasize that this is different, the names of some of these functions have actually been changed
//  from the "standard" set by the other listener classes, i.e. AddListener -> AddRhsListener.
// These differences exist because the nature of the Rhs listener is different.  In some sense, RhsListener is
//  more specific than the other listeners because it doesn't need to specify an event (there is only one).
// We could make changes to the EventManager template so that the RhsListener type is handled as a special case,
//  but the only real benefit of that is that we could treat all listener classes as type EventManager.  However,
//  we don't need that capability.
// The other disadvantage of not inheriting from EventManager is that it defines a couple methods that
//  RhsListener doesn't have.  However, RhsListener has no need for these things right now. If that changes in
//  the future, we may revisit this decision.
//
class RhsListener : public gSKI::IRhsListener
{
protected:
	// Mapping from a rhs function name to the list of connections implementing that function
	RhsMap	m_RhsMap ;

	// Record if we've registered with the kernel to listen to RHS functions
	bool m_bListeningRHS ;

	// The kernel
	KernelSML* m_pKernelSML ;

public:
	RhsListener()
	{
		m_bListeningRHS = false ;
	}

	virtual ~RhsListener()
	{
		Clear() ;
	}

	// Initialize this listener
	void Init(KernelSML* pKernelSML) ;

	// Release memory
	virtual void Clear() ;

	// Returns true if this is the first connection listening for this function name
	void AddRhsListener(char const* pFunctionName, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this function name
	void RemoveRhsListener(char const* pFunctionName, Connection* pConnection) ;

	virtual void RemoveAllListeners(Connection* pConnection) ;

	// Returns the list of connections listening for this RHS function to fire
	ConnectionList* GetRhsListeners(char const* pFunctionName) ;

	// Called for a filter event
	bool HandleFilterEvent(egSKIRhsEventId eventID, gSKI::Agent* pAgent, char const* pArgument,
						    int maxLengthReturnValue, char* pReturnValue) ;

	// Called when a "RhsEvent" occurs in the kernel
	virtual bool HandleEvent(egSKIRhsEventId eventId, gSKI::Agent* pAgent, bool commandLine, char const* pFunctionName, char const* pArgument,
						     int maxLengthReturnValue, char* pReturnValue) ;

	virtual bool ExecuteCommandLine(gSKI::Agent* pAgent, char const* pFunctionName, char const* pArgument, int maxLengthReturnValue, char* pReturnValue) ;
} ;

}

#endif
