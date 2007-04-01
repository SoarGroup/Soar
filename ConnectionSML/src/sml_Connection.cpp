#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

/////////////////////////////////////////////////////////////////
// Connection class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class represents a logical connection between two entities that are communicating via SML.
// For example, an environment (the client) and the Soar kernel.
//
// The connection can be "embedded" which means both the client and the kernel are in the same process
// or it can be "remote" which means the client and the kernel are in different processes and possibly on different machines.
//
// Commands formatted as SML (a form of XML) are sent over this connection to issue commands etc.
//
// This class is actually an abstract base class, with specific implementations of this class
// being used to provide the different types of connections.
//
/////////////////////////////////////////////////////////////////

#include "sml_Connection.h"
#include "sml_ElementXML.h"
#include "sml_MessageSML.h"
#include "sml_EmbeddedConnection.h"
#include "sml_EmbeddedConnectionAsynch.h"
#include "sml_EmbeddedConnectionSynch.h"
#include "sml_RemoteConnection.h"
#include "sml_AnalyzeXML.h"
#include "sml_TagCommand.h"
#include "sml_TagArg.h"
#include "sml_TagError.h"
#include "sml_TagResult.h"
#include "sock_ClientSocket.h"

#include <time.h>	// For debug random start of message id's

using namespace sml ;

/*************************************************************
* @brief Constructor
*************************************************************/
Connection::Connection()
{
	m_MessageID = 0 ;

#ifdef _DEBUG
	// It's useful to start somewhere other than 0 in debug, especially when dealing with
	// remote connections, so it's clear which id's come from which client.
	int rand = (int)(clock() % 10) ;
	m_MessageID = 100 * rand ;
#endif

	m_InitialTimeTagCounter = 0 ;
	m_pUserData = NULL ;
	m_bIsDirectConnection = false ;
	m_bTraceCommunications = false ;
	m_bIsKernelSide = false ;
}

/*************************************************************
* @brief Destructor
*************************************************************/
Connection::~Connection()
{
	// Delete all callback objects
	CallbackMapIter mapIter = m_CallbackMap.begin() ;

	while (mapIter != m_CallbackMap.end())
	{
		//char const*	pType = mapIter->first ;
		CallbackList*	pList = mapIter->second ;

		// Delete all callbacks in this list
		CallbackListIter iter = pList->begin() ;

		while (iter != pList->end())
		{
			Callback* pCallback = *iter ;
			iter++ ;
			delete pCallback ;
		}

		mapIter++ ;

		// Delete the list itself
		delete pList ;
	}

	// Clear out any messages sitting on the queues
	while (m_IncomingMessageQueue.size() > 0)
	{
		ElementXML* pMsg = m_IncomingMessageQueue.back() ;
		delete pMsg ;

		m_IncomingMessageQueue.pop() ;
	}
}


/*************************************************************
* @brief Creates a connection to a receiver that is embedded
*        within the same process.
*
* @param pLibraryName	The name of the library to load, without an extension (e.g. "SoarKernelSML").  Case-sensitive (to support Linux).
*						This library will be dynamically loaded and connected to.
* @param ClientThread	If true, Soar will run in the client's thread and the client must periodically call over to the
*						kernel to check for incoming messages on remote sockets.
*						If false, Soar will run in a thread within the kernel and that thread will check the incoming sockets itself.
*						However, this kernel thread model requires a context switch whenever commands are sent to/from the kernel.
* @param Optimized		If this is a client thread connection, we can short-circuit parts of the messaging system for sending input and
*						running Soar.  If this flag is true we use those short cuts.  If you're trying to debug the SML libraries
*						you may wish to disable this option (so everything goes through the standard paths).  Has no affect if not running on client thread.
* @param port			The port number the server should use to receive remote connections.  The default port for SML is 12121 (picked at random).
*
* @param pError			Pass in a pointer to an int and receive back an error code if there is a problem.
* @returns An EmbeddedConnection instance.
*************************************************************/
Connection* Connection::CreateEmbeddedConnection(char const* pLibraryName, bool clientThread, bool optimized, int portToListenOn, ErrorCode* pError)
{
	// Set an initial error code and then replace it if something goes wrong.
	if (pError) *pError = Error::kNoError ;

	// We also use the terms "synchronous" and "asynchronous" for client thread or kernel thread.
	EmbeddedConnection* pConnection = clientThread ?
									EmbeddedConnectionSynch::CreateEmbeddedConnectionSynch() :
									EmbeddedConnectionAsynch::CreateEmbeddedConnectionAsynch() ;

	pConnection->AttachConnection(pLibraryName, optimized, portToListenOn) ;

	// Report any errors
	if (pError) *pError = pConnection->GetLastError() ;

	return pConnection ;
}

