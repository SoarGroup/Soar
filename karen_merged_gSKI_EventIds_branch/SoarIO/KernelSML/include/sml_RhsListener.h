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

	// Called when a "RhsEvent" occurs in the kernel
	virtual bool HandleEvent(egSKIRhsEventId eventId, gSKI::IAgent* pAgent, bool commandLine, char const* pFunctionName, char const* pArgument,
						     int maxLengthReturnValue, char* pReturnValue) ;

	virtual bool ExecuteCommandLine(gSKI::IAgent* pAgent, char const* pFunctionName, char const* pArgument, int maxLengthReturnValue, char* pReturnValue) ;
} ;

}

#endif
