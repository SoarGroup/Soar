/////////////////////////////////////////////////////////////////
// ListenerThread class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Listens for incoming remote connections and creates
// a new connection object for each one.
//
/////////////////////////////////////////////////////////////////

#ifndef LISTENER_THREAD_H
#define LISTENER_THREAD_H

#include "thread_Thread.h"
#include "sock_DataSender.h"
#include "sock_ListenerSocket.h"
#include "sock_ListenerNamedPipe.h"
#include "sock_SocketLib.h"
#include "sml_Connection.h"

#include <list>
#include <string>
#include <sstream>

namespace sml {

template <typename T>
std::string ToString(const T val)
{
	std::stringstream strm;
	strm << val;
	return strm.str();
}

// Forward declarations
class ConnectionManager ;

// A listener socket wrapped in a thread
class ListenerThread : public soar_thread::Thread
{
protected:
	unsigned short				m_Port ;
	std::string					m_PipeName;
	ConnectionManager*			m_Parent ;
	sock::ListenerSocket		m_ListenerSocket ;
	sock::ListenerSocket		m_LocalListenerSocket;
	sock::ListenerNamedPipe		m_ListenerNamedPipe ;

	void Run() ;

	void CreateConnection(sock::DataSender* pSender);

public:
	ListenerThread(ConnectionManager* parent, unsigned short port) { m_Parent = parent ; m_Port = port ; m_PipeName = "\\\\.\\pipe\\"; m_PipeName.append(ToString(port)); }
} ;


} // Namespace

#endif	// LISTENER_THREAD_H
