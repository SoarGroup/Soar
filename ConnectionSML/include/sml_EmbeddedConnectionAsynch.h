/////////////////////////////////////////////////////////////////
// EmbeddedConnectionAsynch class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class represents a logical connection between two entities that are communicating
// via SML (a form of XML).  In the embedded case that this class represents, both entities
// are within the same process.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_EMBEDDEDCONNECTION_ASYNCH_H
#define SML_EMBEDDEDCONNECTION_ASYNCH_H

#include "sml_Connection.h"
#include "sml_EmbeddedConnection.h"
#include "thread_Event.h"

namespace sml
{

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

	/** A list of messages we've received that have "ack" fields but have yet to match up to the commands which triggered them */
	MessageList		m_ReceivedMessageList ;

	enum 			{ kMaxListSize = 10 } ;

	/** Ensures only one thread accesses the response list at a time **/
	soar_thread::Mutex	m_ListMutex ;

	/** An event object which we use to have one thread sleep while waiting for another thread to drop off a response to a message */
	soar_thread::Event  m_WaitEvent ;

	/** Adds the message to the queue, taking ownership of it at the same time */
	void AddResponseToList(ElementXML* pResponse) ;
	ElementXML* IsResponseInList(char const* pID) ;

	bool DoesResponseMatch(ElementXML* pResponse, char const* pID) ;

public:
	virtual ~EmbeddedConnectionAsynch() ;

	// Commands are added to this queue that this connection will
	// process in the future.  E.g. A client would use this call to add
	// a command to the queue that the kernel would then execute.
	void AddToIncomingMessageQueue(ElementXML* pMsg)
	{
		// Make sure only one thread modifies the message queue at a time.
		soar_thread::Lock lock(&m_IncomingMutex) ;
		m_IncomingMessageQueue.push(pMsg) ;

		// Wake up anybody who's waiting for a response
		m_WaitEvent.TriggerEvent() ;
	}

	virtual bool IsAsynchronous() { return true ; }
	virtual void SendMessage(ElementXML* pMsg) ;
	virtual ElementXML* GetResponseForID(char const* pID, bool wait) ;
	virtual bool ReceiveMessages(bool allMessages) ;
} ;


} // End of namespace

#endif // SML_EMBEDDEDCONNECTION_H