/*************************************************************
* @brief Creates a connection to a receiver that is in a different
*        process.  The process can be on the same machine or a different machine.
*
* @param sharedFileSystem	If true the local and remote machines can access the same set of files.
*					For example, this means when loading a file of productions, sending the filename is
*					sufficient, without actually sending the contents of the file.
*					(NOTE: It may be a while before we really support passing in 'false' here)
* @param pIPaddress The IP address of the remote machine (e.g. "202.55.12.54").
*                   Pass "127.0.0.1" or NULL to create a connection between two processes on the same machine.
* @param port		The port number to connect to.  The default port for SML is 12121 (picked at random).
* @param pError		Pass in a pointer to an int and receive back an error code if there is a problem.  (Can pass NULL).
*
* @returns A RemoteConnection instance.
*************************************************************/
Connection* Connection::CreateRemoteConnection(bool sharedFileSystem, char const* pIPaddress, unsigned short port, ErrorCode* pError)
{
	sock::ClientSocket* pSocket = new sock::ClientSocket() ;

	bool ok = pSocket->ConnectToServer(pIPaddress, port) ;

	if (!ok && pError)
	{
		// BADBAD: Can we get a more detailed error from pSocket?
		*pError = Error::kConnectionFailed ;
		delete pSocket ;
		return NULL ;
	}

	// Wrap the socket inside a remote connection object
	RemoteConnection* pConnection = new RemoteConnection(sharedFileSystem, pSocket) ;

	return pConnection ;
}

/*************************************************************
* @brief Create a new connection object wrapping a socket.
*		 The socket is generally obtained from a ListenerSocket.
*************************************************************/
Connection* Connection::CreateRemoteConnection(sock::Socket* pSocket)
{
	// This is a server side connection, so it doesn't have any way to know
	// if the client and it share the same file system, so just set it to true by default.
	return new RemoteConnection(true, pSocket) ;
}

/*************************************************************
* @brief Retrieve the response to the last call message sent.
*
*		 In an embedded situation, this result is always immediately available and the "wait" parameter is ignored.
*		 In a remote situation, if wait is false and the result is not immediately available this call returns false.
*
*		 The message is only required when the client is remote (because then there might be many responses waiting on the socket).
*		 A message can only be retrieved once, so a second call with the same ID will return NULL.
*		 Only the response to the last call message can be retrieved.
*
*		 The client is not required to call to get the result of a command it has sent.
*
*		 The implementation of this function will call ReceiveMessages() to get messages one at a time and process them.  Thus callbacks may be
*		 invoked while the client is blocked waiting for the particular response they requested.
*
*		 A response that is returned to the client through GetResultOfMessage() will not be passed to a callback
*		 function registered for response messages.  This allows a client to register a general function to check for
*		 any error messages and yet retrieve specific responses to calls that it is particularly interested in.
*
* @param pMsg	The original SML message that we wish to get a response from.
* @param wait	If true wait until the result is received (or we time out and report an error).
*
* @returns The message that is a response to pMsg or NULL if none is found.
*************************************************************/
ElementXML* Connection::GetResponse(ElementXML const* pXML, bool wait /* == true */)
{
	if (!pXML)
	{
		SetError(Error::kInvalidArgument) ;
		return NULL ;
	}

	char const* pID = pXML->GetAttribute(sml_Names::kID) ;

	if (pID == NULL)
	{
		SetError(Error::kArgumentIsNotSML) ;
		return NULL ;
	}

	return GetResponseForID(pID, wait) ;
}

