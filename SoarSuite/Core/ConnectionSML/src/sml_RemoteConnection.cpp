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
// NOTE: This class is VERY similar to the EmbeddedConnectionAsynch class.  In fact we should
// probably fold them together at some point with a common base class.  But for now, be aware
// that if you're changing something here you should probably also be changing it there.
//
/////////////////////////////////////////////////////////////////

#include "sml_RemoteConnection.h"
#include "sock_Socket.h"
#include "sock_Debug.h"
#include "thread_Thread.h"

#include <assert.h>

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

	for (MessageListIter iter = m_ReceivedMessageList.begin() ; iter != m_ReceivedMessageList.end() ; iter++)
	{
		ElementXML* xml = (*iter) ;
		delete xml ;
	}
}

/*************************************************************
* @brief Adds the message to a queue of responses which we're waiting
*		 to pair with the commands that triggered them.
*
*		 This function takes ownership of the object it is passed,
*		 so the caller should not delete it subsequently.
*
*		 The messages kept on this list will all have "ack" fields
*		 as they are responses to commands that have come out of the
*		 expected order.  This can only happen when multiple threads
*		 submit commands.
*************************************************************/
void RemoteConnection::AddResponseToList(ElementXML* pResponse)
{
	if (pResponse == NULL)
		return ;

	// If this message isn't a response to a command we don't need to keep it
	// because we will never need to retrieve it.
	const char* pAckID = pResponse->GetAttribute(sml_Names::kAck) ;

	if (!pAckID)
	{
		delete pResponse ;
		return ;
	}

	soar_thread::Lock lock(&m_ListMutex) ;

	m_ReceivedMessageList.push_front(pResponse) ;

	if (m_bTraceCommunications)
		PrintDebugFormat("!! Adding ack for id %s to the pending message list", pAckID) ;

	// We keep the received message list from growing indefinitely.  This is because
	// a client may send a command and choose not to listen for the response.
	// (I don't believe this happens today, but it is allowed by the API).
	// In that case the message would remain on this list forever and if we allowed it
	// to grow over time we could be searching an ever increasingly large list of dead messages
	// that will never be retrieved.  I believe (but haven't conclusively proved to my satisfaction yet)
	// that we will never have more messages pending here, for which the client is interested in the
	// response, than there are threads sending commands, so a small max list size should be fine.
	while (m_ReceivedMessageList.size() > kMaxListSize)
	{
		if (m_bTraceCommunications)
			PrintDebugFormat("Had to clean a message from the pending message list") ;

		ElementXML* pLast = m_ReceivedMessageList.back() ;
		delete pLast ;
		m_ReceivedMessageList.pop_back() ;
	}
}

/*************************************************************
* @brief	Searches the list of responses to see if there's already
*			been a response generated for this particular message ID.
*
* The list of messages has a fixed maximum size, so this lookup is
* a constant time operation.  If the client is only issuing
* calls on a single thread, the list will always be empty.
*************************************************************/
ElementXML* RemoteConnection::IsResponseInList(char const* pID)
{
	soar_thread::Lock lock(&m_ListMutex) ;

	for (MessageListIter iter = m_ReceivedMessageList.begin() ; iter != m_ReceivedMessageList.end() ; iter++)
	{
		ElementXML* pXML = (*iter) ;
		if (DoesResponseMatch(pXML, pID))
		{
			if (m_bTraceCommunications)
				PrintDebugFormat("Found match for %s in pending message list", pID) ;

			m_ReceivedMessageList.erase(iter) ;
			return pXML ;
		}
	}

	return NULL ;
}

/*************************************************************
* @brief	Returns true if the given response message is
*			an acknowledgement for a message with the given ID.
*************************************************************/
bool RemoteConnection::DoesResponseMatch(ElementXML* pResponse, char const* pID)
{
	if (!pResponse || !pID)
		return false ;

	char const* pMsgID = pResponse->GetAttribute(sml_Names::kAck) ;
	
	if (!pMsgID)
		return false ;

	// Spelling this test out so we can put break points in if we wish.
	if (strcmp(pMsgID, pID) == 0)
		return true ;

	if (m_bTraceCommunications)
		PrintDebugFormat("Received ack for message %s while looking for %s", pMsgID, pID) ;

	return false ;
}

/*************************************************************
* @brief Send a message to the other side of this connection.
*
* For an remote connection this is done by sending the command
* over a socket as an actual XML string.
*
* There is no immediate response because we have to wait for
* the other side to read from the socket and execute the command.
* To get a response call GetResponseForID()
* and wait for the response to occur.
*************************************************************/
void RemoteConnection::SendMessage(ElementXML* pMsg)
{
	ClearError() ;

	// Convert the message to an XML string
	char* pXMLString = pMsg->GenerateXMLString(true) ;

	// Send it
	bool ok = m_Socket->SendString(pXMLString) ;

	// Dump the message if we're tracing
	if (m_bTraceCommunications)
	{
		if (IsKernelSide())
			PrintDebugFormat("Kernel remote send: %s\n", pXMLString) ;
		else
			PrintDebugFormat("Client remote send: %s\n", pXMLString) ;
	}

	// Release the XML string
	pMsg->DeleteString(pXMLString) ;

	// If we had an error close the connection
	if (!ok)
	{
		PrintDebug("Socket has closed down abruptly (during send), so we'll close the connection") ;
		SetError(Error::kSocketError) ;
		CloseConnection() ;
	}
}

