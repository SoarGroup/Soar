/////////////////////////////////////////////////////////////////
// KernelListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This class's HandleEvent method is called when
// specific events occur within the kernel:
/*
*      // System events
*      gSKIEVENT_BEFORE_SHUTDOWN            = 1,
*      gSKIEVENT_AFTER_CONNECTION_LOST,
*      gSKIEVENT_BEFORE_RESTART,
*      gSKIEVENT_AFTER_RESTART,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_ADDED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_ADDED,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_REMOVED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_REMOVED,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_EXECUTED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_EXECUTED,
*/////////////////////////////////////////////////////////////////
#ifndef KERNEL_LISTENER_H
#define KERNEL_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Kernel.h"
#include "sml_EventManager.h"

#include <map>

namespace sml {

class Connection ;

// Mapping from a rhs function name to the list of connections implementing that function
typedef std::map<std::string, ConnectionList*>	RhsMap ;
typedef RhsMap::iterator						RhsMapIter ;

class KernelListener : public gSKI::ISystemListener, public gSKI::IAgentListener, public gSKI::IRhsListener, public EventManager
{
protected:
	// Mapping from a rhs function name to the list of connections implementing that function
	RhsMap	m_RhsMap ;

	// Record if we've registered with the kernel to listen to RHS functions
	bool m_bListeningRHS ;

	// The kernel
	KernelSML* m_pKernelSML ;

public:
	KernelListener()
	{
		m_bListeningRHS = false ;
	}

	virtual ~KernelListener()
	{
		Clear() ;
	}

	// Initialize this listener
	void Init(KernelSML* pKernelSML) ;

	// Release memory
	virtual void Clear() ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIEventId eventID, Connection* pConnection) ;

	// Returns true if this is the first connection listening for this function name
	void AddRhsListener(char const* pFunctionName, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this function name
	void RemoveRhsListener(char const* pFunctionName, Connection* pConnection) ;

	virtual void RemoveAllListeners(Connection* pConnection) ;

	// Returns the list of connections listening for this RHS function to fire
	ConnectionList* GetRhsListeners(char const* pFunctionName) ;

	// Called when a "SystemEvent" occurs in the kernel
	virtual void HandleEvent(egSKIEventId eventId, gSKI::IKernel* kernel) ;

	// Called when an "AgentEvent" occurs in the kernel
	virtual void HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr) ;

	// Called when a "RhsEvent" occurs in the kernel
	virtual bool HandleEvent(egSKIEventId eventId, gSKI::IAgent* pAgent, bool commandLine, char const* pFunctionName, char const* pArgument,
						     int maxLengthReturnValue, char* pReturnValue) ;

	virtual bool ExecuteCommandLine(gSKI::IAgent* pAgent, char const* pFunctionName, char const* pArgument, int maxLengthReturnValue, char* pReturnValue) ;

protected:
	bool IsSystemEvent(egSKIEventId id)
	{
		switch (id)
		{
		case gSKIEVENT_BEFORE_SHUTDOWN:
		case gSKIEVENT_AFTER_CONNECTION_LOST:
		case gSKIEVENT_BEFORE_RESTART:
		case gSKIEVENT_AFTER_RESTART:
		case gSKIEVENT_BEFORE_RHS_FUNCTION_ADDED:
		case gSKIEVENT_AFTER_RHS_FUNCTION_ADDED:
		case gSKIEVENT_BEFORE_RHS_FUNCTION_REMOVED:
		case gSKIEVENT_AFTER_RHS_FUNCTION_REMOVED:
		case gSKIEVENT_BEFORE_RHS_FUNCTION_EXECUTED:
		case gSKIEVENT_AFTER_RHS_FUNCTION_EXECUTED:
			return true ;
		}

		return false ;
	}

	bool IsAgentEvent(egSKIEventId id)
	{
		switch (id)
		{
			case gSKIEVENT_AFTER_AGENT_CREATED:
			case gSKIEVENT_BEFORE_AGENT_DESTROYED:
			case gSKIEVENT_BEFORE_AGENT_REINITIALIZED:
			case gSKIEVENT_AFTER_AGENT_REINITIALIZED:
				return true ;
			default:
				break;
		}
		return false ;
	}
} ;

}

#endif