/*************************************************************
* @brief Register a callback for a particular type of incoming message.
*
*		 Messages are currently one of:
*		 "call", "response" or "notify"
*		 A call is always paired to a response (think of this as a remote function call that returns a value)
*		 while a notify does not receive a response (think of this as a remote function call that does not return a value).
*		 This type is stored in the "doctype" attribute of the top level SML node in the message.
*		 NOTE: doctype's are case sensitive.
*
*		 You MUST register a callback for the "call" type of message.  This callback must return a "response" message which is then
*		 sent back over the connection.  Other callbacks should not return a message.
*		 Once the returned message has been sent it will be deleted.
*
*		 We will maintain a list of callbacks for a given type of SML document and call each in turn.
*		 Each callback on the list will be called in turn until one returns a non-NULL response.  No further callbacks
*		 will be called for that message.  This ensures that only one response is sent to a message.
*
* @param callback	The function to call when an incoming message is received (of the right type)
* @param pUserData	This data is passed to the callback.  It allows the callback to have some context to work in.  Can be NULL.
* @param pType		The type of message to register for (currently one of "call", "response" or "notify").
* @param addToEnd	If true add the callback to the end of the list (called last).  If false, add to front where it will be called first.
*
* @returns 0 if successful, otherwise an error code to indicate what went wrong.
*************************************************************/
void Connection::RegisterCallback(IncomingCallback callback, void* pUserData, char const* pType, bool addToEnd)
{
	ClearError() ;

	if (callback == NULL || pType == NULL)
	{
		SetError(Error::kInvalidArgument) ;
		return ;
	}

	// Create the callback object to be stored in the map
	Callback* pCallback = new Callback(this, callback, pUserData) ;

	CallbackList* pList = NULL ;
//	CallbackList* pList = m_CallbackMap[pType] ;

	// See if we have a list of callbacks for this type yet
	CallbackMapIter iter = m_CallbackMap.find(pType) ;

	if (iter != m_CallbackMap.end())
	{
		// If the list already exists, just grab it
		pList = iter->second ;
	}
	else
	{
		// Need to create the list
		pList = new CallbackList() ;
		m_CallbackMap[pType] = pList ;
	}
	
	// Add the callback to the list
	if (addToEnd)
		pList->push_back(pCallback) ;
	else
		pList->push_front(pCallback) ;
}

/*************************************************************
* @brief Removes a callback from the list of callbacks for a particular type of incoming message.
*
* @param callback	The function that was previously registered.  If NULL removes all callbacks for this type of message.
* @param pType		The type of message to unregister from (currently one of "call", "response" or "notify").
*
* @returns 0 if successful, otherwise an error code to indicate what went wrong.
*************************************************************/
void Connection::UnregisterCallback(IncomingCallback callback, char const* pType)
{
	ClearError() ;

	if (pType == NULL)
	{
		SetError(Error::kInvalidArgument) ;
		return ;
	}

	// See if we have a list of callbacks for this type
	CallbackList* pList = GetCallbackList(pType) ;

	if (pList == NULL)
	{
		SetError(Error::kCallbackNotFound) ;
		return ;
	}

	if (callback == NULL)
	{
		// Caller asked to delete all callbacks for this type
		delete pList ;
		m_CallbackMap[pType] = NULL ;

		return ;
	}

	// Walk the list of callbacks, deleting any objects that
	// match the callback function
	CallbackListIter iter = pList->begin() ;

	bool found = false ;

	while (iter != pList->end())
	{
		Callback* pCallback = *iter ;
		iter++ ;

		// See if this function matches the one we were passed
		// In which case, delete it.
		if (pCallback->getFunction() == callback)
		{
			delete pCallback ;
			found = true ;
		}
	}

	if (!found)
		SetError(Error::kCallbackNotFound) ;
}

/*************************************************************
* @brief Gets the list of callbacks associated with a given doctype (e.g. "call")
**************************************************************/
CallbackList* Connection::GetCallbackList(char const* pType)
{
	// We don't use the simpler m_CallbackMap[pType] because
	// doing this on a map has the nasty side-effect of adding an object to the map.
	// So the preferred method is to instead use find.
	CallbackMapIter iter = m_CallbackMap.find(pType) ;

	if (iter == m_CallbackMap.end())
		return NULL ;
	
	return iter->second ;
}

