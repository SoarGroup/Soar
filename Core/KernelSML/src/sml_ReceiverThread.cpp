#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

/////////////////////////////////////////////////////////////////
// ReceiverThread class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Listens for incoming remote commands sent from external
// processes (e.g. commands from a debugger).
//
/////////////////////////////////////////////////////////////////

#include "sml_ConnectionManager.h"
#include "sml_ReceiverThread.h"
#include "sml_KernelSML.h"
#include "sock_Debug.h"

#include <time.h>	// To get clock

using namespace sml ;
using namespace sock ;

void ReceiverThread::Run()
{
	// While this thread is alive, keep checking for incoming messages
	// and executing them.  These messages could be coming in from a remote socket
	// or from an embedded connection.  If we're executing on the client thread
	// for an embedded connection (the "sycnhronous" model) then this thread is shut down
	// and it's the client's responsibility to check for incoming messages by calling
	// "GetIncomingCommands" periodically.

	// Record the time of the last message we received and keep polling
	// for a little after at a high speed, before sleeping.
	// The reason we track this is because calling "Sleep(0)" gives us maximum
	// performance (on towers-of-hanoi calling Sleep(1) instead slows execution down
	// by a factor of 80!), but calling "Sleep(0)" means we're stuck in a tight loop checking
	// messages continually.  If we're using Soar through a command line interface (for instance)
	// then the CPU would be 100% loaded all of the time, even if the user wasn't typing anything.
	// This method of switching between Sleep(0) and a true sleep means we get maximum performance
	// during a run (when messages are flying back and forth) but when the user stops doing anything
	// the CPU quickly drops off to 0% usage as we sleep a lot.
	// The other option would be to have a notification system where we sleep until a command
	// comes in, but building that in a cross platform fashion isn't something we want to tackle right now.
	clock_t last = 0 ;

	// How long to wait before sleeping (currently 1 sec)
	clock_t delay = CLOCKS_PER_SEC ;

//	clock_t report = 0 ;

	while (!m_QuitNow)
	{
#ifdef _DEBUG
		/*
		// Every so often report that we're alive
		if (clock() - report > CLOCKS_PER_SEC * 3)
		{
			report = clock() ;
			PrintDebugFormat("ReceiverThread::Run alive") ;
		}
		*/
#endif
		// Receive any incoming commands and execute them
		bool receivedMessage = m_ConnectionManager->ReceiveAllMessages() ;

		if (receivedMessage)
		{
			// Record the time of the last incoming message
			last = clock() ;
		}

		clock_t current = clock() ;

		// If it's been a while since the last incoming message
		// then start sleeping a reasonable amount.
		// Sleep(0) just allows other threads to run before we continue
		// to execute.
		if (current - last > delay)
			Sleep(0, 5) ;
		else
			Sleep(0, 0) ;
	}
}
