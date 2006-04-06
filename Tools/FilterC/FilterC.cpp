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

#include "thread_Thread.h"
#include "thread_Lock.h"
#include "thread_Event.h"

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
	ElementXML* pXML = ElementXML::ParseXMLFromString(pCommandLine) ;

	std::string commandLine = pXML->GetAttribute(sml_Names::kFilterCommand) ;
	std::string output ;

	cout << "Received command line " << commandLine << endl ;

	if (commandLine.compare(0, 5, "print") == 0)
	{
		if (commandLine.compare("print --stack") == 0)
		{
			// Consume the command and return our own output
			commandLine = "" ;
			output = "Stack command consumed by filter" ;

			cout << "FILTER -- print stack consumed by the filter" << endl ;
		}
		else
		{
			commandLine += " --depth 2" ;
			cout << "FILTER -- print command changed to " << commandLine << endl ;
		}
	}

	// Replace the command attribute in the XML
	pXML->AddAttribute(sml_Names::kFilterCommand, commandLine.c_str()) ;
	pXML->AddAttribute(sml_Names::kFilterOutput, output.c_str()) ;

	// Convert the XML back to a string and put it into a std::string ready to return
	char *pXMLString = pXML->GenerateXMLString(true) ;
	std::string res = pXMLString ;
	pXML->DeleteString(pXMLString) ;
	delete pXML ;

	return res ;
}

// This is just so we can listen for 'q' to exit.
// It's not part of the filter.
class InputThread ;

soar_thread::Mutex	g_pInputQueueMutex;
InputThread*		g_pInputThread = 0;

class InputThread : public soar_thread::Thread {
public:
	InputThread() { m_Stop = false ; }

	void Run() {

		// Get input
		char ch ;
		while ((!this->m_Stop) && (cin.get(ch)))
		{
			soar_thread::Lock lock(&g_pInputQueueMutex);

			if (ch == 'q' || ch == 'Q')
				m_Stop = true ;
		}
	}

	bool StopNow() { soar_thread::Lock lock(&g_pInputQueueMutex) ; return m_Stop ; }

protected:
	bool	m_Stop ;
};

int main(int argc, char* argv[])
{
	cout << "Trying to connect to remote kernel" << endl ;
	sml::Kernel* pKernel = sml::Kernel::CreateRemoteConnection(true, NULL) ;

	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl ;
		cout << "The filter needs to be run after the kernel it is working with has been started" << endl ;
		exit(0) ;
	}

	// Record a filter
	int clientFilter = pKernel->RegisterForClientMessageEvent(sml_Names::kFilterName, &MyFilter, 0) ;

	g_pInputThread = new InputThread() ;
	g_pInputThread->Start() ;

	cout << "Listening to remote kernel now.  Type q <return> to exit." << endl ;

	// Sleep until someone types 'q <return>'
	while (!g_pInputThread->StopNow())
	{
		SLEEP(1,0) ;
	}

	// Don't think we'll ever get here...
	pKernel->Shutdown() ;
	delete pKernel ;
	delete g_pInputThread ;
}
