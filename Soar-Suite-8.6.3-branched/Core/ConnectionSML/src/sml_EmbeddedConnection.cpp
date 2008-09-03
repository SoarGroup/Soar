#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

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

#include "sml_EmbeddedConnection.h"
#include "sml_EmbeddedConnectionAsynch.h"
#include "sml_ElementXML.h"
#include "sml_MessageSML.h"
#include "thread_Thread.h"
#include "sock_Debug.h"

#include <string>
#include <iostream>
#include <assert.h>

#ifdef _WIN32
#include "Windows.h"	// Needed for load library
#undef SendMessage		// Windows defines this as a macro.  Yikes!

#elif defined(HAVE_DLFCN_H)
#include <dlfcn.h>      // Needed for dlopen and dlsym
#define GetProcAddress dlsym

#endif // _WIN32

using namespace sml ;

EmbeddedConnection::EmbeddedConnection()
{
	m_pLastResponse = new ElementXML() ;
	m_hConnection   = NULL ;
	m_pProcessMessageFunction = NULL ;
	m_pCreateEmbeddedFunction = NULL ;
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
	return (EmbeddedConnection*)hConnection ;
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
	EmbeddedConnection* pConnection = (EmbeddedConnection*)hReceiverConnection ;

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
		ElementXML* pIncomingMsg = new ElementXML(hIncomingMsg) ;

		((EmbeddedConnectionAsynch*)pConnection)->AddToIncomingMessageQueue(pIncomingMsg) ;

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
bool EmbeddedConnection::AttachConnection(char const* pLibraryName, bool optimized, int portToListenOn)
{
	ClearError() ;

	// Make a copy of the library name so we can work on it.
	std::string libraryName = pLibraryName ;

	// We shouldn't be passed something with an extension
	// but if we are, we'll try to strip the extension to be helpful
	size_t pos = libraryName.find_last_of('.') ;
	if (pos != std::string::npos)
	{
		libraryName.erase(pos) ;
	}

#ifdef _WIN32
#  ifdef STATIC_LINKED
#    define WINDOWS_STATIC
#  else
#    define WINDOWS_SHARED
#  endif
#endif

#ifdef HAVE_CONFIG_H //to test for Linux/OS X
#  ifdef STATIC_LINKED
#    define LINUX_STATIC
#  elif defined(HAVE_DLFCN_H) //if we don't have this, then we're implicitly doing a static build
#    define LINUX_SHARED
#  endif
#endif

#ifdef WINDOWS_SHARED
	// The windows shared library
	libraryName = libraryName + ".dll";
	
	// Now load the library itself.
	HMODULE hLibrary = LoadLibrary(libraryName.c_str()) ;

#elif defined(LINUX_SHARED) 
	std::string newLibraryName = "lib" + libraryName + ".so";
	void* hLibrary = 0;
	hLibrary = dlopen(newLibraryName.c_str(), RTLD_LAZY);
	if (!hLibrary) {
		// Try again with mac extention
		newLibraryName = "lib" + libraryName + ".dylib";
		hLibrary = dlopen(newLibraryName.c_str(), RTLD_LAZY);
	}
	// FIXME error details can be returned by a call to dlerror()
#endif

#if defined(LINUX_SHARED) || defined(WINDOWS_SHARED)
	if (!hLibrary)
	{
		SetError(Error::kLibraryNotFound) ;
		return false ;
	}

	// Get the functions that a DLL must export to support an embedded connection.
	m_pProcessMessageFunction = (ProcessMessageFunction)GetProcAddress(hLibrary, "sml_ProcessMessage") ;
	m_pCreateEmbeddedFunction = (CreateEmbeddedConnectionFunction)GetProcAddress(hLibrary, "sml_CreateEmbeddedConnection") ;

#ifdef KERNEL_SML_DIRECT
	m_pDirectAddWMEStringFunction =		(DirectAddWMEStringFunction)GetProcAddress(hLibrary, "sml_DirectAddWME_String") ;
	m_pDirectAddWMEIntFunction =		(DirectAddWMEIntFunction)GetProcAddress(hLibrary, "sml_DirectAddWME_Int") ;
	m_pDirectAddWMEDoubleFunction =		(DirectAddWMEDoubleFunction)GetProcAddress(hLibrary, "sml_DirectAddWME_Double") ;
	m_pDirectRemoveWMEFunction =		(DirectRemoveWMEFunction)GetProcAddress(hLibrary, "sml_DirectRemoveWME") ;

	m_pDirectAddIDFunction =			(DirectAddIDFunction)GetProcAddress(hLibrary, "sml_DirectAddID") ;
	m_pDirectLinkIDFunction =			(DirectLinkIDFunction)GetProcAddress(hLibrary, "sml_DirectLinkID") ;
	m_pDirectGetThisWMObjectFunction =	(DirectGetThisWMObjectFunction)GetProcAddress(hLibrary, "sml_DirectGetThisWMObject") ;

	m_pDirectGetRootFunction =			(DirectGetRootFunction)GetProcAddress(hLibrary, "sml_DirectGetRoot") ;
	m_pDirectGetWorkingMemoryFunction = (DirectGetWorkingMemoryFunction)GetProcAddress(hLibrary, "sml_DirectGetWorkingMemory") ;
	m_pDirectRunFunction =			    (DirectRunFunction)GetProcAddress(hLibrary, "sml_DirectRun") ;
	
	m_pDirectReleaseWMEFunction =		(DirectReleaseWMEFunction)GetProcAddress(hLibrary, "sml_DirectReleaseWME") ;
	m_pDirectReleaseWMObjectFunction =	(DirectReleaseWMObjectFunction)GetProcAddress(hLibrary, "sml_DirectReleaseWMObject") ;

	// Check that we got the list of functions and if so enable the direct connection
	if (m_pDirectAddWMEStringFunction && m_pDirectAddWMEIntFunction && m_pDirectAddWMEDoubleFunction &&
		m_pDirectRemoveWMEFunction    && m_pDirectAddIDFunction     && m_pDirectLinkIDFunction &&
		m_pDirectGetThisWMObjectFunction && m_pDirectGetRootFunction && m_pDirectGetWorkingMemoryFunction &&
		m_pDirectReleaseWMEFunction && m_pDirectReleaseWMObjectFunction && m_pDirectRunFunction)
	{
		// We only enable direct connections if we found all of the methods, this is a synchronous connection (i.e. we execute
		// on the client's thread) and the client says it's ok to use these optimizations.
		if (optimized && !IsAsynchronous())
			m_bIsDirectConnection = true ;
	}
#endif

	// See if we got the functions
	if (!m_pProcessMessageFunction || !m_pCreateEmbeddedFunction)
	{
		SetError(Error::kFunctionsNotFound) ;
		return false ;
	}

#else // defined(LINUX_SHARED) || defined(WINDOWS_SHARED)
	// If we're not in Windows and we can't dynamically load methods we'll just get
	// by with the two we really need.  This just means we can't get maximum optimization on
	// this particular platform.
	m_pProcessMessageFunction = &sml_ProcessMessage;
	m_pCreateEmbeddedFunction = &sml_CreateEmbeddedConnection;

#endif // defined(LINUX_SHARED) || defined(WINDOWS_SHARED)

	// We only use the creation function once to create a connection object (which we'll pass back
	// with each call).
	int connectionType = this->IsAsynchronous() ? SML_ASYNCH_CONNECTION : SML_SYNCH_CONNECTION ;
	m_hConnection = m_pCreateEmbeddedFunction( (Connection_Sender_Handle)this, LocalProcessMessage, connectionType, portToListenOn) ;

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
		ElementXML_Handle hResponse = m_pProcessMessageFunction(m_hConnection, (ElementXML_Handle)NULL, SML_MESSAGE_ACTION_CLOSE) ;
		unused(hResponse) ;
	}
	
	m_hConnection = NULL ;
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
		ElementXML_Handle hResponse = m_pProcessMessageFunction(m_hConnection, (ElementXML_Handle)NULL, state ? SML_MESSAGE_ACTION_TRACE_ON : SML_MESSAGE_ACTION_TRACE_OFF) ;
		unused(hResponse) ;
	}
}