/*************************************************************
* @brief	Look for a response to the given message (based on its ID).
*			Optionally, wait for that response to come in.
*************************************************************/
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

	// Also check the list of responses we've stored
	// (This list will always be empty if we're only executing commands
	//  on one thread, but if we are using multiple threads it can come into play).
	pResponse = IsResponseInList(pID) ;
	if (pResponse)
	{
		return pResponse ;
	}

	long sleepTimeSecs = 0 ;			// How long we sleep in seconds each pass through
	long sleepTimeMillisecs = 0 ;		// How long we sleep in milliseconds each pass through

	// How long we sleep on the socket waiting for data in msecs
	// We want to wait for a long time.  We used to set this to 0 and just poll the socket,
	// but that means we're consuming all of the CPU.  Setting a long wait doesn't
	// impact performance because we're not trying to do anything else other than get a response here.
	long waitForMessageTimeSeconds = 1 ;
	long waitForMessageTimeMilliseconds = 0 ;

	// If we don't already have this response cached,
	// then read any pending messages.
	do
	{
		// Loop until there are no more messages waiting on the socket
		while (ReceiveMessages(false, waitForMessageTimeSeconds, waitForMessageTimeMilliseconds))
		{
			// Check each message to see if it's a match
			if (DoesResponseMatch(m_pLastResponse, pID))
			{
				pResponse = m_pLastResponse ;
				m_pLastResponse = NULL ;
				return pResponse ;
			} else {
				AddResponseToList(m_pLastResponse) ;
				m_pLastResponse = NULL ;
			}
		}

		// Check to see if the message has been added to the list of
		// waiting messages.  This could have happened on a different
		// thread while we were in here waiting.
		ElementXML* pResponse = IsResponseInList(pID) ;
		if (pResponse != NULL)
		{
			return pResponse ;
		}

		// Allow other threads the chance to update
		// (by calling with 0 for sleep time we don't give away cycles if
		//  no other thread is waiting to execute).
		soar_thread::Thread::SleepStatic(sleepTimeSecs, sleepTimeMillisecs) ;

		// Check if the connection has been closed
		if (IsClosed())
			return NULL ;

	} while (wait) ;

	// If we get here we didn't find the response.
	// (If we're waiting we'll wait forever, so we'll only get here if
	//  we chose not to wait).
	return NULL ;
}

/*************************************************************
* @brief	Retrieve any messages we've been sent and process them.
*
*			Returns true if at least one message has been read.
*************************************************************/
bool RemoteConnection::ReceiveMessages(bool allMessages)
{
	return ReceiveMessages(allMessages, 0, 0) ;
}

bool RemoteConnection::ReceiveMessages(bool allMessages, long secondsWait, long millisecondsWait)
{
	assert(millisecondsWait<1000 && "specified milliseconds must be less than 1000");

	// Make sure only one thread is sending messages at a time
	// (This allows us to run a separate thread in clients polling for events even
	//  when the client is sleeping, but we don't want them both to be sending/receiving at the same time).
	soar_thread::Lock lock(&m_ClientMutex) ;

	std::string xmlString ;
	bool receivedMessage = false ;
	bool ok = true ;

	// While we have messages waiting to come in keep reading them
	bool haveData = true ;

	while (haveData)
	{
		bool alive = m_Socket->IsAlive() ;
		if (!alive)
		{
			// The socket has closed down so close our connection object too.
			this->CloseConnection() ;
			return receivedMessage ;
		}

		// Only check for read data after we've checked that the socket is still alive.
		// (This is because IsReadData can't signal the difference between a dead connection and no data)
		haveData = m_Socket->IsReadDataAvailable(secondsWait, millisecondsWait) ;
		if (!haveData)
			break ;

		// 	Read the first message that's waiting
		ok = m_Socket->ReceiveString(&xmlString) ;

		if (!ok)
		{
			this->SetError(Error::kSocketError) ;
			this->CloseConnection() ;

			return receivedMessage ;
		}

		// Dump the message if we're tracing
		if (m_bTraceCommunications)
		{
			if (IsKernelSide())
				PrintDebugFormat("Kernel remote receive: %s\n", xmlString.c_str()) ;
			else
				PrintDebugFormat("Client remote receive: %s\n", xmlString.c_str()) ;
		}

		// Get an XML message from the incoming string
		ElementXML* pIncomingMsg = ElementXML::ParseXMLFromString(xmlString.c_str()) ;

		if (!pIncomingMsg)
		{
			this->SetError(Error::kParsingXMLError) ;
			return receivedMessage ;
		}

#ifdef _DEBUG
		// Check that the parse worked
		//char* pMsgText = pIncomingMsg->GenerateXMLString(true) ;
		//pIncomingMsg->DeleteString(pMsgText) ;
#endif

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

