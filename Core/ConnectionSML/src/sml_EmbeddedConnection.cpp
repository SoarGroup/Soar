#include <portability.h>

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

#include "sml_Utils.h"
#include "sml_EmbeddedConnection.h"
#include "sml_EmbeddedConnectionAsynch.h"
#include "ElementXML.h"
#include "sml_MessageSML.h"
#include "thread_Thread.h"

#include <string>
#include <iostream>
#include <assert.h>

using namespace sml ;
using namespace soarxml ;

#ifdef SCONS // scons has detected a posix environment
#  ifdef STATIC_LINKED
#    define LINUX_STATIC
#  else
#    define LINUX_SHARED
#  endif
#endif

#ifdef _WIN32
#  ifdef STATIC_LINKED
#    define WINDOWS_STATIC
#  else
#    define WINDOWS_SHARED
#  endif
#endif

namespace sml
{
	struct LibraryHandle
	{
#ifdef WINDOWS_SHARED
		HMODULE hLibrary;
#elif defined(LINUX_SHARED) 
		void* hLibrary;
#endif
	};
}

EmbeddedConnection::EmbeddedConnection()
{
	m_pLastResponse = new ElementXML() ;
	m_hConnection   = NULL ;
	m_pProcessMessageFunction = NULL ;
	m_pCreateEmbeddedFunction = NULL ;
	m_pLH = 0;
}

EmbeddedConnection::~EmbeddedConnection()
{
	delete m_pLastResponse ;
}

/*************************************************************
* @brief Simple function that does the casting from the handle
*		 (which we passed to the other side of the connection)
*		 back to its original object.
*************************************************************/
EmbeddedConnection* GetConnectionFromHandle(Connection_Receiver_Handle hConnection)
{
	return reinterpret_cast<EmbeddedConnection*>(hConnection);
}

/*************************************************************
* @brief	This is the raw function which will be called by the other
*			side of the embeddded connection to pass a message.
*			In some sense, all message passing starts here.
*
*	The parameters are defined as "handles".  That it, they are opaque
*	pointers.
*
*	The connection handle is our "EmbeddedConnection" object
*	which we pass to the other side of the embedded connection and it
*	passes back to us in each call.
*
*	The incoming message is a handle to an ElementXML object which is
*	allocated and owned by the ElementXML library.  Again it's opaque.
*	To work with it you need to pass this handle back to the DLL.
*
*	The reason for all this "opacity" is that it ensures we don't
*	accidentally bind the two processes too tightly together.
*	For example, if we allocated a C++ object for the incoming message
*	passed it to the other side of the connection and then accessed the
*	members directly we would get a crash if the headers used to build one
*	side of the connection didn't match those on the other side.
*	This happens if you build one side with one compiler and the other
*	with a different compiler, or change the definition of the object and
*	only rebuild one side of the connection.  In any case, the rule of
*	"he who creates is the only one who accesses and deletes" ensures no
*	such problems and that's the model we're using here.
*	
*************************************************************/
ElementXML_Handle LocalProcessMessage(Connection_Receiver_Handle hReceiverConnection, ElementXML_Handle hIncomingMsg, int action)
{
	// This is the connection object we created in this class, passed to the kernel and have
	// now received back.
	EmbeddedConnection* pConnection = reinterpret_cast<EmbeddedConnection*>(hReceiverConnection) ;

	// Make sure we have been passed a valid connection object.
	if (pConnection == NULL)
		return NULL ;

	if (action == SML_MESSAGE_ACTION_CLOSE)
	{
		// Close our connection to the remote process
		pConnection->ClearConnectionHandle() ;

		return NULL ;
	}

	// Synch connections are all happening within a single thread
	if (action == SML_MESSAGE_ACTION_SYNCH)
	{
		// Create an object to wrap this message.
		ElementXML incomingMsg(hIncomingMsg) ;

		// For a synchronous connection, immediately execute the incoming message, generating a response
		// which is immediately passed back to the caller.
		ElementXML* pResponse = pConnection->InvokeCallbacks(&incomingMsg) ;

		if (!pResponse)
			return NULL ;

		ElementXML_Handle hResponse = pResponse->Detach() ;
		delete pResponse ;
		return hResponse ;
	}

	// Asynch connections involve a thread switch.  The message comes in on
	// one thread, is dropped in a message queue and picked up by a second thread.
	if (action == SML_MESSAGE_ACTION_ASYNCH)
	{
		// Store the incoming message on a queue and execute it on the receiver's thread (our thread) at a later point.
		EmbeddedConnectionAsynch* eca = static_cast<EmbeddedConnectionAsynch*>(pConnection);
		eca->AddToIncomingMessageQueue(hIncomingMsg) ;

		// There is no immediate response to an asynch message.
		// The response will be sent back to the caller as another asynch message later, once the command has been executed.
		return NULL ;
	}

	// Not an action we understand, so just ignore it.
	// This allows future versions to use other actions if they wish and
	// we'll remain somewhat compatible.
	return NULL ;
}

