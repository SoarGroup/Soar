/////////////////////////////////////////////////////////////////
// RemoteConnection class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This class represents a logical connection between two entities that are communicating via SML over a socket.
// For example, an environment (the client) and the Soar kernel.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_REMOTE_CONNECTION_H
#define SML_REMOTE_CONNECTION_H

#include "sml_Connection.h"

namespace sock
{
	// Forward declarations
	class Socket ;
}

namespace sml
{

class RemoteConnection : public Connection
{
	// Allow the connection class to create instances of this class
	friend Connection ;

protected:
	// Clients should not use this.  Use Connection::CreateRemoteConnection instead.
	// Making it protected so you can't accidentally create one like this.
	RemoteConnection(bool sharedFileSystem, sock::Socket* pSocket) ;

protected:
	// The socket we use to send and receive messages (sockets are always 2-way)
	sock::Socket*	m_Socket ;

	// Whether both sides of the socket have access to the same file system
	// (i.e. whether sending a filename makes sense or if we need to send the file contents).
	// As of today (Oct 2004) this flag is always assumed to be true, but later on we
	// may start to support situations where it's false and having the flag means less
	// breaking of existing code.
	bool m_SharedFileSystem ;

	/** We need to cache the responses to calls **/
	ElementXML* m_pLastResponse ;

public:
	virtual ~RemoteConnection();

	virtual void SendMessage(ElementXML* pMsg) ;
	virtual ElementXML* GetResponseForID(char const* pID, bool wait) ;
	virtual bool ReceiveMessages(bool allMessages) ;
	virtual void CloseConnection() ;
	virtual bool IsClosed() ;
	virtual bool IsRemoteConnection() { return true ; }

};

} // End of namespace

#endif // SML_REMOTE_CONNECTION_H