/*************************************************************
* @brief Invoke the list of callbacks matching the doctype of the incoming message.
*
* @param pIncomingMsg	The SML message that should be passed to the callbacks.
* @returns The response message (or NULL if there is no response from any callback).
*************************************************************/
ElementXML* Connection::InvokeCallbacks(ElementXML *pIncomingMsg)
{
	ClearError() ;

	MessageSML *pIncomingSML = (MessageSML*)pIncomingMsg ;

	// Check that we were passed a valid message.
	if (pIncomingMsg == NULL)
	{
		SetError(Error::kInvalidArgument) ;
		return NULL ;
	}

	// Retrieve the type of this message
	char const* pType = pIncomingSML->GetDocType() ;

	// Check that this message has a valid doc type (all valid SML do)
	if (pType == NULL)
	{
		SetError(Error::kNoDocType) ;
		return NULL ;
	}

	// Decide if this message is a "call" which requires a "response"
	bool isIncomingCall = pIncomingSML->IsCall() ;

	// See if we have a list of callbacks for this type
	CallbackList* pList = GetCallbackList(pType) ;

	// Nobody was interested in this type of message, so we're done.
	if (pList == NULL)
	{
		return NULL ;
	}

	CallbackListIter iter = pList->begin() ;

	// Walk the list of callbacks in turn until we reach
	// the end or one returns a message.
	while (iter != pList->end())
	{
		Callback* pCallback = *iter ;
		iter++ ;

		ElementXML* pResponse = pCallback->Invoke(pIncomingMsg) ;

		if (pResponse != NULL)
		{
			if (isIncomingCall)
				return pResponse ;

			// This callback was not for a call and should not return a result.
			// Delete the result and ignore it.
			pResponse->ReleaseRefOnHandle() ;
			pResponse = NULL ;
		}
	}

	// If this is a call, we must respond
	if (isIncomingCall)
		SetError(Error::kNoResponseToCall) ;

	// Nobody returned a response
	return NULL ;
}

/*************************************************************
* @brief Send a message and get the response.
*
* @param pAnalysis	This will be filled in with the analyzed response
* @param pMsg		The message to send
* @returns			True if got a reply and there were no errors.
*************************************************************/
bool Connection::SendMessageGetResponse(AnalyzeXML* pAnalysis, ElementXML* pMsg)
{
	// If the connection is already closed, don't do anything
	if (IsClosed())
		return false ;

	// Make sure only one thread is sending messages at a time
	// (This allows us to run a separate thread in clients polling for events even
	//  when the client is sleeping, but we don't want them both to be sending/receiving at the same time).
	soar_thread::Lock lock(&m_ClientMutex) ;

	// Send the command over.
	SendMessage(pMsg);

	// There was an error in the send, so we're done.
	if (HadError())
	{
		return false ;
	}

	// Get the response
	ElementXML* pResponse = GetResponse(pMsg) ;

	// These was an error in getting the response
	if (HadError())
		return false ;

	if (!pResponse)
	{
		// We failed to get a reply when one was expected
		SetError(Error::kFailedToGetResponse) ;
		return false ;
	}

	// Analyze the response and return the analysis
	pAnalysis->Analyze(pResponse) ;

#ifdef _DEBUG
	char* pMsgText = pResponse->GenerateXMLString(true) ;
	pResponse->DeleteString(pMsgText) ;
#endif

	delete pResponse ;

	// If the response is not SML, return false
	if (!pAnalysis->IsSML())
	{
		SetError(Error::kResponseIsNotSML) ;
		return false ;
	}

	// If we got an error, return false.
	if (pAnalysis->GetErrorTag())
	{
		SetError(Error::kSMLErrorMessage) ;
		return false ;
	}

	return true ;
}

