/////////////////////////////////////////////////////////////////
// EmbeddedConnectionSynch class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class represents a logical connection between two entities that are communicating
// via SML (a form of XML).  In the embedded case that this class represents, both entities
// are within the same process.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_EMBEDDEDCONNECTION_SYNCH_H
#define SML_EMBEDDEDCONNECTION_SYNCH_H

#include "sml_Connection.h"
#include "sml_EmbeddedConnection.h"

namespace sml
{

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

} // End of namespace

#endif // SML_EMBEDDEDCONNECTION_H
