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

#ifndef SML_EVENT_THREAD_H
#define SML_EVENT_THREAD_H

#include "thread_Thread.h"

namespace sml
{

// Forward declarations
class Connection ;

class EventThread : public soar_thread::Thread
{
protected:
	Connection*	m_pConnection ;

public:
	EventThread(Connection* pConnection) ;
	void Run() ;
} ;

}

#endif	// SML_EVENT_THREAD