/*************************************************************
* @brief Build an SML message and send it over the connection
*		 returning the analyzed version of the response.
*
* This family of commands are designed for access based on
* a named agent.  This agent's name is passed as the first
* parameter and then the other parameters define the details
* of which method to call for the agent.
* 
* Passing NULL for the agent name is valid and indicates
* that the command is not agent specific (e.g. "shutdown-kernel"
* would pass NULL).
*
* Uses SendMessageGetResponse() to do its work.
*
* @param pResponse		The response from the kernel to this command.
* @param pCommandName	The command to execute
* @param pAgentName		The name of the agent this command is going to (can be NULL -> implies going to top level of kernel)
* @param pParamName1	The name of the first argument for this command
* @param pParamVal1		The value of the first argument for this command
* @param rawOuput		If true, sends back a simple string form for the result which the caller will probably just print.
*						If false, sendds back a structured XML object that the caller can analyze and do more with.
* @returns	True if command was sent and received back without any errors (either in sending or in executing the command).
*************************************************************/
bool Connection::SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, bool rawOutput)
{
	ElementXML* pMsg = CreateSMLCommand(pCommandName, rawOutput) ;

	bool result = SendMessageGetResponse(pResponse, pMsg) ;

	delete pMsg ;
	
	return result ;
}

/*************************************************************
* @brief Build an SML message and send it over the connection
*		 returning the analyzed version of the response.
* See SendAgentCommand() above.
*************************************************************/
bool Connection::SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName, bool rawOutput)
{
	ElementXML* pMsg = CreateSMLCommand(pCommandName, rawOutput) ;

	//add the agent name parameter
	if (pAgentName)
		AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, pAgentName);

	bool result = SendMessageGetResponse(pResponse, pMsg) ;

	delete pMsg ;
	
	return result ;
}

/*************************************************************
* @brief Build an SML message and send it over the connection
*		 returning the analyzed version of the response.
* See SendAgentCommand() above.
*************************************************************/
bool Connection::SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName,
					char const* pParamName1, char const* pParamVal1,
					bool rawOutput)
{
	ElementXML* pMsg = CreateSMLCommand(pCommandName, rawOutput) ;

	//add the agent name parameter
	if (pAgentName)
		AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, pAgentName);

	// add the other parameters
	AddParameterToSMLCommand(pMsg, pParamName1, pParamVal1);

	bool result = SendMessageGetResponse(pResponse, pMsg) ;

	delete pMsg ;
	
	return result ;
}

/*************************************************************
* @brief Build an SML message and send it over the connection
*		 returning the analyzed version of the response.
* See SendAgentCommand() above.
*************************************************************/
bool Connection::SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName,
					char const* pParamName1, char const* pParamVal1,
					char const* pParamName2, char const* pParamVal2,
					bool rawOutput)
{
	ElementXML* pMsg = CreateSMLCommand(pCommandName, rawOutput) ;

	//add the agent name parameter
	if (pAgentName)
		AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, pAgentName);

	// add the other parameters
	AddParameterToSMLCommand(pMsg, pParamName1, pParamVal1);
	AddParameterToSMLCommand(pMsg, pParamName2, pParamVal2);

	bool result = SendMessageGetResponse(pResponse, pMsg) ;

	delete pMsg ;
	
	return result ;
}

/*************************************************************
* @brief Build an SML message and send it over the connection
*		 returning the analyzed version of the response.
* See SendAgentCommand() above.
*************************************************************/
bool Connection::SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName,
					char const* pParamName1, char const* pParamVal1,
					char const* pParamName2, char const* pParamVal2,
					char const* pParamName3, char const* pParamVal3,
					bool rawOutput)
{
	ElementXML* pMsg = CreateSMLCommand(pCommandName, rawOutput) ;

	//add the agent name parameter
	if (pAgentName)
		AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, pAgentName);

	// add the other parameters
	AddParameterToSMLCommand(pMsg, pParamName1, pParamVal1);
	AddParameterToSMLCommand(pMsg, pParamName2, pParamVal2);
	AddParameterToSMLCommand(pMsg, pParamName3, pParamVal3);

	bool result = SendMessageGetResponse(pResponse, pMsg) ;

	delete pMsg ;
	
	return result ;
}

/*************************************************************
* @brief Build an SML message and send it over the connection
*		 returning the analyzed version of the response.
* See SendMessageGetResponse() for more.
*************************************************************/
bool Connection::SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName)
{
	ElementXML* pMsg = CreateSMLCommand(pCommandName) ;

	bool result = SendMessageGetResponse(pResponse, pMsg) ;

	delete pMsg ;
	
	return result ;
}

