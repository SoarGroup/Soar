#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// RemoteConnection class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This class represents a logical connection between two entities that are communicating via SML over a socket.
// For example, an environment (the client) and the Soar kernel.
//
/////////////////////////////////////////////////////////////////

#include "sml_RemoteConnection.h"
#include "sock_Socket.h"
#include "sock_Debug.h"
#include "thread_Thread.h"

using namespace sml ;

RemoteConnection::RemoteConnection(bool sharedFileSystem, sock::Socket* pSocket)
{
	m_SharedFileSystem = sharedFileSystem ;
	m_Socket = pSocket ;
	m_pLastResponse = NULL ;
}

RemoteConnection::~RemoteConnection()
{
	delete m_pLastResponse ;
	delete m_Socket ;
}

void RemoteConnection::SendMessage(ElementXML* pMsg)
{
	ClearError() ;

	// Convert the message to an XML string
	char* pXMLString = pMsg->GenerateXMLString(true) ;

	// Send it
	m_Socket->SendString(pXMLString) ;

	// Dump the message if we're tracing
	if (m_bTraceCommunications)
		PrintDebug(pXMLString) ;

	// Release the XML string
	pMsg->DeleteString(pXMLString) ;
}

static bool DoesResponseMatch(ElementXML* pResponse, char const* pID)
{
	if (!pResponse || !pID)
		return false ;

	char const* pMsgID = pResponse->GetAttribute(sml_Names::kAck) ;
	
	if (!pMsgID)
		return false ;

	// Spelling this test out so we can put break points in if we wish.
	if (strcmp(pMsgID, pID) == 0)
		return true ;
	else
		return false ;
}

ElementXML* RemoteConnection::GetResponseForID(char const* pID, bool wait)
{
	ElementXML* pResponse = NULL ;

	// Check if we already have this response cached
	if (DoesResponseMatch(m_pLastResponse, pID))
	{
		pResponse = m_pLastResponse ;
		m_pLastResponse = NULL ;
		return pResponse ;
	}

	int sleepTime = 0 ;			// How long we sleep in milliseconds each pass through

	// If we don't already have this response cached,
	// then read any pending messages.
	do
	{
		// Loop until there are no more messages waiting on the socket
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

		// At this point we didn't find the message sitting on the socket
		// so we need to decide if we should wait or not.
		if (wait)
		{
			soar_thread::Thread::SleepStatic(sleepTime) ;

			// Check if the connection has been closed
			if (IsClosed())
				return NULL ;
		}

	} while (wait) ;

	// If we get here we didn't find the response.
	// Either it's not come in yet (and we didn't choose to wait for it)
	// or we've timed out waiting for it.
	return NULL ;
}

bool RemoteConnection::ReceiveMessages(bool allMessages)
{
	// Make sure only one thread is sending messages at a time
	// (This allows us to run a separate thread in clients polling for events even
	//  when the client is sleeping, but we don't want them both to be sending/receiving at the same time).
	soar_thread::Lock lock(&m_ClientMutex) ;

	std::string xmlString ;
	bool receivedMessage = false ;
	bool ok = true ;

	// While we have messages waiting to come in keep reading them
	while (m_Socket->IsReadDataAvailable())
	{
		// 	Read the first message that's waiting
		ok = m_Socket->ReceiveString(&xmlString) ;

		if (!ok)
		{
			this->SetError(Error::kSocketError) ;
			return receivedMessage ;
		}

		// Dump the message if we're tracing
		if (m_bTraceCommunications)
			PrintDebug(xmlString.c_str()) ;

		// Get an XML message from the incoming string
		ElementXML* pIncomingMsg = ElementXML::ParseXMLFromString(xmlString.c_str()) ;

		if (!pIncomingMsg)
		{
			this->SetError(Error::kParsingXMLError) ;
			return receivedMessage ;
		}

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
	}

	return receivedMessage ;
}

void RemoteConnection::SetTraceCommunications(bool state)
{
	m_bTraceCommunications = state ;
	if (m_Socket) m_Socket->SetTraceCommunications(state) ;
}

void RemoteConnection::CloseConnection()
{
	m_Socket->CloseSocket() ;
}

bool RemoteConnection::IsClosed()
{
	return !m_Socket->IsAlive() ;
}

