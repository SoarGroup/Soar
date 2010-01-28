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

#ifndef RECEIVER_THREAD_H
#define RECEIVER_THREAD_H

#include "thread_Thread.h"
#include "sml_Connection.h"

#include <list>

namespace sml {

// Forward declarations
class ConnectionManager ;

class ReceiverThread : public soar_thread::Thread
{
protected:
	ConnectionManager*		m_ConnectionManager ;

	// This method is executed in the different thread
	void Run() ;

public:
	ReceiverThread(ConnectionManager* pManager) { m_ConnectionManager = pManager ; }
} ;

} // Namespace

#endif	// RECEIVER_THREAD_H
