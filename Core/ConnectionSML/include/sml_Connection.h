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
/////////////////////////////////////////////////////////////////

#ifndef SML_CONNECTION_H
#define SML_CONNECTION_H

// A null pointer
#ifndef NULL
#define NULL 0
#endif

// The end of a null terminated string
#ifndef NUL
#define NUL 0
#endif

#include <string>
#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <list>
#include <map>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif

#include <queue>
#include <list>

#ifndef unused
#define unused(x) (void)(x)
#endif

#include "sml_Errors.h"
#include "thread_Lock.h"

// These last ones are just for convenience, they could come out
#include "sml_ElementXML.h"
#include "sml_AnalyzeXML.h"
#include "sml_Names.h"

#include <windows.h>

namespace sock
{
	// Forward declarations
	class DataSender ;
}

namespace sml
{

// Forward declarations
class ConnectionManager ;
class Connection ;
class ElementXML ;
class AnalyzeXML ;

// Define the ListenerCallback to pass the new connection and some user data.
typedef void (*ListenerCallback)(Connection*, void*);

// Define the IncomingCallback to pass the existing connection, incoming message
// and some user data.  The result can be NULL or a message that is sent back over the connection.
typedef ElementXML* (*IncomingCallback)(Connection*, ElementXML*, void*) ;

/*************************************************************
* @brief The Callback class is a simple wrapper around a callback.
*        We use it to keep together the callback function pointer and the
*        user data we'll pass to the function.
*************************************************************/
class Callback
{
protected:
	Connection*			m_pConnection ;
	IncomingCallback	m_pCallback ;
	void*				m_pUserData ;

public:
	/*************************************************************
	* @brief Accessors
	*************************************************************/
	IncomingCallback getFunction() { return m_pCallback ; }

	/*************************************************************
	* @brief Constructor
	*************************************************************/
	Callback(Connection* pConnection, IncomingCallback pFunc, void* pUserData)
	{
		m_pConnection	= pConnection ;
		m_pCallback		= pFunc ;
		m_pUserData		= pUserData ; 
	}
	
	/*************************************************************
	* @brief Invoke this callback, passing the message,
	*		 the connection and the user's data (which can be anything).
	* 
	* @returns NULL or a response to this message.
	*************************************************************/
	ElementXML* Invoke(ElementXML* pIncomingMessage)
	{
		ElementXML* pResult = (*m_pCallback)(m_pConnection, pIncomingMessage, m_pUserData) ;
		return pResult ;
	}
} ;

// Used to store a list of callback objects
typedef std::list< Callback* >	CallbackList ;
typedef CallbackList::iterator	CallbackListIter ;

// Used to store a map from type (a string) to list of callbacks
typedef std::map< std::string, CallbackList* >	CallbackMap ;
typedef CallbackMap::iterator					CallbackMapIter ;

// Used to store a queue of messages
typedef std::queue< ElementXML* >	MessageQueue ;

typedef std::list< ElementXML* >	MessageList ;
typedef MessageList::iterator		MessageListIter ;

/*************************************************************
* @brief The Connection class represents a logical link
*		 between two entities that are communicating through
*		 SML messages.
*************************************************************/
class Connection 
{
public:
	enum { kDefaultSMLPort = 12121 } ;

protected:
	// Maps from SML document types (e.g. "call") to a list of functions to call when that type of message is received.
	CallbackMap		m_CallbackMap ;

	// The client or kernel may wish to keep state information (e.g. a pointer into gSKI for the kernel) with the
	// connection.  If so, it can store it here.  The caller retains ownership of this object, so it won't be
	// deleted when the connection is deleted.
	void*			m_pUserData ;

	// The ID to use for the next message we send
	int				m_MessageID ;

	// The error status of the last function called.
	ErrorCode		m_ErrorCode ;

	// A list of messages that have been received on this connection and are waiting to be executed.
	// This queue may not be in use for a given type of connection
	MessageQueue	m_IncomingMessageQueue ;

	// We use this mutex to serialize acccess to the incoming message queue (for certain types of connections)
	soar_thread::Mutex	m_IncomingMutex ;

	// True if we can make direct calls to gSKI to optimize I/O
	bool m_bIsDirectConnection ;

	// True if we want to dump debug info about messages sent and received.
	bool m_bTraceCommunications ;

	// True if this connection object is on the kernel side of the conversation
	// This has no effect on the logic, but can be helpful for debugging
	bool m_bIsKernelSide ;

	// A client needs to get this mutex before sending and receiving messages.
	// This allows us to use a separate thread in the client to keep connections
	// alive even when the client itself goes to sleep.
	soar_thread::Mutex	m_ClientMutex ;

