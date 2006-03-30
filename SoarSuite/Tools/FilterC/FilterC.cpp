/////////////////////////////////////////////////////////////////
// FilterC
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Example application for how to build a command line filter.
// That is, an application that runs quietly in the background
// intercepting and potentially modifying commands that the user types.
//
// This is really aimed at providing Tcl support for productions
// but we're trying to make sure it's general, so this example will
// be in C++.
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <assert.h>
#include <iostream>

#include "sml_Connection.h"		// To get sml_Names
#include "sml_Client.h"

using namespace std ;
using namespace sml ;

// Define a sleep
#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>
void SLEEP(long secs, long msecs)
{
	assert(msecs < 1000 && "Specified milliseconds too large; use seconds argument to specify part of time >= 1000 milliseconds");
	Sleep((secs * 1000) + msecs) ;
}
#else
#include <time.h>
void SLEEP(long secs, long msecs)
{
	assert(msecs < 1000 && "Specified milliseconds too large; use seconds argument to specify part of time >= 1000 milliseconds");
	struct timespec sleeptime;
	sleeptime.tv_sec = secs;
	sleeptime.tv_nsec = msecs * 1000000;
	nanosleep(&sleeptime, 0);
}
#endif

// This filter echos all of the user's commands and adds " --depth 2" to any print commands.
// It also consumes all "print --stack" commands (just to show how to do that) -- effectively preventing them from executing.
std::string MyFilter(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pMessageType, char const* pCommandLine)
{
	cout << "Received command line " << pCommandLine << endl ;

	std::string res = pCommandLine ;
	if (strncmp(pCommandLine, "print", 5) == 0)
	{
		if (strcmp(pCommandLine, "print --stack") == 0)
		{
			res = "" ;
			cout << "FILTER -- print stack consumed by the filter" << endl ;
		}
		else
		{
			res += " --depth 2" ;
			cout << "FILTER -- print command changed to " << res << endl ;
		}
	}

	return res ;
}

int main(int argc, char* argv[])
{
	sml::Kernel* pKernel = sml::Kernel::CreateRemoteConnection(true, NULL) ;

	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl ;
		cout << "The filter needs to be run after the kernel it is working with has been started" << endl ;
		exit(0) ;
	}

	// Record a filter
	int clientFilter = pKernel->RegisterForClientMessageEvent(sml_Names::kFilterName, &MyFilter, 0) ;

	// Sleep forever
	for(;;)
	{
		SLEEP(100,0) ;
	}

	// Don't think we'll ever get here...
	pKernel->Shutdown() ;
	delete pKernel ;
}
