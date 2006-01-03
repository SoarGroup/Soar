#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// EmbeddedConnectionAsynch class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class represents a logical connection between two entities that are communicating
// via SML (a form of XML).  In the embedded case that this class represents, both entities
// are within the same process.  For the "Asynch" variant, the two entities execute in
// different threads.
//
// NOTE: This class is VERY similar to the RemoteConnection class.  In fact we should
// probably fold them together at some point with a common base class.  But for now, be aware
// that if you're changing something here you should probably also be changing it there.
//
/////////////////////////////////////////////////////////////////

#include "sml_EmbeddedConnectionAsynch.h"
#include "sml_ElementXML.h"
#include "sml_MessageSML.h"
#include "thread_Thread.h"
#include "sock_Debug.h"

#include <string>
#include <iostream>
#include <assert.h>

using namespace sml ;

EmbeddedConnectionAsynch::~EmbeddedConnectionAsynch()
{
	for (MessageListIter iter = m_ReceivedMessageList.begin() ; iter != m_ReceivedMessageList.end() ; iter++)
	{
		ElementXML* xml = (*iter) ;
		delete xml ;
	}
}

/*************************************************************
* @brief Send a message to the other side of this connection.
*
* For an asynchronous connection this is done by adding
* the message to a queue owned by the receiver.
*
* There is no immediate response because we have to wait for
* a context switch and another thread to run to actually execute
* this command.  To get a response call GetResponseForID()
* and wait for the response to occur.
*************************************************************/
void EmbeddedConnectionAsynch::SendMessage(ElementXML* pMsg)
{
	ClearError() ;

	// Check that we have somebody to send this message to.
	if (m_hConnection == NULL)
	{
		SetError(Error::kNoEmbeddedLink) ;
		return ;
	}

	// Add a reference to this object, which will then be released by the receiver of this message when
	// they are done with it.
	pMsg->AddRefOnHandle() ;
	ElementXML_Handle hSendMsg = pMsg->GetXMLHandle() ;

#ifdef _DEBUG
	if (IsTracingCommunications())
	{
		char* pStr = pMsg->GenerateXMLString(true) ;
		PrintDebugFormat("%s Sending %s\n", IsKernelSide() ? "Kernel" : "Client", pStr) ;
		pMsg->DeleteString(pStr) ;
	}
#endif

	// Make the call to the kernel, passing the message over with the ASYNCH flag, which means there
	// will be no immediate response.
	ElementXML_Handle hResponse = m_pProcessMessageFunction(m_hConnection, hSendMsg, SML_MESSAGE_ACTION_ASYNCH) ;

	if (hResponse != NULL)
	{
		SetError(Error::kInvalidResponse) ;
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
void EmbeddedConnectionAsynch::AddResponseToList(ElementXML* pResponse)
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
ElementXML* EmbeddedConnectionAsynch::IsResponseInList(char const* pID)
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
bool EmbeddedConnectionAsynch::DoesResponseMatch(ElementXML* pResponse, char const* pID)
{
	if (!pResponse || !pID)
		return false ;

	char const* pMsgID = pResponse->GetAttribute(sml_Names::kAck) ;
	
	if (!pMsgID)
		return false ;

	if (strcmp(pMsgID, pID) == 0)
		return true ;
	
	if (m_bTraceCommunications)
		PrintDebugFormat("Received ack for message %s while looking for %s", pMsgID, pID) ;

	return false ;
}

/*************************************************************
* @brief	Look for a response to the given message (based on its ID).
*			Optionally, wait for that response to come in.
*************************************************************/
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

	// Also check the list of responses we've stored
	// (This list will always be empty if we're only executing commands
	//  on one thread, but if we are using multiple threads it can come into play).
	pResponse = IsResponseInList(pID) ;
	if (pResponse)
	{
		return pResponse ;
	}

	// How long we sleep in seconds+milliseconds each pass through
	// (0 means we only sleep if another thread is scheduled to run --
	//  it ensures maximum performance otherwise).
	long sleepTimeSecs = 0 ;
	long sleepTimeMillisecs = 0 ;

	// How long we will wait before checking for a message (in msecs)
	// (If one comes in it'll wake us up from this immediately, but having
	//  a timeout ensures we don't get stuck forever somehow).
	long maximumWaitTimeSeconds = 1 ;
	long maximumWaitTimeMilliseconds = 0 ;

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

		// Wait for a response for up to a second
		// If one comes in it will trigger this event to wake us up immediately.
		m_WaitEvent.WaitForEvent(maximumWaitTimeSeconds, maximumWaitTimeMilliseconds) ;

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
bool EmbeddedConnectionAsynch::ReceiveMessages(bool allMessages)
{
	// Make sure only one thread is sending messages at a time
	// (This allows us to run a separate thread in clients polling for events even
	//  when the client is sleeping, but we don't want them both to be sending/receiving at the same time).
	soar_thread::Lock lock(&m_ClientMutex) ;

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
