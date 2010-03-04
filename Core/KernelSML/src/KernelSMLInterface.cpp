#include <portability.h>

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

#include "sml_Utils.h"
#include "sml_Connection.h"
#include "sml_EmbeddedConnection.h"
#include "sml_EmbeddedConnectionAsynch.h"
#include "sml_EmbeddedConnectionSynch.h"
#include "ElementXML.h"
#include "sml_Names.h"
#include "sml_KernelSML.h"

using namespace sml ;

soarxml::ElementXML* ReceivedCall(Connection* pConnection, soarxml::ElementXML* pIncoming, void* /*pUserData*/)
{
	// This must be initialized when the connection was created.
	KernelSML* pKernel = static_cast<KernelSML*>(pConnection->GetUserData()) ;

	return pKernel->ProcessIncomingSML(pConnection, pIncoming) ;
}

static EmbeddedConnection* GetConnectionFromHandle(Connection_Receiver_Handle hConnection)
{
	return reinterpret_cast<EmbeddedConnection*>(hConnection) ;
}

EXPORT Connection_Receiver_Handle sml_CreateEmbeddedConnection(Connection_Sender_Handle hSenderConnection, ProcessMessageFunction pProcessMessage, int connectionType, int portToListenOn)
{
	bool synch = (connectionType == SML_SYNCH_CONNECTION) ;

	// Create a connection object which we'll use to talk back to this sender
	EmbeddedConnection* pConnection = synch ?
						EmbeddedConnectionSynch::CreateEmbeddedConnectionSynch() :
						EmbeddedConnectionAsynch::CreateEmbeddedConnectionAsynch() ;

	// For debugging, record that this connection object is from kernel to client.
	// The client will also have a Connection object which will not have this flag set.
	pConnection->SetIsKernelSide(true) ;

	// Record our kernel object with this connection.
	KernelSML* pKernelSML = KernelSML::CreateKernelSML(portToListenOn) ;
	pConnection->SetUserData(pKernelSML) ;

	// If this is a synchronous connection then commands will execute on the embedded client's thread
	// and we don't use the receiver thread.  (Why not?  If we allowed it to run then we'd have to (a)
	// sychronize execution between the two threads and (b) sometimes Soar would be running in the client's thread and
	// sometimes in the receiver's thread (depending on where "run" came from) and that could easily introduce a lot of
	// complicated bugs or where performance would be different depending on whether you pressed "run" in the environment or "run" in a
	// remote debugger).
	if (!pConnection->IsAsynchronous())
		pKernelSML->StopReceiverThread() ;

	// Register for "calls" from the client.
	pConnection->RegisterCallback(ReceivedCall, NULL, sml_Names::kDocType_Call, true) ;

	// The original sender is a receiver to us so we need to reverse the type.
	pConnection->AttachConnectionInternal(reinterpret_cast<Connection_Receiver_Handle>(hSenderConnection), pProcessMessage) ;

	// Record this as one of the active connections
	// Must only do this after the pConnection object has been fully initialized
	// as the receiver thread may access it once it has been added to this list.
	pKernelSML->AddConnection(pConnection) ;

	return reinterpret_cast<Connection_Receiver_Handle>(pConnection) ;
}

EXPORT ElementXML_Handle sml_ProcessMessage(Connection_Receiver_Handle hReceiverConnection, ElementXML_Handle hIncomingMsg, int action)
{
	EmbeddedConnection* pConnection = GetConnectionFromHandle(hReceiverConnection) ;

	if (action == SML_MESSAGE_ACTION_CLOSE)
	{
		if (pConnection)
		{
			// Close our connection to the remote process
			pConnection->ClearConnectionHandle() ;

			// When the embedded connection disconnects we're about to exit the application
			// so shutdown any remote connections cleanly and do any other cleanup.
			KernelSML* pKernelSML = reinterpret_cast<KernelSML*>(pConnection->GetUserData());
			assert(pKernelSML);
			pKernelSML->Shutdown() ;

			// The shutdown call above will also delete our connection object as part of its cleanup
			// so set it to NULL here to make sure we don't try to use it again.
			pConnection = NULL ;
		}

		return NULL ;
	}

	if (action == SML_MESSAGE_ACTION_SYNCH)
	{
		// Create an object to wrap this message.
		// When this object is deleted, it releases our reference to this handle.
		soarxml::ElementXML incomingMsg(hIncomingMsg) ;

		soarxml::ElementXML* pResponse = pConnection->InvokeCallbacks(&incomingMsg) ;

		if (!pResponse)
			return NULL ;

		ElementXML_Handle hResponse = pResponse->Detach() ;
		delete pResponse ;
		return hResponse ;
	}

	if (action == SML_MESSAGE_ACTION_ASYNCH)
	{
		// Store the incoming message on a queue and execute it on the receiver's thread (our thread) at a later point.
		EmbeddedConnectionAsynch* eca = static_cast<EmbeddedConnectionAsynch*>(pConnection);
		eca->AddToIncomingMessageQueue(hIncomingMsg) ;

		// There is no immediate response to an asynch message.
		// The response will be sent back to the caller as another asynch message later, once the command has been executed.
		return NULL ;
	}

	if (action == SML_MESSAGE_ACTION_TRACE_ON || action == SML_MESSAGE_ACTION_TRACE_OFF)
	{
		// Report more details on the messages being sent and received.
		// Currently this only affects remote connections but we may extend it to all connections.
		KernelSML* pKernelSML = reinterpret_cast<KernelSML*>(pConnection->GetUserData());
		assert(pKernelSML);
		pKernelSML->SetTraceCommunications( (action == SML_MESSAGE_ACTION_TRACE_ON) ) ;
	}

	// Not an action we understand, so just ignore it.
	return NULL ;
}