	// This information can be requested and set by clients, so one client can
	// find out who else is connected.
	std::string m_ID ;			// Unique ID, machine generated (by kernel)
	std::string m_Name ;		// Name, optionally set by client (e.g. debugger)
	std::string m_Status ;		// Status, optionally set by client from fixed set of values
	std::string m_AgentStatus ;	// Agent status, referring to last created agent.  Similar to connection status above.

	// The value to use for this connection's client side time tags (so each connection can have its own part of the id space)
	long		m_InitialTimeTagCounter ;

public:
	Connection() ;
	virtual ~Connection() ;

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
	static Connection* CreateEmbeddedConnection(char const* pLibraryName, bool clientThread, bool optimized, int portToListenOn = kDefaultSMLPort, ErrorCode* pError = NULL) ;

	/*************************************************************
	* @brief Creates a connection to a receiver that is in a different
	*        process.  The process can be on the same machine or a different machine.
	*
	* @param sharedFileSystem	If true the local and remote machines can access the same set of files.
	*					For example, this means when loading a file of productions, sending the filename is
	*					sufficient, without actually sending the contents of the file.
	*					(NOTE: It may be a while before we really support passing in 'false' here)
	* @param pIPaddress The IP address of the remote machine (e.g. "202.55.12.54").
	*                   Pass "127.0.0.1" to create a connection between two processes on the same machine.
	* @param port		The port number to connect to.  The default port for SML is 12121 (picked at random).
	* @param pError		Pass in a pointer to an int and receive back an error code if there is a problem.  (Can pass NULL).
	*
	* @returns A RemoteConnection instance.
	*************************************************************/
	static Connection* CreateRemoteConnection(bool sharedFileSystem, char const* pIPaddress, unsigned short port = kDefaultSMLPort, ErrorCode* pError = NULL) ;

	/*************************************************************
	* @brief Create a new connection object wrapping a socket.
	*		 The socket is generally obtained from a ListenerSocket.
	*		 (Clients don't generally use this method--use the one above instead)
	*************************************************************/
	static Connection* CreateRemoteConnection(sock::DataSender* pDataSender) ;

public:
	/*************************************************************
	* @brief Shuts down this connection.
	*************************************************************/
	virtual void CloseConnection() = 0 ;

	/*************************************************************
	* @brief Returns true if this connection has been closed or
	*		 is otherwise not usable.
	*************************************************************/
	virtual bool IsClosed() = 0 ;

	/*************************************************************
	* @brief Returns true if this is a remote connection (i.e. over a socket,
	*		 may in fact be on the same machine).
	*************************************************************/
	virtual bool IsRemoteConnection() = 0 ;

	/*************************************************************
	* @brief Returns true if messages are queued and executed on receiver's thread.
	*		 (Always true for a remote connection.  May be true or false for
	*		 an embedded connection, depending on how it was created).
	*************************************************************/
	virtual bool IsAsynchronous() = 0 ;

	/*************************************************************
	* @brief Returns true if direct access to gSKI is available.
	*		 This allows us to optimize I/O calls by calling directly
	*		 to gSKI (and hence the kernel) without using the messaging system at all.
	*		 The direct connection is only true if this is a synchronous embedded connection.
	*************************************************************/
	virtual bool IsDirectConnection() { return m_bIsDirectConnection ; }

	/*************************************************************
	* @brief Print out debug information about the messages we are sending and receiving.
	*		 Currently only affects remote connections, but we may extend things.
	*************************************************************/
	virtual void		SetTraceCommunications(bool state)	{ m_bTraceCommunications = state ; }
	virtual bool		IsTracingCommunications()			{ return m_bTraceCommunications ; }

	/*************************************************************
	* @brief True if this connection is from the kernel to the client (false if other way, from client to kernel).
	*        This has no impact on the logic but can help with debugging.
	*************************************************************/
	virtual void		SetIsKernelSide(bool state)	{ m_bIsKernelSide = state ; }
	virtual bool		IsKernelSide()				{ return m_bIsKernelSide ; }

	/*************************************************************
	* @brief Send a message to the SML receiver (e.g. from the environment to the Soar kernel).
	*        The error code that is returned indicates whether the command was successfully sent,
	*		 not whether the command was interpreted successfully by Soar.
	*
	* @param pMsg	The message (as an object representing XML) that is to be sent.
	*				The caller should release this message object after making the send call
	*               once it if finished using it.
	*************************************************************/
	virtual void SendMessage(ElementXML* pMsg) = 0 ;

