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

#ifdef _WIN32
#include "Windows.h"
#endif

#include "sml_EmbeddedConnection.h"
#include "sml_ElementXML.h"

#include <string>

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

	if (action == MESSAGE_ACTION_CLOSE)
	{
		delete pConnection ;

		return NULL ;
	}

	if (action == MESSAGE_ACTION_NORMAL)
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
	if (pos != -1)
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
	m_hConnection = m_pCreateEmbeddedFunction( (Connection_Sender_Handle)this, LocalProcessMessage) ;

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


void EmbeddedConnection::SendMessage(ElementXML* pMsg)
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
	hResponse = m_pProcessMessageFunction(m_hConnection, hSendMsg, MESSAGE_ACTION_NORMAL) ;

	// We cache the response
	m_pLastResponse->Attach(hResponse) ;
}

ElementXML* EmbeddedConnection::GetResponseForID(char const* pID, bool wait)
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

void EmbeddedConnection::CloseConnection()
{
	ClearError() ;

	if (m_hConnection)
	{
		// Make the call to the kernel to close this connection
		ElementXML_Handle hResponse = m_pProcessMessageFunction(m_hConnection, (ElementXML_Handle)NULL, MESSAGE_ACTION_CLOSE) ;
		unused(hResponse) ;
	}
	
	m_hConnection = NULL ;
}