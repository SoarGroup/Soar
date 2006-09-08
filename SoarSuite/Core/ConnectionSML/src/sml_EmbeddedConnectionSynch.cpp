#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

/////////////////////////////////////////////////////////////////
// EmbeddedConnectionSynch class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class represents a logical connection between two entities that are communicating
// via SML (a form of XML).  In the embedded case that this class represents, both entities
// are within the same process.  For the "Synch" variant, the two entities execute in
// the same thread (so we're just dealing with direct function calls).
//
/////////////////////////////////////////////////////////////////

#include "sml_EmbeddedConnectionSynch.h"
#include "sml_ElementXML.h"
#include "sml_MessageSML.h"
#include "thread_Thread.h"
#include "sock_Debug.h"

#include <string>
#include <iostream>
#include <assert.h>

using namespace sml ;

/*************************************************************
* @brief Send the message over to the other side of the connection
*		 and store the response.
*
*		 The caller can pick up the response later with a
*		 call to GetResponseForID().
*************************************************************/
void EmbeddedConnectionSynch::SendMessage(ElementXML* pMsg)
{
	ClearError() ;

	// Check that we have somebody to send this message to.
	assert(m_hConnection);
	if (m_hConnection == NULL)
	{
		SetError(Error::kNoEmbeddedLink) ;
		return ;
	}

#ifdef _DEBUG
	if (IsTracingCommunications())
	{
		char* pStr = pMsg->GenerateXMLString(true) ;
		PrintDebugFormat("%s Sending %s\n", IsKernelSide() ? "Kernel" : "Client", pStr) ;
		pMsg->DeleteString(pStr) ;
	}
#endif

	ElementXML_Handle hResponse = NULL ;

	// Add a reference to this object, which will then be released by the receiver of this message when
	// they are done with it.
	pMsg->AddRefOnHandle() ;
	ElementXML_Handle hSendMsg = pMsg->GetXMLHandle() ;

	// Make the call to the kernel, passing the message over and getting an immediate response since this is
	// an embedded synchronous (in thread) call.
	hResponse = m_pProcessMessageFunction(m_hConnection, hSendMsg, SML_MESSAGE_ACTION_SYNCH) ;

	// We cache the response
	m_pLastResponse->Attach(hResponse) ;
}

/*************************************************************
* @brief Look up the response to a particular message.
*
*		We require these calls to be paired with the order the messages
*		were sent (so send-msg-1, send-msg-2, get-response-2, get-response-1).
*		Think of sending a message as a message call.  You can't return to
*		a function higher up the stack, you need to go in order.
*
*		As a result, we only store a single response at a time
*		and this must match the message we just sent (since its a synchronous
*		connection so it's executed immediately).
*************************************************************/
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

#ifdef _DEBUG
	if (IsTracingCommunications())
	{
		char* pStr = pResult->GenerateXMLString(true) ;
		PrintDebugFormat("%s Received %s\n", IsKernelSide() ? "Kernel" : "Client", pStr) ;
		pResult->DeleteString(pStr) ;
	}
#endif

	return pResult ;
}

