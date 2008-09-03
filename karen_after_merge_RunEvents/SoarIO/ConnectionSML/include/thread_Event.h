/////////////////////////////////////////////////////////////////
// Event class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// An event can be used to connect two threads.  One thread waits
// on the thread for a set amount of time (or forever) and the other can trigger
// the event to awake the first.
//
/////////////////////////////////////////////////////////////////

#ifndef THREAD_EVENT_H
#define THREAD_EVENT_H

namespace soar_thread {

// Forward declarations
class OSSpecificEvent ;

class Event
{
protected:
	// OS specific implementation
	OSSpecificEvent*	m_Imp ;

public:
	Event() ;
	~Event() ;

	void WaitForEventForever() ;

	// Returns true if event was triggered.  False if we just timed out.
	//The timeout is seconds + milliseconds, where milliseconds < 1000
	bool WaitForEvent(long seconds, long milliseconds) ;
	void TriggerEvent() ;	// Causes waiting thread to wake up
} ;

} // Namespace

#endif // THREAD_EVENT_H