/*************************************************************
* @brief Build an SML message and send it over the connection
*		 returning the analyzed version of the response.
*
* @param pCommandName	The command to send
* @param pThisID		The id of the object (e.g. IAgent) whose method we are calling.
* @returns	An analyzed version of the reply
*************************************************************/
bool Connection::SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID)
{
	ElementXML* pMsg = CreateSMLCommand(pCommandName) ;

	//add the 'this' pointer parameter
	AddParameterToSMLCommand(pMsg, sml_Names::kParamThis, pThisID);

	bool result = SendMessageGetResponse(pResponse, pMsg) ;

	delete pMsg ;
	
	return result ;
}

/*************************************************************
* @brief Build an SML message and send it over the connection
*		 returning the analyzed version of the response.
*
* @param pCommandName	The command to send
* @param pThisID		The id of the object (e.g. IAgent) whose method we are calling.
* @param pParamName1	The name of the 1st parameter (after the 'this' param)
* @param pParamVal1		The value of the 1st parameter (after the 'this' param) (can be NULL if an optional param)
* @returns	An analyzed version of the reply
*************************************************************/
bool Connection::SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID,
									char const* pParamName1, char const* pParamVal1)
{
	ElementXML* pMsg = CreateSMLCommand(pCommandName) ;

	//add the 'this' pointer parameter
	AddParameterToSMLCommand(pMsg, sml_Names::kParamThis, pThisID);
	if (pParamVal1) AddParameterToSMLCommand(pMsg, pParamName1, pParamVal1);

	bool result = SendMessageGetResponse(pResponse, pMsg) ;

	delete pMsg ;
	
	return result ;
}

/*************************************************************
* @brief Build an SML message and send it over the connection
*		 returning the analyzed version of the response.
*
* @param pCommandName	The command to send
* @param pThisID		The id of the object (e.g. IAgent) whose method we are calling.
* @param pParamName1	The name of the 1st parameter (after the 'this' param)
* @param pParamVal1		The value of the 1st parameter (after the 'this' param) (can be NULL if optional param)
* @param pParamName2	The name of the 2nd parameter (after the 'this' param)
* @param pParamVal2		The value of the 2nd parameter (after the 'this' param) (can be NULL if optional param)
* @returns	An analyzed version of the reply
*************************************************************/
bool Connection::SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID,
									char const* pParamName1, char const* pParamVal1,
									char const* pParamName2, char const* pParamVal2)
{
	ElementXML* pMsg = CreateSMLCommand(pCommandName) ;

	//add the 'this' pointer parameter
	AddParameterToSMLCommand(pMsg, sml_Names::kParamThis, pThisID);

	// Note: If first param is missing, second must be ommitted too (normal optional param syntax)
	if (pParamVal1)				  AddParameterToSMLCommand(pMsg, pParamName1, pParamVal1);
	if (pParamVal1 && pParamVal2) AddParameterToSMLCommand(pMsg, pParamName2, pParamVal2);

	bool result = SendMessageGetResponse(pResponse, pMsg) ;

	delete pMsg ;
	
	return result ;
}

/*************************************************************
* @brief Build an SML message and send it over the connection
*		 returning the analyzed version of the response.
*
* @param pCommandName	The command to send
* @param pThisID		The id of the object (e.g. IAgent) whose method we are calling.
* @param pParamName1	The name of the 1st parameter (after the 'this' param)
* @param pParamVal1		The value of the 1st parameter (after the 'this' param)
* @param pParamName2	The name of the 2nd parameter (after the 'this' param)
* @param pParamVal2		The value of the 2nd parameter (after the 'this' param)
* @param pParamName3	The name of the 3rd parameter (after the 'this' param)
* @param pParamVal3		The value of the 3rd parameter (after the 'this' param)
* @returns	An analyzed version of the reply
*************************************************************/
bool Connection::SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID,
									char const* pParamName1, char const* pParamVal1,
									char const* pParamName2, char const* pParamVal2,
									char const* pParamName3, char const* pParamVal3)
{
	ElementXML* pMsg = CreateSMLCommand(pCommandName) ;

	//add the 'this' pointer parameter
	AddParameterToSMLCommand(pMsg, sml_Names::kParamThis, pThisID);
	
	if (pParamVal1)								AddParameterToSMLCommand(pMsg, pParamName1, pParamVal1);
	if (pParamVal1 && pParamVal2)				AddParameterToSMLCommand(pMsg, pParamName2, pParamVal2);
	if (pParamVal1 && pParamVal2 && pParamVal3) AddParameterToSMLCommand(pMsg, pParamName3, pParamVal3);

	bool result = SendMessageGetResponse(pResponse, pMsg) ;

	delete pMsg ;
	
	return result ;
}


