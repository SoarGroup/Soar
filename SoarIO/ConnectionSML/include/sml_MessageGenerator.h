/////////////////////////////////////////////////////////////////
// MessageGenerator class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class contains helper functions that can be used to create SML messages.
// The user can choose to just roll their own messages without using these methods,
// but they may find them useful.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_MESSAGE_GENERATOR_H
#define SML_MESSAGE_GENERATOR_H

// A null pointer
#ifndef NULL
#define NULL 0
#endif

// The end of a null terminated string
#ifndef NUL
#define NUL 0
#endif

#ifndef unused
#define unused(x) (void)(x)
#endif

#include "sml_Errors.h"

namespace sml
{

// Forward declarations
class ElementXML ;		// NOTE: We are a friend of ElementXML, so can access the "Fast" functions which are tricky to use safely, so generally hidden.

/*************************************************************
* @brief The MessageGenerator class contains helper functions
*		 to make it easier to build up a valid SML message.
*************************************************************/
class MessageGenerator
{
protected:
	// The ID to use for the next message we send
	int				m_MessageID ;

	// The error status of the last function called.
	ErrorCode		m_ErrorCode ;

public:
	MessageGenerator() ;
	virtual ~MessageGenerator() ;

	/*************************************************************
	* @brief Returns the error status from the last function called.
	*		 0 if successful, otherwise an error code to indicate what went wrong.
	*************************************************************/
	ErrorCode GetLastError() { return m_ErrorCode ; }

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
	* @returns 0 if successful, otherwise an error code to indicate what went wrong.
	*************************************************************/
	virtual void AddParameterToSMLCommand(ElementXML* pCommand, char const* pName, char const* pValue, char const* pValueType = NULL) ;

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

protected:
	/*************************************************************
	* @brief Resets the last error value to 0.
	*************************************************************/
	void ClearError()	{ m_ErrorCode = Error::kNoError ; }

	/*************************************************************
	* @brief Set the error code
	*************************************************************/
	void SetError(ErrorCode error)	{ m_ErrorCode = error ; }

};

} // End of namespace

#endif // SML_MESSAGE_GENERATOR_H
