#include "simplelistener.h"

#include <string>
#include <sstream>
#include <iostream>

#include "sml_Client.h"
#include "sml_Utils.h"

extern bool g_CancelPressed;

std::string listenerRhsFunctionHandler(sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pFunctionName, char const* pArgument){
	// Return the argument we are passed plus some of our text
	//cout << "Received rhs function call with argument: " << pArgument << endl ;

	if ( !pArgument ) return std::string();

	std::stringstream res;
	res << "my listener result " << pArgument;
	return res.str() ;
}

// Create a process that listens for remote commands and lives
// for "life" 10'ths of a second (e.g. 10 = live for 1 second)
SimpleListener::SimpleListener( int life, int port )
: life( life ), port( port ), useCurrentThread( false )
{
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

	pKernel->SetConnectionInfo( "listener", "ready", "ready" );

	// Register here so we can test the order that RHS functions are called
	pKernel->AddRhsFunction( "test-rhs", &listenerRhsFunctionHandler, 0 ) ;

	// Comment this in if you need to debug the messages going back and forth.
	//pKernel->SetTraceCommunications(true) ;

	long pauseSecs = 0;
	long pauseMsecs = 100 ;
	long pauseMsecsTotal = pauseSecs*1000+pauseMsecs;

	// If we're running in this thread we need to wake up more rapidly.
	if (useCurrentThread) { life *= 10 ; pauseMsecsTotal /= 10 ; }

	// How often we check to see if the list of connections has changed.
	//int checkConnections = 500 / pauseMsecsTotal ;
	//int counter = checkConnections ;

	for (int i = 0 ; i < life && !g_CancelPressed ; i++)
	{
		// Don't need to check for incoming as we're using the NewThread model.
		if (useCurrentThread)
		{
			bool check = pKernel->CheckForIncomingCommands() ;
			unused(check);
		}

		sml::Sleep(pauseSecs, pauseMsecs) ;
	}

	delete pKernel ;

	return 0;
}

