/////////////////////////////////////////////////////////////////
// EmbeddedConnection class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class represents a logical connection between two entities that are communicating
// via SML (a form of XML).  In the embedded case that this class represents, both entities
// are within the same process.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_EMBEDDEDCONNECTION_H
#define SML_EMBEDDEDCONNECTION_H

#include "sml_Connection.h"
#include "sml_Handles.h"
#include "EmbeddedSMLInterface.h"

namespace sml
{

class EmbeddedConnectionSynch ;
class EmbeddedConnectionAsynch ;

// Abstract base class for embedded connections
class EmbeddedConnection : public Connection
{
public:
	// Clients should not use this.  Use Connection::CreateEmbeddedConnection instead which creates
	// a two-way connection.  This just creates a one-way object.
//	static Connection* CreateEmbeddedConnection() ;

protected:
	// Clients should not use this.  Use Connection::CreateEmbeddedConnection instead.
	// Making it protected so you can't accidentally create one like this.
	EmbeddedConnection() ;

protected:
	/** To "send" a message we call to the process message function for this receiver. **/
	Connection_Receiver_Handle m_hConnection ;

	/** These are the two functions a DLL exports to support an embedded connection interface */
	ProcessMessageFunction				m_pProcessMessageFunction ;
	CreateEmbeddedConnectionFunction	m_pCreateEmbeddedFunction ;

	/** We need to cache the responses to calls **/
	ElementXML* m_pLastResponse ;

public:
	virtual ~EmbeddedConnection() ;

	// We only use this queue when using Asynch embedded connections
	void AddToIncomingMessageQueue(ElementXML* pMsg)
	{
		// Make sure only one thread modifies the message queue at a time.
		soar_thread::Lock lock(&m_Mutex) ;
		m_IncomingMessageQueue.push(pMsg) ;
	}

	// Link two embedded connections together
	virtual void AttachConnectionInternal(Connection_Receiver_Handle hConnection, ProcessMessageFunction pProcessMessage) ;
	virtual bool AttachConnection(char const* pLibraryName, int portToListenOn) ;
	virtual void ClearConnectionHandle() { m_hConnection = NULL ; }

	virtual void CloseConnection() ;
	virtual bool IsClosed() ;
	virtual bool IsRemoteConnection() { return false ; }

	// Overridden in concrete subclasses
	virtual bool IsAsynchronous() = 0 ;		// Returns true if messages are queued and executed on receiver's thread
	virtual void SendMessage(ElementXML* pMsg) = 0 ;
	virtual ElementXML* GetResponseForID(char const* pID, bool wait) = 0 ;
	virtual bool ReceiveMessages(bool allMessages) = 0 ;
} ;

// This version makes synchronous calls, which means for example that a "run" command
// will be executed on the client's thread.
class EmbeddedConnectionSynch : public EmbeddedConnection
{
public:
	// Clients should not use this.  Use Connection::CreateEmbeddedConnection instead which creates
	// a two-way connection.  This just creates a one-way object.
	static EmbeddedConnection* CreateEmbeddedConnectionSynch() { return new EmbeddedConnectionSynch() ; }

protected:
	// Clients should not use this.  Use Connection::CreateEmbeddedConnection instead.
	// Making it protected so you can't accidentally create one like this.
	EmbeddedConnectionSynch() { } 

public:
	virtual ~EmbeddedConnectionSynch() { } 

	virtual bool IsAsynchronous() { return false ; }
	virtual void SendMessage(ElementXML* pMsg) ;
	virtual ElementXML* GetResponseForID(char const* pID, bool wait) ;
	virtual bool ReceiveMessages(bool allMessages)		{ unused(allMessages) ; ClearError() ; return false ; } 
};

// This version makes asynchronous calls, which means commands are stored
// in a queue and are actually executed on a different thread in the server (kernel).

class EmbeddedConnectionAsynch : public EmbeddedConnection
{
public:
	static EmbeddedConnection* CreateEmbeddedConnectionAsynch() { return new EmbeddedConnectionAsynch() ; }

protected:
	// Clients should not use this.  Use Connection::CreateEmbeddedConnection instead.
	// Making it protected so you can't accidentally create one like this.
	EmbeddedConnectionAsynch() { } 

public:
	virtual ~EmbeddedConnectionAsynch() { } 

	virtual bool IsAsynchronous() { return true ; }
	virtual void SendMessage(ElementXML* pMsg) ;
	virtual ElementXML* GetResponseForID(char const* pID, bool wait) ;
	virtual bool ReceiveMessages(bool allMessages) ;
} ;


} // End of namespace

#endif // SML_EMBEDDEDCONNECTION_H
