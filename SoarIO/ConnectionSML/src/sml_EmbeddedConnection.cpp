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

#include "sml_EmbeddedConnection.h"
#include "sml_ElementXML.h"
#include "thread_Thread.h"

#include <string>

#ifdef _WIN32
#include "Windows.h"	// Needed for load library

#undef SendMessage		// Windows defines this as a macro.  Yikes!
#endif // WIN32

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

EmbeddedConnection* GetConnectionFromHandle(Connection_Receiver_Handle hConnection)
{
	return (EmbeddedConnection*)hConnection ;
}

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

	if (action == SML_MESSAGE_ACTION_SYNCH)
	{
		// Create an object to wrap this message.
		ElementXML incomingMsg(hIncomingMsg) ;

		ElementXML* pResponse = pConnection->InvokeCallbacks(&incomingMsg) ;

		if (!pResponse)
			return NULL ;

		ElementXML_Handle hResponse = pResponse->Detach() ;
		delete pResponse ;
		return hResponse ;
	}

	if (action == SML_MESSAGE_ACTION_ASYNCH)
	{
		// Store the incoming message on a queue and execute it on the receiver's thread (our thread) at a later point.
		ElementXML* pIncomingMsg = new ElementXML(hIncomingMsg) ;

		pConnection->AddToIncomingMessageQueue(pIncomingMsg) ;

		// There is no immediate response to an asynch message.
		// The response will be sent back to the caller as another asynch message later, once the command has been executed.
		return NULL ;
	}

	// Not an action we understand, so just ignore it.
	// This allows future versions to use other actions if they wish and
	// we'll remain somewhat compatible.
	return NULL ;
}

bool EmbeddedConnection::AttachConnection(char const* pLibraryName)
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
	// The windows shared library
	libraryName.append(".dll") ;

	// Now load the library itself.
	HMODULE hLibrary = LoadLibrary(libraryName.c_str()) ;

	if (!hLibrary)
	{
		SetError(Error::kLibraryNotFound) ;
		return false ;
	}

	// Get the functions that a DLL must export to support an embedded connection.
	m_pProcessMessageFunction = (ProcessMessageFunction)GetProcAddress(hLibrary, "sml_ProcessMessage") ;
	m_pCreateEmbeddedFunction = (CreateEmbeddedConnectionFunction)GetProcAddress(hLibrary, "sml_CreateEmbeddedConnection") ;

	// See if we got the functions
	if (!m_pProcessMessageFunction || !m_pCreateEmbeddedFunction)
	{
		SetError(Error::kFunctionsNotFound) ;
		return false ;
	}

	// We only use the creation function once to create a connection object (which we'll pass back
	// with each call).
	int connectionType = this->IsAsynchronous() ? SML_ASYNCH_CONNECTION : SML_SYNCH_CONNECTION ;
	m_hConnection = m_pCreateEmbeddedFunction( (Connection_Sender_Handle)this, LocalProcessMessage, connectionType) ;

	if (!m_hConnection)
	{
		SetError(Error::kCreationFailed) ;
		return false ;
	}

	// When we reach here we have a connection object (m_hConnection) back from KernelSML and
	// we have the function (m_pProcessMessageFunction) that we'll use to communicate with that library.
	return true ;
#else
// BUGBUG: We'll need to do a Linux equivalent here.  Not sure what the equivalents are.
	return false ;
#endif
}

// Link two embedded connections together.
void EmbeddedConnection::AttachConnection(Connection_Receiver_Handle hConnection, ProcessMessageFunction pProcessMessage)
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

void EmbeddedConnectionSynch::SendMessage(ElementXML* pMsg)
{
	ClearError() ;

	// Check that we have somebody to send this message to.
	if (m_hConnection == NULL)
	{
		SetError(Error::kNoEmbeddedLink) ;
		return ;
	}

	ElementXML_Handle hResponse = NULL ;

	// Add a reference to this object, which will then be released by the receiver of this message when
	// they are done with it.
	pMsg->AddRefOnHandle() ;
	ElementXML_Handle hSendMsg = pMsg->GetXMLHandle() ;

	// Make the call to the kernel, passing the message over and getting an immediate response since this is
	// an embedded call.
	hResponse = m_pProcessMessageFunction(m_hConnection, hSendMsg, SML_MESSAGE_ACTION_SYNCH) ;

	// We cache the response
	m_pLastResponse->Attach(hResponse) ;
}

