/////////////////////////////////////////////////////////////////
// KernelSMLInterface file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This file provides a C level interface into the KernelSML library and
// implements "EmbeddedSMLInterface.h" (so there is no KernelSMLInteface.h file).
//
// KernelSML receives commands in SML (a dialect of XML), sends them to the Soar kernel
// and then returns the results in SML.
//
// The SML can be passed directly as an object into this library (if the client and kernel happen
// to be in the same process) or the SML can be sent as a regular XML stream.
//
/////////////////////////////////////////////////////////////////

#include "EmbeddedSMLInterface.h"
#include "sml_Connection.h"
#include "sml_EmbeddedConnection.h"
#include "sml_ElementXML.h"
#include "sml_Names.h"
#include "sml_KernelSML.h"

using namespace sml ;

static ElementXML* ReceivedCall(Connection* pConnection, ElementXML* pIncoming, void* pUserData)
{
	unused(pUserData) ;

	// This must be initialized when the connection was created.
	KernelSML* pKernel = (KernelSML*)pConnection->GetUserData() ;

	return pKernel->ProcessIncomingSML(pConnection, pIncoming) ;
}

static EmbeddedConnection* GetConnectionFromHandle(Connection_Receiver_Handle hConnection)
{
	return (EmbeddedConnection*)hConnection ;
}

EXPORT Connection_Receiver_Handle sml_CreateEmbeddedConnection(Connection_Sender_Handle hSenderConnection, ProcessMessageFunction pProcessMessage)
{
	// Create a connection object which we'll use to talk back to this sender
	EmbeddedConnection* pConnection = EmbeddedConnection::CreateEmbeddedConnection() ;

	// Record our kernel object with this connection.  I think we only want one kernel
	// object even if there are many connections (because there's only one kernel) so for now
	// that's how things are set up.
	pConnection->SetUserData(KernelSML::GetKernelSML()) ;

	// Register for "calls" from the client.
	pConnection->RegisterCallback(ReceivedCall, NULL, sml_Names::kDocType_Call, true) ;

	// The original sender is a receiver to us so we need to reverse the type.
	pConnection->AttachConnection((Connection_Receiver_Handle)hSenderConnection, pProcessMessage) ;

	return (Connection_Receiver_Handle)pConnection ;
}

EXPORT ElementXML_Handle sml_ProcessMessage(Connection_Receiver_Handle hReceiverConnection, ElementXML_Handle hIncomingMsg, int action)
{
	EmbeddedConnection* pConnection = GetConnectionFromHandle(hReceiverConnection) ;

	if (action == MESSAGE_ACTION_CLOSE)
	{
		if (pConnection)
		{
			delete pConnection ;
		}

		return NULL ;
	}

	if (action == MESSAGE_ACTION_NORMAL)
	{
		// Create an object to wrap this message.
		// When this object is deleted, it releases our reference to this handle.
		ElementXML incomingMsg(hIncomingMsg) ;

		ElementXML* pResponse = pConnection->InvokeCallbacks(&incomingMsg) ;

		if (!pResponse)
			return NULL ;

		ElementXML_Handle hResponse = pResponse->Detach() ;
		delete pResponse ;
		return hResponse ;
	}

	// Not an action we understand, so just ignore it.
	return NULL ;
}
