/////////////////////////////////////////////////////////////////
// Event handler stuff
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This includes the events that an SML agent can listen for and definitions
// for the handlers that will be called.
/////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_EVENT_ID_H
#define SML_CLIENT_EVENT_ID_H

#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <map>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif
#include <string>

#include "sml_Events.h"

namespace sml {

// Forward declaration
class Agent ;
class Kernel ;
class WMElement ;
class ClientXML ;

// These typedefs all define types of functions.
// For example: typedef void (*X)(type1 arg1, type2 arg2) means we're defining function "X" to take (type1 arg1, type2 arg2) and return void.
// To provide such a handler define a function with this type and pass its address in to the registration function for the event.
// For example, for RunEventHandler you define a function like this:
// void MyRunEventHandler(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase) { do-stuff ; }

// Handler for Run events.
// Passed back the event ID, the agent and the phase together with whatever user data we registered with the client
typedef void (*RunEventHandler)(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase);

// Handler for Agent events (such as creation/destruction etc.).
typedef void (*AgentEventHandler)(smlAgentEventId id, void* pUserData, Agent* pAgent) ;

// Handler for Print events.
typedef void (*PrintEventHandler)(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage) ;

// Handler for Production manager events.
typedef void (*ProductionEventHandler)(smlProductionEventId id, void* pUserData, Agent* pAgent, char const* pProdName, char const* pInstantion) ;

// Handler for System events.
typedef void (*SystemEventHandler)(smlSystemEventId id, void* pUserData, Kernel* pKernel) ;

// Handler for output events.
// You register a specific attribute name (e.g. "move") and when this attribute appears on the output link (^io.output-link.move M3)
// you are passed the working memory element ((I3 ^move M3) in this case) in the callback.  This mimics gSKI's output producer model.
typedef void (*OutputEventHandler)(void* pUserData, Agent* pAgent, char const* pCommandName, WMElement* pOutputWme) ;

// This is a simpler notification event -- it just tells you that some output was received for this agent.
// You then call to the other client side methods to determine what has changed.
typedef void (*OutputNotificationHandler)(void* pUserData, Agent* pAgent) ;

// Handler for Update events.
typedef void (*UpdateEventHandler)(smlUpdateEventId id, void* pUserData, Kernel* pKernel, smlRunFlags runFlags) ;

// Handler for string based events.
typedef std::string (*StringEventHandler)(smlStringEventId id, void* pUserData, Kernel* pKernel, char const* pString) ;

// Handler for XML events.  The data for the event is passed back in pXML.
// NOTE: To keep a copy of the ClientXML* you are passed use ClientXML* pMyXML = new ClientXML(pXML) to create
// a copy of the object.  This is very efficient and just adds a reference to the underlying XML message object.
// You need to delete ClientXML objects you create and you should not delete the pXML object you are passed.
typedef void (*XMLEventHandler)(smlXMLEventId id, void* pUserData, Agent* pAgent, ClientXML* pXML) ;

// Handler for RHS (right hand side) function firings
// pFunctionName and pArgument define the RHS function being called (the client may parse pArgument to extract other values)
// The return value is a string which allows the RHS function to create a symbol: e.g. ^att (exec plus 2 2) producting ^att 4
// NOTE: This is the one place in clientSML where we use a std::string in an interface.  If you wish to compile with a pure "C" interface
// this can be replaced by a handler that is passed a buffer and a length.  The length is passed within the framework already (from the kernel to here)
// so this is an easy transition.
typedef std::string (*RhsEventHandler)(smlRhsEventId id, void* pUserData, Agent* pAgent,
								char const* pFunctionName, char const* pArgument) ;

// Handler for a generic "client message".  The content is determined by the client sending this data.
// The message is sent as a simple string and the response is also a string.  The string can contain data that is intended to be parsed,
// such as a simple series of integers up to a complete XML message.
typedef std::string (*ClientMessageHandler)(smlRhsEventId id, void* pUserData, Agent* pAgent,
								char const* pClientName, char const* pMessage) ;

// We'll store a handler function together with a generic pointer to data of the user's choosing
// (which is then passed back into the handler when the event occurs).
// We also include a callback "id" which is a unique way to refer to this callback--used during unregistering.
class EventHandlerPlusData
{
public:
	int				m_EventID ;		// E.g. smlEVENT_BEFORE_SHUTDOWN
	void*			m_UserData ;	// Arbitrary data from the user which we pass back to them
	int				m_CallbackID ;	// A unique identifier for this callback (used to unregister)

public:
	
	EventHandlerPlusData()
	{
		m_EventID = 0;
		m_UserData = 0;
		m_CallbackID = 0;
	}

	EventHandlerPlusData(int eventID, void* pData, int callbackID)
	{
		m_EventID    = eventID ;
		m_UserData   = pData ;
		m_CallbackID = callbackID ;
	}

	int		getEventID()	{ return m_EventID ; }
	void*	getUserData()   { return m_UserData ; }
	int		getCallbackID() { return m_CallbackID ; }
} ;

} 	// End of namespace

#endif	// Header