	/*************************************************************
	* @brief Retrieve any commands, notifications, responses etc. that are waiting.
	*		 Messages that are received are routed to callback functions in the client for processing.
	*
	*		 This call never blocks.  In an embedded situation, this does nothing as incoming messages are
	*        sent directly to the callback functions.
	*        In a remote situation, the client must call this function periodically, to read incoming messages
	*		 on the socket.
	*
	*        We use a callback model (rather than retrieving each message in turn here) so that the embedded model and
	*		 the remote model are closer to each other.
	*
	* @param allMessages	If false, only retrieves at most one message before returning, otherwise gets all messages.
	* @return	True if read at least one message.
	*************************************************************/
	virtual bool ReceiveMessages(bool allMessages) = 0 ;

	/*************************************************************
	* @brief Retrieve the response to the last call message sent.
	*
	*		 In an embedded situation, this result is always immediately available and the "wait" parameter is ignored.
	*		 In a remote situation, if wait is false and the result is not immediately available this call returns false.
	*
	*		 The ID is only required when the client is remote (because then there might be many responses waiting on the socket).
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
	* @param pID	The id of the original SML message (the id is a attribute in the top level [sml] tag)
	* @param wait	If true wait until the result is received (or we time out and report an error).
	*
	* @returns The message that is a response to pID or NULL if none is found.
	*************************************************************/
	virtual ElementXML* GetResponseForID(char const* pID, bool wait) = 0 ;

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
	virtual ElementXML* GetResponse(ElementXML const* pMsg, bool wait = true) ;

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
	*************************************************************/
	virtual void RegisterCallback(IncomingCallback callback, void* pUserData, char const* pType, bool addToEnd) ;

	/*************************************************************
	* @brief Removes a callback from the list of callbacks for a particular type of incoming message.
	*
	* @param callback	The function that was previously registered.  If NULL removes all callbacks for this type of message.
	* @param pType		The type of message to unregister from (currently one of "call", "response" or "notify").
	*************************************************************/
	virtual void UnregisterCallback(IncomingCallback callback, char const* pType) ;

	/*************************************************************
	* @brief Invoke the list of callbacks matching the doctype of the incoming message.
	*
	* @param pIncomingMsg	The SML message that should be passed to the callbacks.
	*
	* @returns The response message (or NULL if there is no response from any calback).
	*************************************************************/
	virtual ElementXML* InvokeCallbacks(ElementXML *pIncomingMsg) ;

	/*************************************************************
	* @brief Get and set the user data.
	*
	* The client or kernel may wish to keep state information (e.g. a pointer into gSKI for the kernel) with the
	* connection.  If so, it can store it here.  The caller retains ownership of this object, so it won't be
	* deleted when the connection is deleted.
	*************************************************************/
	void	SetUserData(void* pUserData)	{ m_pUserData = pUserData ; }
	void*	GetUserData()					{ return m_pUserData ; }

	/*************************************************************
	* @brief Get and set id, name and status
	* ID - unique machine generated id (created by kernel)
	* Name   - optional, set by client.  Should always be the same for a given client (e.g. debugger/java-toh etc.)
	* Status - optional, set by client from fixed list of values
	* Agent status - optional, set by client and refers to last created agent (set to "created" initially by kernel).
	*************************************************************/
	char const* GetID()					{ return m_ID.c_str() ; }
	void SetID(char const* pID)			{ m_ID = pID ; }
	char const* GetName()				{ return m_Name.c_str() ; }
	void SetName(char const* pName)		{ m_Name = pName ; }
	char const* GetStatus()				{ return m_Status.c_str() ; }
	void SetStatus(char const* pStatus) { m_Status = pStatus ; }
	char const* GetAgentStatus()		{ return m_AgentStatus.c_str() ; }
	void SetAgentStatus(char const* pStatus) { m_AgentStatus = pStatus ; }

	/*************************************************************
	* @brief Send a message and get the response.
	*
	* @param pAnalysis	This will be filled in with the analyzed response
	* @param pMsg		The message to send
	* @returns			True if got a reply
	*************************************************************/
	bool SendMessageGetResponse(AnalyzeXML* pAnalysis, ElementXML* pMsg) ;

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
	bool SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, bool rawOutput = false) ;

