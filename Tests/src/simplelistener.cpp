#include "simplelistener.h"

#include <string>
#include <sstream>
#include <iostream>

#include "sml_Utils.h"

extern bool g_Cancel;

std::string listenerRhsFunctionHandler(sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pFunctionName, char const* pArgument){
	// Return the argument we are passed plus some of our text
	//cout << "Received rhs function call with argument: " << pArgument << endl ;

	if ( !pArgument ) return std::string();

	std::stringstream res;
	res << "my listener result " << pArgument;
	return res.str() ;
}

bool SimpleListener::shutdownMessageReceived = false;
std::string SimpleListener::MyClientMessageHandler( sml::smlRhsEventId, void* pUserData, sml::Agent*, char const*, char const* pMessage)
{
	if ( pMessage && std::string( pMessage ) == "shutdown" )
	{
		//std::cout << "SimpleListener got shutdown message." << std::endl;
		shutdownMessageReceived = true;
		return "ok";
	}
	return "unknown command";
}

// Create a process that listens for remote commands and lives
// for "life" 10'ths of a second (e.g. 10 = live for 1 second)
SimpleListener::SimpleListener( int life, int port )
: life( life ), port( port ), useCurrentThread( false )
{
   //std::cout << "Starting listener on " << port << std::endl;
}

int SimpleListener::run()
{
	// Create the kernel instance
	sml::Kernel* pKernel = useCurrentThread ? sml::Kernel::CreateKernelInCurrentThread(sml::Kernel::GetDefaultLibraryName(), false, port) :
											  sml::Kernel::CreateKernelInNewThread(sml::Kernel::GetDefaultLibraryName(), port) ;

	if ( pKernel->HadError() )
	{
		std::cerr << pKernel->GetLastErrorDescription() << std::endl ;
		return 1;
	}

	//std::cout << "SimpleListener alive." << std::endl;

	pKernel->SetConnectionInfo( "listener", "ready", "ready" );

	// Register here so we can test the order that RHS functions are called
	//pKernel->AddRhsFunction( "test-rhs", &listenerRhsFunctionHandler, 0 ) ;

	// register for this so we know when to stop
	pKernel->RegisterForClientMessageEvent( "test-listener", SimpleListener::MyClientMessageHandler, 0 ) ;

	// Comment this in if you need to debug the messages going back and forth.
	//pKernel->SetTraceCommunications(true) ;

	long pauseSecs = 0;
	long pauseMsecs = 100 ;
	long pauseMsecsTotal = pauseSecs*1000+pauseMsecs;

	// If we're running in this thread we need to wake up more rapidly.
	if (useCurrentThread) 
	{ 
		life *= 10 ; 
		pauseMsecsTotal /= 10 ; 
	}

	// How often we check to see if the list of connections has changed.
	int checkConnections = 500 / pauseMsecsTotal ;
	int counter = checkConnections ;

	for (int i = 0 ; i < life && !g_Cancel && !shutdownMessageReceived ; i++)
	{
		// Don't need to check for incoming as we're using the NewThread model.
		if (useCurrentThread) pKernel->CheckForIncomingCommands() ;

		sml::Sleep(pauseSecs, pauseMsecs) ;
	}

	pKernel->Shutdown();	// this is technically redundant
	delete pKernel ;		// since this calls it

	//std::cout << std::endl << "SimpleListener done." << std::endl;

	//sml::Sleep(1, 0) ;
	
	return 0;
}