/*************************************************************
* @brief Create a basic SML message, with the top level <sml> tag defined
*		 together with the version, doctype, soarVersion and id filled in.
*
* Use this call if you plan on building up a message manually and would like
* a little help getting started.
*
* @param pType	The type of message (currently one of "call", "response" or "notify").
*				Think of a call as a remote function call that returns a value (the response).
*				Think of a notify as a remote function call that does not return a value.
*
* @returns The new SML message
*************************************************************/
ElementXML* Connection::CreateSMLMessage(char const* pType)
{
	MessageSML* pMsg = new MessageSML() ;
	pMsg->SetID(GenerateID()) ;
	pMsg->SetDocType(pType) ;

	return pMsg ;
}

/*************************************************************
* @brief Create a basic SML command message.  You should then add parameters to this command.
*		 This function calls CreateSMLMessage() and then adds a <command> tag to it.
*		 E.g. the command might be "excise -production" so the name of the command is "excise".
*		 Then add parameters to this.
*
* @param pName		The name of the command (the meaning depends on whoever receives this command).
* @param rawOutput	If true, results from command will be a string wrapped in a <raw> tag, rather than full structured XML.
* 
* @returns The new SML command message.
*************************************************************/
ElementXML* Connection::CreateSMLCommand(char const* pName, bool rawOutput /* == false */)
{
	// Create a new call message
	MessageSML* pMsg = new MessageSML(MessageSML::kCall, GenerateID()) ;

	// Create the command tag
	TagCommand* pCommand = new TagCommand() ;
	pCommand->SetName(pName) ;

	if (rawOutput)
		pCommand->AddAttributeFastFast(sml_Names::kCommandOutput, sml_Names::kRawOutput) ;

	pMsg->AddChild(pCommand) ;

	return pMsg ;
}

/*************************************************************
* @brief Add a parameter to an SML command message.
*
* The type of the value is optional as presumably the recipient knows how to parse it.
*
* @param pCommand	An existing SML command message.
* @param pName		The name of this parameter (can't be NULL).
* @param pValue		The value of this parameter (represented as a string).  Can be empty, can't be NULL.
* @param pValueType	The type of the value (e.g. "int" or "string".  Anything can go here as long as the recipient understands it) (usually will be NULL).
* 
* @returns Pointer to the ElementXML_Handle for the <command> tag (not the full message, just the <command> part)
*		   This is rarely needed, but could be used to optimize the code.  DO NOT release this handle.
*************************************************************/
ElementXML_Handle Connection::AddParameterToSMLCommand(ElementXML* pMsg, char const* pName, char const* pValue, char const* pValueType)
{
	ClearError() ;

#ifdef DEBUG
	if (!pName || !pValue)
	{
		SetError(Error::kNullArgument) ;
		return NULL ;
	}

	if (!pMsg->IsTag(sml_Names::kTagSML))
	{
		SetError(Error::kArgumentIsNotSML) ;
		return NULL ;
	}
#endif

	// Get the command object
	ElementXML command(NULL) ;
	ElementXML* pCommand = &command ;
	bool found = pMsg->GetChild(pCommand, 0) ;

#ifdef DEBUG
	if (!found || !pCommand->IsTag(sml_Names::kTagCommand))
	{
		SetError(Error::kSMLHasNoCommand) ;
		return NULL ;
	}
#else
	unused(found) ;
#endif

	// Create the arg tag
	TagArg* pArg = new TagArg() ;

	pArg->SetParam(pName) ;
	pArg->SetValue(pValue) ;
	
	if (pValueType)
		pArg->SetType(pValueType) ;

	pCommand->AddChild(pArg) ;

	return pCommand->GetXMLHandle() ;
}