	bool SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName, bool rawOutput = false) ;

	bool SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName,
					 char const* pParamName1, char const* pParamVal1, bool rawOutput = false) ;

	bool SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName,
					 char const* pParamName1, char const* pParamVal1,
					 char const* pParamName2, char const* pParamVal2, bool rawOutput = false) ;

	bool SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName,
					 char const* pParamName1, char const* pParamVal1,
					 char const* pParamName2, char const* pParamVal2,
					 char const* pParamName3, char const* pParamVal3, bool rawOutput = false) ;

	/*************************************************************
	* @brief Build an SML message and send it over the connection
	*		 returning the analyzed version of the response.
	*
	* This family of commands are designed for an object model access
	* to the kernel (e.g. using the gSKI interfaces).
	* In this model, the first parameter is always an indentifier
	* representing the "this" pointer.  The name of the command gives
	* the method name (in some manner) and the other parameters
	* define the arguments to the method.
	*
	* As of this writing, we are largely moving away from this model,
	* but the code is still here in case it has value in the future.
	*
	* Uses SendMessageGetResponse() to do its work.
	*************************************************************/
	bool SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName) ;

	bool SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID) ;

	bool SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID,
					 char const* pParamName1, char const* pParamVal1) ;

	bool SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID,
					 char const* pParamName1, char const* pParamVal1,
					 char const* pParamName2, char const* pParamVal2) ;

	bool SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID,
					 char const* pParamName1, char const* pParamVal1,
					 char const* pParamName2, char const* pParamVal2,
					 char const* pParamName3, char const* pParamVal3) ;

	/*************************************************************
	* @brief Returns the error status from the last function called.
	*		 0 if successful, otherwise an error code to indicate what went wrong.
	*************************************************************/
	ErrorCode GetLastError() { return m_ErrorCode ; }

	bool	  HadError()	 { return m_ErrorCode != Error::kNoError ; } 

	/*************************************************************
	* @brief Creates a new ID that's unique for this generator.
	*
	* @returns The new ID.
	*************************************************************/
	int GenerateID() { return m_MessageID++ ; }

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
	virtual ElementXML* CreateSMLMessage(char const* pType) ;

	/*************************************************************
	* @brief Create a basic SML command message.  You should then add parameters to this command.
	*		 This function calls CreateSMLMessage() and then adds a <command> tag to it.
	*		 E.g. the command might be "excise -production" so the name of the command is "excise".
	*		 Then add parameters to this.
	*
	* @param pName	The name of the command (the meaning depends on whoever receives this command).
	* @param rawOutput	If true, results from command will be a string wrapped in a <raw> tag, rather than full structured XML. (Defaults to false).
	* 
	* @returns The new SML command message.
	*************************************************************/
	virtual ElementXML* CreateSMLCommand(char const* pCommandName, bool rawOutput = false) ;

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
	virtual ElementXML_Handle AddParameterToSMLCommand(ElementXML* pCommand, char const* pName, char const* pValue, char const* pValueType = NULL) ;

	/*************************************************************
	* @brief Create a basic SML response message.  You should then add content to this response.
	*		 This function calls CreateSMLMessage() and fills in the appropriate "ack" attribute
	*		 to respond to the incoming message.
	*
	* @param pIncomingMsg	The original message that we are responding to.
	* 
	* @returns The new SML response message.
	*************************************************************/
	virtual ElementXML* CreateSMLResponse(ElementXML const* pIncomingMsg) ;

	/*************************************************************
	* @brief Adds an <error> tag and an error message to a response message.
	*
	* @param pResponse	The response message we are adding an error to.
	* @param pErrorMsg	A description of the error in a form presentable to the user
	* @param errorCode	An optional numeric code for the error (to support programmatic responses to the error)
	*************************************************************/
	virtual void AddErrorToSMLResponse(ElementXML* pResponse, char const* pErrorMsg, int errorCode = -1) ;

	/*************************************************************
	* @brief Adds a <result> tag and fills in character data for that result.
	*
	* @param pResponse	The response message we are adding an error to.
	* @param pResult	The result (as a text string)
	*************************************************************/
	virtual void AddSimpleResultToSMLResponse(ElementXML* pResponse, char const* pResult) ;

	void SetInitialTimeTagCounter(long value)	{ m_InitialTimeTagCounter = value ; }
	long GetInitialTimeTagCounter()				{ return m_InitialTimeTagCounter ; } 

protected:
	/*************************************************************
	* @brief Resets the last error value to 0.
	*************************************************************/
	void ClearError()	{ m_ErrorCode = Error::kNoError ; }

	/*************************************************************
	* @brief Set the error code
	*************************************************************/
	void SetError(ErrorCode error)	{ m_ErrorCode = error ; }

	/*************************************************************
	* @brief Gets the list of callbacks associated with a given doctype (e.g. "call")
	**************************************************************/
	virtual CallbackList* GetCallbackList(char const* pType) ;

	/*************************************************************
	* @brief Removes the top message from the incoming message queue
	*		 in a thread safe way.
	*		 Returns NULL if there is no waiting message.
	*************************************************************/
	ElementXML* PopIncomingMessageQueue() ;

};

} // End of namespace

#endif // SML_CONNECTION_H
