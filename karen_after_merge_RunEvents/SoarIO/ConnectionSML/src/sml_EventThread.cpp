#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// EventThread class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : January 2005
//
// This thread can be used by a client to keep it alive to incoming
// events even when the main client is sleeping.
//
// Here's the problem situation: client registers for an event,
// goes to sleep somewhere (e.g. inside a keyboard input handler or
// a GUI event loop) and then another client runs Soar.
// The events from Soar will block when they try to contact the sleeping
// client until it wakes up.  This thread is one way to make sure the
// client remains responsive.
//
/////////////////////////////////////////////////////////////////

#include "sml_EventThread.h"
#include "sml_Connection.h"
#include "sock_Debug.h"

#include <time.h>	// To get clock

using namespace sml ;

EventThread::EventThread(Connection* pConnection)
{
	m_pConnection = pConnection ;
}

/*************************************************************
* @brief The event thread's logic.
*		 When this method exits, the thread shuts down.
*
* This thread is very simple at its core, it calls "ReceiveMessages()"
* in a loop forever.
*
* The complexity comes in from throttling this thread correctly.
* If the client is actively sending messages to/from Soar we don't
* what this thread to slow it down by polling for messages.
* Also when the client sleeps we don't want this to be polling so
* quickly that it eats up all of the CPU on the machine.
* But equally, if the client is sleeping and someone else runs the kernel
* and starts sending tons of messages we want it to be as responsive
* as possible during that run.
*
* So here's the current solution.  We use a mutex (m_ClientMutex)
* inside the SendMessageGetResponse() and ReceiveMessages() methods to make
* sure either this thread or the main client is sending/receiving.
* By putting this in "SendMessageGetResponse" the client itself will lock
* out this thread for long periods (e.g. during an entire "run" call).
* 
* Then when this thread is functional, it's either polling really
* fast (Sleep(0)) or pausing for decent spans (Sleep(5)) which
* will lead to close to 0% CPU usage when everyone is sleeping
* (empirical tests on my laptop show 0%).
*************************************************************/
void EventThread::Run()
{
	// When we last received a message (if its recent we barely sleep so we're maximally responsive)
	clock_t last = 0 ;

	// How long to wait before sleeping (currently 1 sec)
	clock_t delay = CLOCKS_PER_SEC ;
/*
#ifdef _DEBUG
	PrintDebugFormat("Starting EventThread\n") ;
	long counter = 0 ;
#endif
*/
	while (!m_QuitNow && !m_pConnection->IsClosed())
	{
		// Our purpose is to check for incoming messages when the
		// client itself is sleeping for some reason.  It makes
		// the client logic simpler.
		bool receivedMessage = m_pConnection->ReceiveMessages(true) ;

		if (receivedMessage)
		{
			// Record the time of the last incoming message
			last = clock() ;
		}

/*
#ifdef _DEBUG
		counter++ ;
		if (counter % 1000 == 0)
			PrintDebugFormat("EventThread alive\n") ;
#endif
*/
		clock_t current = clock() ;

		// If it's been a while since the last incoming message
		// then start sleeping a reasonable amount.
		// Sleep(0) just allows other threads to run before we continue
		// to execute.
		if (current - last > delay)
			Sleep(0, 5) ;
		else
			// Changed this to always use sleep 5 now.
			// Calling sleep(0) here as we used to can cause a single threaded app
			// to take over the CPU, slowing down the entire system.  Doesn't happen
			// on a hyper threaded CPU, but on a normal CPU it's a significant problem.
			// The trade-off is that the response to events may not be quite as fast.
			Sleep(0, 5) ;
	}

/*
#ifdef _DEBUG
	PrintDebugFormat("EventThread terminated\n") ;
#endif
*/
}
