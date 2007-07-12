#include <portability.h>

/////////////////////////////////////////////////////////////////
// Thread class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Represents a thread.  To use this class
// derive from it and implement the "Run" method.
//
// Then create an instance of your class and call "Start".
//
/////////////////////////////////////////////////////////////////

#include "sml_Utils.h"
#include "thread_Thread.h"
#include "thread_OSspecific.h"

using namespace soar_thread ;

void ThreadStartFunction(void* pThreadObject)
{
	Thread* pThread = (Thread*)pThreadObject ;
	pThread->Run() ;

	// When Run() terminates this "ThreadStartFunction" will terminate
	// which will cause the thread itself to terminate.
	// At least, that works for Windows.  If it fails on Linux we'll
	// need an explicit "End" method.
	pThread->SetStopped(true) ;
}

Thread::Thread()
{
	m_QuitNow = false ;
	m_Stopped = true ;
	m_Started = false ;
}

Thread::~Thread() 
{
}

void Thread::Start()
{
	// If we've already been started ignore subsequent attempts
	// to start this thread.
	if (m_Started)
		return ;

	m_Stopped = false;
	m_Started = true ;
	// This creates a new thread and calls our static start function which in
	// turn calls back to the "Run" method of this class, so the user of the thread
	// has a nice object wrapped in a class to work with for their thread.
	BeginThread(ThreadStartFunction, this) ;
}

void Thread::Stop(bool waitTillStopped)
{
	// We also just signal the thread and ask it to stop.
	// We don't currently offer a "force thread to stop" method as that's
	// generally not a very safe thing to do.  If you feel you need it, add a "ForceStop" method.
	m_QuitNow = true ;

	// If the thread was never started or is already stopped we're done.
	if (!m_Started || IsStopped())
		return ;

	int maxTries = 1000 ;

	// If we've been asked to wait for the thread to quit then pause for a moment while it terminates.
	while (waitTillStopped && !IsStopped() && maxTries > 0)
	{
		sml::Sleep(0, 10) ;
		maxTries-- ;
	}

	if (maxTries <= 0)
	{
		sml::PrintDebug("Timed out waiting for thread to stop") ;
	}
}

