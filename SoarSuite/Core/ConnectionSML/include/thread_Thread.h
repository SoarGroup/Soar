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

#ifndef THREAD_THREAD_H
#define THREAD_THREAD_H

#include "thread_Event.h"

namespace soar_thread {

class Thread
{
protected:
	volatile bool	m_QuitNow ;
	bool	m_Started ;
	bool	m_Stopped ;
	Event	m_Pause ;	// We use this to support suspend/resume

	bool QuitNow()	{ return m_QuitNow ; }

public:
	// Static sleep method (platform independent)
	// which causes the caller's thread to sleep.
	static void SleepStatic(long seconds, long milliseconds) ;

protected:
	// Cause this thread to sleep for a while
	void Sleep(long seconds, long milliseconds) ;

public:
	Thread() ;
	virtual ~Thread() ;

	// Call to start this thread running (from another thread)
	void Start() ;

	// Call to ask this thread to stop running (from another thread)
	void Stop(bool waitTillStopped) ;

	// This returns true once the thread has actually stopped running
	// which may be a little after the request to stop.
	bool IsStopped() { return m_Stopped ; }

	// True if this thread has been started
	bool IsStarted() { return m_Started ; }

	// Cause this thread to sleep until "Resume" is called
	void Suspend() { m_Pause.WaitForEventForever() ; }

	// Wake this thread up after a "Suspend" call.
	void Resume() { m_Pause.TriggerEvent() ; }

	// Implement this in your thread.
	// When it exits, the thread quits.
	// Test for "QuitNow" to know when you've been asked to stop.
	// So generally it takes the form "while (!m_QuitNow && !done) { doWork() ; SleepThread() ;  } "
	virtual void Run() = 0 ;

	// Client's should call this (consider it to be private, but we can't actually make it so)
	// If you wish to stop the thread, call "Stop" instead (which asks it to stop).
	void SetStopped(bool state) { m_Stopped = state ; }
} ;

} // Namespace

#endif // THREAD_THREAD_H

