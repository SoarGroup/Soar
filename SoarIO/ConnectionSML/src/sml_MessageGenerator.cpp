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

#include "sml_MessageGenerator.h"
#include "sml_ElementXML.h"
#include "sml_MessageSML.h"
#include "sml_TagCommand.h"
#include "sml_TagArg.h"
#include "sml_TagError.h"
#include "sml_TagResult.h"

using namespace sml ;

/*************************************************************
* @brief Constructor
*************************************************************/
MessageGenerator::MessageGenerator()
{
	m_MessageID = 0 ;
}

/*************************************************************
* @brief Destructor
*************************************************************/
MessageGenerator::~MessageGenerator()
{
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
ElementXML* MessageGenerator::CreateSMLMessage(char const* pType)
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
ElementXML* MessageGenerator::CreateSMLCommand(char const* pName, bool rawOutput /* == false */)
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
ElementXML_Handle MessageGenerator::AddParameterToSMLCommand(ElementXML* pMsg, char const* pName, char const* pValue, char const* pValueType)
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
ElementXML* MessageGenerator::CreateSMLResponse(ElementXML const* pIncomingMsg)
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
void MessageGenerator::AddErrorToSMLResponse(ElementXML* pResponse, char const* pErrorMsg, int errorCode /* = -1 */)
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
	// Create the arg tag
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
void MessageGenerator::AddSimpleResultToSMLResponse(ElementXML* pResponse, char const* pResult)
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

	pResponse->AddChild(pTag) ;
}

