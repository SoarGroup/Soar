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
	DirectLinkIDFunction				m_pDirectLinkIDFunction ;
	DirectGetThisWMObjectFunction		m_pDirectGetThisWMObjectFunction ;

	DirectGetRootFunction				m_pDirectGetRootFunction ;
	DirectGetWorkingMemoryFunction		m_pDirectGetWorkingMemoryFunction ;
	DirectRunFunction					m_pDirectRunFunction ;

	DirectReleaseWMEFunction			m_pDirectReleaseWMEFunction ;
	DirectReleaseWMObjectFunction		m_pDirectReleaseWMObjectFunction ;
#endif

	/** We need to cache the responses to calls **/
	ElementXML* m_pLastResponse ;

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
	virtual void SendMessage(ElementXML* pMsg) = 0 ;
	virtual ElementXML* GetResponseForID(char const* pID, bool wait) = 0 ;
	virtual bool ReceiveMessages(bool allMessages) = 0 ;

#ifdef KERNEL_SML_DIRECT
	// Direct methods, only supported for embedded connections and only used to optimize
	// the speed when doing I/O over an embedded connection (where speed is most critical)
	Direct_WME_Handle			DirectAddWME_String(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long clientTimeTag, char const* pAttribute, char const* value)
	{
		return m_pDirectAddWMEStringFunction(wm, parent, clientTimeTag, pAttribute, value) ;
	}
	Direct_WME_Handle			DirectAddWME_Int(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long clientTimeTag, char const* pAttribute, int value)
	{
		return m_pDirectAddWMEIntFunction(wm, parent, clientTimeTag, pAttribute, value) ;
	}
	Direct_WME_Handle			DirectAddWME_Double(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long clientTimeTag, char const* pAttribute, double value)
	{
		return m_pDirectAddWMEDoubleFunction(wm, parent, clientTimeTag, pAttribute, value) ;
	}
	void						DirectRemoveWME(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme, long clientTimeTag)
	{
		m_pDirectRemoveWMEFunction(wm, wme, clientTimeTag) ;
	}

	Direct_WME_Handle			DirectAddID(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long clientTimeTag, char const* pAttribute)
	{
		return m_pDirectAddIDFunction(wm, parent, clientTimeTag, pAttribute) ;
	}
	Direct_WME_Handle			DirectLinkID(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long clientTimeTag, char const* pAttribute, Direct_WMObject_Handle orig)
	{
		return m_pDirectLinkIDFunction(wm, parent, clientTimeTag, pAttribute, orig) ;
	}
	Direct_WMObject_Handle		DirectGetThisWMObject(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme)
	{
		return m_pDirectGetThisWMObjectFunction(wm, wme) ;
	}

	Direct_WorkingMemory_Handle DirectGetWorkingMemory(char const* pAgentName, bool input)
	{
		return m_pDirectGetWorkingMemoryFunction(pAgentName, input) ;
	}
	Direct_WMObject_Handle		DirectGetRoot(char const* pAgentName, bool input)
	{
		return m_pDirectGetRootFunction(pAgentName, input) ;
	}
	void						DirectRun(char const* pAgentName, bool forever, int stepSize, int interleaveSize, int count)
	{
		m_pDirectRunFunction(pAgentName, forever, stepSize, interleaveSize, count) ;
	}

	void						DirectReleaseWME(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme, long clientTimeTag)
	{
		return m_pDirectReleaseWMEFunction(wm, wme, clientTimeTag) ;
	}
	void						DirectReleaseWMObject(Direct_WMObject_Handle parent)
	{
		return m_pDirectReleaseWMObjectFunction(parent) ;
	}
#endif
} ;

} // End of namespace

#endif // SML_EMBEDDEDCONNECTION_H