ElementXML* EmbeddedConnectionSynch::GetResponseForID(char const* pID, bool wait)
{
	// For the embedded connection there's no ambiguity over what was the "last" call.
	unused(pID) ;

	// There's also no need to wait, we always have the result on hand.
	unused(wait) ;
	
	ClearError() ;

	ElementXML_Handle hResponse = m_pLastResponse->Detach() ;

	if (!hResponse)
		return NULL ;

	// We create a new wrapper object and return that.
	// (If we returned a pointer to m_LastResponse it could change when new messages come in).
	ElementXML* pResult = new ElementXML(hResponse) ;
	return pResult ;
}

void EmbeddedConnectionAsynch::SendMessage(ElementXML* pMsg)
{
	ClearError() ;

	// Check that we have somebody to send this message to.
	if (m_hConnection == NULL)
	{
		SetError(Error::kNoEmbeddedLink) ;
		return ;
	}

	ElementXML_Handle hResponse = NULL ;

	// Add a reference to this object, which will then be released by the receiver of this message when
	// they are done with it.
	pMsg->AddRefOnHandle() ;
	ElementXML_Handle hSendMsg = pMsg->GetXMLHandle() ;

	// Make the call to the kernel, passing the message over with the ASYNCH flag, which means there
	// will be no immediate response.
	hResponse = m_pProcessMessageFunction(m_hConnection, hSendMsg, SML_MESSAGE_ACTION_ASYNCH) ;

	if (hResponse != NULL)
	{
		SetError(Error::kInvalidResponse) ;
	}
}

static bool DoesResponseMatch(ElementXML* pResponse, char const* pID)
{
	if (!pResponse || !pID)
		return false ;

	char const* pMsgID = pResponse->GetAttribute(sml_Names::kID) ;
	
	return (pMsgID && strcmp(pMsgID, pID) == 0) ;
}

ElementXML* EmbeddedConnectionAsynch::GetResponseForID(char const* pID, bool wait)
{
	ElementXML* pResponse = NULL ;

	// Check if we already have this response cached
	if (DoesResponseMatch(m_pLastResponse, pID))
	{
		pResponse = m_pLastResponse ;
		m_pLastResponse = NULL ;
		return pResponse ;
	}

// BUGBUG: I don't think we can time out on these.
// We just need to have a way to detect if the connection has been closed
// The problem is Soar can run for an arbitrary amount of time and we'll
// be waiting for the response
// Logic should be something like
// while (ConnectionNotClosed()) { CheckForMessages() ; Sleep() ; }
// We may need a better event based model to handle this, so no sleeping etc.
// The trick is to include a "closed connection" message in the queue.
// Then we can wait on an incoming message forever and still wake up if the connection dies.

	int sleepTime = 5 ;			// How long we sleep in milliseconds each pass through
	int maxRetries = 4000 ;		// This times sleepTime gives the timeout period e.g. (4000 * 5 == 20 secs)

	// If we don't already have this response cached,
	// then read any pending messages.
	do
	{
		// Loop until there are no more messages waiting for us
		while (ReceiveMessages(false))
		{
			// Check each message to see if it's a match
			if (DoesResponseMatch(m_pLastResponse, pID))
			{
				pResponse = m_pLastResponse ;
				m_pLastResponse = NULL ;
				return pResponse ;
			}
		}

		// At this point we didn't find the message sitting on the incoming connection
		// so we need to decide if we should wait or not.
		if (wait)
		{
			soar_thread::Thread::SleepStatic(sleepTime) ;
			maxRetries-- ;

			// Eventually time out
			if (maxRetries == 0)
			{
				SetError(Error::kConnectionTimedOut) ;			
				wait = false ;
			}
		}

	} while (wait) ;

	// If we get here we didn't find the response.
	// Either it's not come in yet (and we didn't choose to wait for it)
	// or we've timed out waiting for it.
	return NULL ;
}

bool EmbeddedConnectionAsynch::ReceiveMessages(bool allMessages)
{
	bool receivedMessage = false ;

	ElementXML* pIncomingMsg = PopIncomingMessageQueue() ;

	// While we have messages waiting to come in keep reading them
	while (pIncomingMsg)
	{
		// Record that we got at least one message
		receivedMessage = true ;

		// Pass this message back to the client and possibly get their response
		ElementXML* pResponse = this->InvokeCallbacks(pIncomingMsg) ;

		// If we got a response to the incoming message, send that response back.
		if (pResponse)
		{
			SendMessage(pResponse) ;		
		}

		// We're done with the response
		delete pResponse ;

		// Record the last incoming message
		delete m_pLastResponse ;
		m_pLastResponse = pIncomingMsg ;

		// If we're only asked to read one message, we're done.
		if (!allMessages)
			break ;

		// Get the next message from the queue
		pIncomingMsg = PopIncomingMessageQueue() ;
	}

	return receivedMessage ;
}
