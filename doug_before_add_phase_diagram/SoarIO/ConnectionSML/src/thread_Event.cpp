#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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

#include "thread_Event.h"
#include "thread_OSspecific.h"

using namespace soar_thread ;

Event::Event()
{
	m_Imp = MakeEvent() ;
}

Event::~Event()
{
	delete m_Imp ;
}

void Event::WaitForEventForever()
{
	m_Imp->WaitForEventForever() ;
}

bool Event::WaitForEvent(long seconds, long milliseconds)
{
	return m_Imp->WaitForEvent(seconds, milliseconds) ;
}

void Event::TriggerEvent()
{
	m_Imp->TriggerEvent() ;
}