/*************************************************************
* @brief Create a basic SML response message.  You should then add content to this response.
*		 This function calls CreateSMLMessage() and fills in the appropriate "ack" attribute
*		 to respond to the incoming message.
*
* @param pIncomingMsg	The original message that we are responding to.
* 
* @returns The new SML response message.
*************************************************************/
ElementXML* Connection::CreateSMLResponse(ElementXML const* pIncomingMsg)
{
	ClearError() ;

#ifdef DEBUG
	if (!pIncomingMsg)
	{
		SetError(Error::kNullArgument) ;
		return NULL ;
	}

	if (!pIncomingMsg->IsTag(sml_Names::kTagSML))
	{
		SetError(Error::kArgumentIsNotSML) ;
		return NULL ;
	}

	MessageSML* pIncomingSML = (MessageSML*)pIncomingMsg ;

	if (!pIncomingSML->GetID())
	{
		SetError(Error::kArgumentIsNotSML) ;
		return NULL ;
	}
#endif

	// Create a new response message
	MessageSML* pMsg = new MessageSML(MessageSML::kResponse, GenerateID()) ;

	// Messages must have an ID and we use that as the response
	char const* pAck = ((MessageSML*)pIncomingMsg)->GetID() ;

	// Add an "ack=<id>" field to the response so the caller knows which
	// message we are responding to.
	pMsg->AddAttributeFast(sml_Names::kAck, pAck) ;

	return pMsg ;
}

/*************************************************************
* @brief Adds an <error> tag and an error message to a response message.
*
* @param pResponse	The response message we are adding an error to.
* @param pErrorMsg	A description of the error in a form presentable to the user
* @param errorCode	An optional numeric code for the error (to support programmatic responses to the error)
*************************************************************/
void Connection::AddErrorToSMLResponse(ElementXML* pResponse, char const* pErrorMsg, int errorCode /* = -1 */)
{
	ClearError() ;

#ifdef DEBUG
	if (!pResponse || !pErrorMsg)
	{
		SetError(Error::kNullArgument) ;
		return ;
	}

	if (!pResponse->IsTag(sml_Names::kTagSML))
	{
		SetError(Error::kArgumentIsNotSML) ;
		return ;
	}
#endif
	// Create a result tag so if you only
	// check for results you still get an error message.
	// Also add an error tag, so we can distinguish between
	// errors and success based on message structure.
	TagResult* pTag = new TagResult() ;

	pTag->SetCharacterData(pErrorMsg) ;
	pTag->AddAttributeFastFast(sml_Names::kCommandOutput, sml_Names::kRawOutput) ;

	pResponse->AddChild(pTag) ;

	// Create the error tag
	TagError* pError = new TagError() ;

	pError->SetDescription(pErrorMsg) ;

	if (errorCode != -1)
		pError->SetErrorCode(errorCode) ;

	pResponse->AddChild(pError) ;
}

/*************************************************************
* @brief Adds a <result> tag and fills in character data for that result.
*
* @param pResponse	The response message we are adding an error to.
* @param pResult	The result (as a text string)
*************************************************************/
void Connection::AddSimpleResultToSMLResponse(ElementXML* pResponse, char const* pResult)
{
	ClearError() ;

#ifdef DEBUG
	if (!pResponse || !pResult)
	{
		SetError(Error::kNullArgument) ;
		return ;
	}

	if (!pResponse->IsTag(sml_Names::kTagSML))
	{
		SetError(Error::kArgumentIsNotSML) ;
		return ;
	}
#endif
	// Create the result tag
	TagResult* pTag = new TagResult() ;

	pTag->SetCharacterData(pResult) ;
	pTag->AddAttributeFastFast(sml_Names::kCommandOutput, sml_Names::kRawOutput) ;

	pResponse->AddChild(pTag) ;
}

/*************************************************************
* @brief Removes the top message from the incoming message queue
*		 in a thread safe way.
*		 Returns NULL if there is no waiting message.
*************************************************************/
ElementXML* Connection::PopIncomingMessageQueue()
{
	// Ensure only one thread is changing the message queue at a time
	// This lock is released when we exit this function.
	soar_thread::Lock lock(&m_IncomingMutex) ;

	if (m_IncomingMessageQueue.size() == 0)
		return NULL ;

	// 	Read the first message that's waiting
	ElementXML* pIncomingMsg = m_IncomingMessageQueue.front() ;
	m_IncomingMessageQueue.pop() ;

	return pIncomingMsg ;
}


