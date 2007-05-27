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
	class DataSender ;
}

namespace sml
{

class RemoteConnection : public Connection
{
	// Allow the connection class to create instances of this class
	friend class Connection ;

protected:
	// Clients should not use this.  Use Connection::CreateRemoteConnection instead.
	// Making it protected so you can't accidentally create one like this.
	RemoteConnection(bool sharedFileSystem, sock::DataSender* pDataSender) ;

protected:
	// The data sender we use to send and receive messages (data senders are always 2-way)
	sock::DataSender*	m_DataSender ;

	// Whether both sides of the socket have access to the same file system
	// (i.e. whether sending a filename makes sense or if we need to send the file contents).
	// As of today (Oct 2004) this flag is always assumed to be true, but later on we
	// may start to support situations where it's false and having the flag means less
	// breaking of existing code.
	bool m_SharedFileSystem ;

	/** We need to cache the responses to calls **/
	ElementXML* m_pLastResponse ;

	/** A list of messages we've received that have "ack" fields but have yet to match up to the commands which triggered them */
	MessageList		m_ReceivedMessageList ;

	enum 			{ kMaxListSize = 10 } ;

	/** Ensures only one thread accesses the response list at a time **/
	soar_thread::Mutex	m_ListMutex ;

	/** Adds the message to the queue, taking ownership of it at the same time */
	void AddResponseToList(ElementXML* pResponse) ;
	ElementXML* IsResponseInList(char const* pID) ;

	bool DoesResponseMatch(ElementXML* pResponse, char const* pID) ;

	/** The timeout for receiving messages is secondsWait + millisecondsWait, where millisecondsWait < 1000 */
	bool ReceiveMessages(bool allMessages, long secondsWait, long millisecondsWait) ;

public:
	virtual ~RemoteConnection();

	virtual void SendMessage(ElementXML* pMsg) ;
	virtual ElementXML* GetResponseForID(char const* pID, bool wait) ;
	virtual bool ReceiveMessages(bool allMessages) ;
	virtual void CloseConnection() ;
	virtual bool IsClosed() ;
	virtual bool IsRemoteConnection() { return true ; }
	virtual bool IsAsynchronous() { return true ; }
	virtual void SetTraceCommunications(bool state) ;

};

} // End of namespace

#endif // SML_REMOTE_CONNECTION_H

