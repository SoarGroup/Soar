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

class EmbeddedConnection : public Connection
{
public:
	// Clients should not use this.  Use Connection::CreateEmbeddedConnection instead.
	static EmbeddedConnection* CreateEmbeddedConnection() { return new EmbeddedConnection() ; }

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
	virtual ~EmbeddedConnection();

	// Link two embedded connections together
	virtual void AttachConnection(Connection_Receiver_Handle hConnection, ProcessMessageFunction pProcessMessage) ;
	virtual bool AttachConnection(char const* pLibraryName) ;

	virtual void SendMessage(ElementXML* pMsg) ;
	virtual ElementXML* GetResponseForID(char const* pID, bool wait) ;
	virtual void ReceiveMessages(bool allMessages)		{ unused(allMessages) ; ClearError() ; } 
	virtual void CloseConnection() ;
	virtual bool IsClosed() ;
};

} // End of namespace

#endif // SML_EMBEDDEDCONNECTION_H
