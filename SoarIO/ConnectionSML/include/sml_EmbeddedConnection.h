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
	DirectRunTilOutputFunction			m_pDirectRunTilOutputFunction ;

	DirectReleaseWMEFunction			m_pDirectReleaseWMEFunction ;
	DirectReleaseWMObjectFunction		m_pDirectReleaseWMObjectFunction ;
#endif

	/** We need to cache the responses to calls **/
	ElementXML* m_pLastResponse ;

public:
	virtual ~EmbeddedConnection() ;

	// We only use this queue when using Asynch embedded connections
	void AddToIncomingMessageQueue(ElementXML* pMsg)
	{
		// Make sure only one thread modifies the message queue at a time.
		soar_thread::Lock lock(&m_IncomingMutex) ;
		m_IncomingMessageQueue.push(pMsg) ;
	}

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
	Direct_WME_Handle			DirectAddWME_String(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, char const* pAttribute, char const* value)
	{
		return m_pDirectAddWMEStringFunction(wm, parent, pAttribute, value) ;
	}
	Direct_WME_Handle			DirectAddWME_Int(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, char const* pAttribute, int value)
	{
		return m_pDirectAddWMEIntFunction(wm, parent, pAttribute, value) ;
	}
	Direct_WME_Handle			DirectAddWME_Double(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, char const* pAttribute, double value)
	{
		return m_pDirectAddWMEDoubleFunction(wm, parent, pAttribute, value) ;
	}
	void						DirectRemoveWME(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme)
	{
		m_pDirectRemoveWMEFunction(wm, wme) ;
	}

	Direct_WME_Handle			DirectAddID(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, char const* pAttribute)
	{
		return m_pDirectAddIDFunction(wm, parent, pAttribute) ;
	}
	Direct_WME_Handle			DirectLinkID(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, char const* pAttribute, Direct_WMObject_Handle orig)
	{
		return m_pDirectLinkIDFunction(wm, parent, pAttribute, orig) ;
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
	void						DirectRunTilOutput(char const* pAgentName)
	{
		m_pDirectRunTilOutputFunction(pAgentName) ;
	}

	void						DirectReleaseWME(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme)
	{
		return m_pDirectReleaseWMEFunction(wm, wme) ;
	}
	void						DirectReleaseWMObject(Direct_WMObject_Handle parent)
	{
		return m_pDirectReleaseWMObjectFunction(parent) ;
	}
#endif
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
	static EmbeddedConnection* CreateEmbeddedConnectionAsynch(bool useSynchCalls = false) { return new EmbeddedConnectionAsynch(useSynchCalls) ; }

protected:
	// Clients should not use this.  Use Connection::CreateEmbeddedConnection instead.
	// Making it protected so you can't accidentally create one like this.
	EmbeddedConnectionAsynch(bool useSynchCalls) { m_UseSynchCalls = useSynchCalls ; } 

	// The kernel can send 'call' messages back to the client directly without
	// placing them in a queue.  In fact, it has to do this in situations
	// where it's sending event information back and the client may not be
	// waiting to receive an incoming message.  Thus the kernel can set this
	// flag and send messages back directly.
	bool	m_UseSynchCalls ;

public:
	virtual ~EmbeddedConnectionAsynch() { } 

	virtual bool IsAsynchronous() { return true ; }
	virtual void SendMessage(ElementXML* pMsg) ;
	virtual ElementXML* GetResponseForID(char const* pID, bool wait) ;
	virtual bool ReceiveMessages(bool allMessages) ;

	// Even though this is an asynch connection, send this message synchronously.
	//void SendSynchMessage(ElementXML_Handle hSendMsg) ;
} ;


} // End of namespace

#endif // SML_EMBEDDEDCONNECTION_H
