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

// Define this to allow direct (highly optimized) calls for
// I/O over embedded connections.
#define KERNEL_SML_DIRECT

#ifdef KERNEL_SML_DIRECT
#include "KernelSMLDirect.h"
#endif

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

#ifdef KERNEL_SML_DIRECT
	// These are shortcut methods we can use if this is an embedded connection
	// to optimize I/O performance.
	DirectAddWMEStringFunction			m_pDirectAddWMEStringFunction ;
	DirectAddWMEIntFunction				m_pDirectAddWMEIntFunction ;
	DirectAddWMEDoubleFunction			m_pDirectAddWMEDoubleFunction ;
	DirectRemoveWMEFunction				m_pDirectRemoveWMEFunction ;

	DirectAddIDFunction					m_pDirectAddIDFunction ;

	DirectGetAgentSMLHandleFunction		m_pDirectGetAgentSMLHandleFunction;

	DirectRunFunction					m_pDirectRunFunction ;
#endif

	/** We need to cache the responses to calls **/
	soarxml::ElementXML* m_pLastResponse ;

public:
	virtual ~EmbeddedConnection() ;

	// Link two embedded connections together
	virtual void AttachConnectionInternal(Connection_Receiver_Handle hConnection, ProcessMessageFunction pProcessMessage) ;
	virtual bool AttachConnection(char const* pLibraryName, bool optimized, int portToListenOn) ;
	virtual void ClearConnectionHandle() { m_hConnection = NULL ; }

	virtual void CloseConnection() ;
	virtual bool IsClosed() ;
	virtual bool IsRemoteConnection() { return false ; }

	virtual void SetTraceCommunications(bool state) ;

	// Overridden in concrete subclasses
	virtual bool IsAsynchronous() = 0 ;		// Returns true if messages are queued and executed on receiver's thread
	virtual void SendMsg(soarxml::ElementXML* pMsg) = 0 ;
	virtual soarxml::ElementXML* GetResponseForID(char const* pID, bool wait) = 0 ;
	virtual bool ReceiveMessages(bool allMessages) = 0 ;

#ifdef KERNEL_SML_DIRECT
	// Direct methods, only supported for embedded connections and only used to optimize
	// the speed when doing I/O over an embedded connection (where speed is most critical)
	void DirectAddWME_String(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, char const* pValue, long clientTimetag)
	{
		m_pDirectAddWMEStringFunction(pAgentSML, pId, pAttribute, pValue, clientTimetag) ;
	}
	void DirectAddWME_Int(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, int value, long clientTimetag)
	{
		m_pDirectAddWMEIntFunction(pAgentSML, pId, pAttribute, value, clientTimetag) ;
	}
	void DirectAddWME_Double(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, double value, long clientTimetag)
	{
		m_pDirectAddWMEDoubleFunction(pAgentSML, pId, pAttribute, value, clientTimetag) ;
	}
	void DirectRemoveWME(Direct_AgentSML_Handle pAgentSML, long clientTimetag)
	{
		m_pDirectRemoveWMEFunction(pAgentSML, clientTimetag) ;
	}

	void DirectAddID(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, char const* pValueId, long clientTimetag)
	{
		m_pDirectAddIDFunction(pAgentSML, pId, pAttribute, pValueId, clientTimetag) ;
	}

	Direct_AgentSML_Handle DirectGetAgentSMLHandle(char const* pAgentName)
	{
		return m_pDirectGetAgentSMLHandleFunction(pAgentName) ;
	}

	void DirectRun(char const* pAgentName, bool forever, int stepSize, int interleaveSize, int count)
	{
		m_pDirectRunFunction(pAgentName, forever, stepSize, interleaveSize, count) ;
	}

#endif
} ;

} // End of namespace

#endif // SML_EMBEDDEDCONNECTION_H
