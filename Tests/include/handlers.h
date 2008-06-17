#ifndef HANDLERS_H
#define HANDLERS_H

#include <string>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_Utils.h"

class Handlers
{
public:
	// callbacks
	static void MyBoolShutdownHandler( sml::smlSystemEventId id, void* pUserData, sml::Kernel* pKernel );
	static void MyEventShutdownHandler( sml::smlSystemEventId id, void* pUserData, sml::Kernel* pKernel );
	static void MyDeletionHandler( sml::smlAgentEventId id, void* pUserData, sml::Agent* pAgent );
	static void MySystemEventHandler( sml::smlSystemEventId id, void* pUserData, sml::Kernel* pKernel );
	static void MyCreationHandler( sml::smlAgentEventId id, void* pUserData, sml::Agent* pAgent );
	static void MyProductionHandler( sml::smlProductionEventId id, void* pUserData, sml::Agent* pAgent, char const* pProdName, char const* pInstantiation );
	static std::string MyClientMessageHandler( sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessageType, char const* pMessage );
	static std::string MyFilterHandler( sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessageType, char const* pCommandLine );
	static void MyRunEventHandler( sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase );
	static void MyUpdateEventHandler( sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags );
	static void MyOutputNotificationHandler( void* pUserData, sml::Agent* pAgent);
	static void MyRunSelfRemovingHandler( sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase );
	static std::string MyStringEventHandler( sml::smlStringEventId id, void* pUserData, sml::Kernel* pKernel, char const* pData );
	static void MyDuplicateRunEventHandler( sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase );
	static void MyPrintEventHandler( sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage );
	static void MyXMLEventHandler( sml::smlXMLEventId id, void* pUserData, sml::Agent* pAgent, sml::ClientXML* pXML );
	static void MyInterruptHandler( sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase );
	static std::string MyRhsFunctionHandler( sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pFunctionName, char const* pArgument );
	static void MyMemoryLeakUpdateHandlerDestroyChildren( sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags );
	static void MyMemoryLeakUpdateHandler( sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags );
	static void MyCallStopOnUpdateEventHandler( sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags );

private:
	static void MyMemoryLeakUpdateHandlerInternal( bool destroyAll, sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags );

	// This class is meant to be used to contain static functions only and never instantiated.
	Handlers() {}
	Handlers(const Handlers&) {}
};

#endif // HANDLERS_H