/*************************************************************
* @brief Loads a library and creates an embedded connection to it.
*
*		 The library at a minimum needs to export 2 methods:
*		 sml_CreateEmbeddedConnection (for initialization) and
*		 sml_ProcessMessage (for sending messages)
*
*		 If KERNEL_SML_DIRECT is defined we also see if the
*		 the library exports a series of "Direct" methods.  These
*		 allow us to short circuit the message passing and call directly
*		 into gSKI for maximum performance on some I/O calls.
*
*		 If the library doesn't export these functions
*		 or if the caller doesn't pass "true" for optimized
*		 or if this is an asychronous (out of thread) connection
*		 then the "m_bIsDirectConnection" will be false and we'll
*		 just fall back on our normal message passing model.
*		 To date, it's not clear there's much value in supporting these
*		 direct methods, but they do allow a client that is really
*		 time critical to use SML with almost no performance hit over
*		 using gSKI directly.
*************************************************************/
bool EmbeddedConnection::AttachConnection(bool optimized, int portToListenOn)
{
	ClearError() ;
	m_pProcessMessageFunction = &sml_ProcessMessage;
	m_pCreateEmbeddedFunction = &sml_CreateEmbeddedConnection;

	// We only use the creation function once to create a connection object (which we'll pass back
	// with each call).
	int connectionType = this->IsAsynchronous() ? SML_ASYNCH_CONNECTION : SML_SYNCH_CONNECTION ;
	m_hConnection = m_pCreateEmbeddedFunction( Dangerous_Pointer_Cast<Connection_Sender_Handle>::from(this), LocalProcessMessage, connectionType, portToListenOn) ;

	if (!m_hConnection)
	{
		SetError(Error::kCreationFailed) ;
		return false ;
	}

	// When we reach here we have a connection object (m_hConnection) back from KernelSML and
	// we have the function (m_pProcessMessageFunction) that we'll use to communicate with that library.
	return true ;
}

/*************************************************************
* @brief Each side of the communication (client and kernel) will
*		 have an EmbeddedConnection object.
*
*		 This function is used to link the two together
*		 (e.g. this tells the client which object in the kernel to
*		  use when sending a command to the kernel).
*************************************************************/
void EmbeddedConnection::AttachConnectionInternal(Connection_Receiver_Handle hConnection, ProcessMessageFunction pProcessMessage)
{
	ClearError() ;
	m_hConnection = hConnection ;
	m_pProcessMessageFunction = pProcessMessage ;
}

/*************************************************************
* @brief Returns true if this connection has been closed or
*		 is otherwise not usable.
*************************************************************/
bool EmbeddedConnection::IsClosed()
{
	return (m_hConnection == NULL) ;
}

/*************************************************************
* @brief Shut down the connection.  In this case we release
*		 our connection handle.
*************************************************************/
void EmbeddedConnection::CloseConnection()
{
	ClearError() ;

	if (m_hConnection)
	{
		// Make the call to the kernel to close this connection
		m_pProcessMessageFunction(m_hConnection, 0, SML_MESSAGE_ACTION_CLOSE) ;
	}
	
	m_hConnection = NULL ;

	if (m_pLH)
	{
#ifdef WINDOWS_SHARED
		FreeLibrary(m_pLH->hLibrary);
#elif defined(LINUX_SHARED) 
		dlclose(m_pLH->hLibrary);
#endif
		delete m_pLH;
		m_pLH = 0;
	}
}

/*************************************************************
* @brief Setting this to true prints out lots of debug
*		 information to stderr and a trace file.
*************************************************************/
void EmbeddedConnection::SetTraceCommunications(bool state)
{
	ClearError() ;

	m_bTraceCommunications = state ;

	if (m_hConnection)
	{
		// Tell the kernel to turn tracing on or off
		m_pProcessMessageFunction(m_hConnection, 0, state ? SML_MESSAGE_ACTION_TRACE_ON : SML_MESSAGE_ACTION_TRACE_OFF) ;
	}
}
