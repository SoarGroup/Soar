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

#include <functional>
#include <map>
#include <string>

#include "sml_Events.h"

namespace sml
{

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
    typedef std::string(*StringEventHandler)(smlStringEventId id, void* pUserData, Kernel* pKernel, char const* pString) ;

// Handler for XML events.  The data for the event is passed back in pXML.
// NOTE: To keep a copy of the ClientXML* you are passed use ClientXML* pMyXML = new ClientXML(pXML) to create
// a copy of the object.  This is very efficient and just adds a reference to the underlying XML message object.
// You need to delete ClientXML objects you create and you should not delete the pXML object you are passed.
    typedef void (*XMLEventHandler)(smlXMLEventId id, void* pUserData, Agent* pAgent, ClientXML* pXML) ;


// TODO: move to Kernel::AddRhsHandler, which clients actually see. Expand on why this is the way it is.
// Maintainer note: RhsEventHandler and ClientMessageHandler require special handling:
// RETURN: returns pointer to buff.  If buff is not large enough, it will return NULL and set buffSize to a new size. Reallocate the new size and call again.
// Implementations should save a static value locally to be returned when the client retries the call. This also means that the client
// MUST re-do the call if NULL was returned, or else the next call will return an unrelated value.
// Recommended implementation:
// // at beginning of function:
// static std::string prevResult;
// if ( !prevResult.empty() )
// {
//     strncpy( buf, prevResult.c_str(), *bufSize );
//     prevResult = "";
//     return buf;
// }
// ...
// // at end of function
// if ( resultString.length() + 1 > *bufSize )
// {
//     *bufSize = resultString.length() + 1;
//     prevResult = resultString;
//     return NULL;
// }
// strcpy( buf, resultString.c_str() );
// return buf;

// Handler for RHS (right hand side) function firings
// pFunctionName and pArgument define the RHS function being called (the client may parse pArgument to extract other values)
// The return value is a string which allows the RHS function to create a symbol: e.g. ^att (exec plus 2 2) producing ^att 4
// SEE MAINTAINER NOTE ABOVE!
    typedef char const *(*RhsEventHandler)(smlRhsEventId id, void* pUserData, Agent* pAgent,
                                          char const* pFunctionName, char const* pArgument, int *buffSize, char *buff) ;

    using RhsEventHandlerCPP = std::function<std::string(smlRhsEventId id, Agent *pAgent, char const *pFunctionName, char const *pArgument)>;

    // Handler for a generic "client message".  The content is determined by the client sending this data.
    // The message is sent as a simple string and the response is also a string.  The string can contain data that is intended to be parsed,
    // such as a simple series of integers up to a complete XML message.
    // SEE MAINTAINER NOTE ABOVE!
    // TODO: add alternative similar to RhsEventHandlerCPP above
    typedef char const *(*ClientMessageHandler)(smlRhsEventId id, void *pUserData, Agent *pAgent,
                                                char const *pFunctionName, char const *pArgument, int *buffSize, char *buff);

    // We'll store a handler function together with a generic pointer to data of the user's choosing
    // (which is then passed back into the handler when the event occurs).
    // We also include a callback "id" which is a unique way to refer to this callback--used during unregistering.
    class EventHandlerPlusData
    {
        public:
            int             m_EventID ;     // E.g. smlEVENT_BEFORE_SHUTDOWN
            void*           m_UserData ;    // Arbitrary data from the user which we pass back to them
            int             m_CallbackID ;  // A unique identifier for this callback (used to unregister)

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

            int     getEventID()
            {
                return m_EventID ;
            }
            // TODO: get rid of this in favor of storing in std::function captures
            void*   getUserData()
            {
                return m_UserData ;
            }
            int     getCallbackID()
            {
                return m_CallbackID ;
            }
    } ;

}   // End of namespace

#endif  // Header
