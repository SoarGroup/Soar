/////////////////////////////////////////////////////////////////
// KernelSML handlers file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// These are the command handler methods for KernelSML.
// Just moved to a separate implementation file to
// keep the code more manageable.
//
/////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <crtdbg.h>
#endif

#include "sml_KernelSML.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "sml_OutputListener.h"

#include "gSKI.h"
#include <iostream>
#include <fstream>
#include <map>
#include <stdlib.h>

#include "IgSKI_KernelFactory.h"
#include "gSKI_Stub.h"
#include "IgSKI_Kernel.h"

// BADBAD: I think we should be using an error class instead to work with error objects.
#include "../../gSKI/src/gSKI_Error.h"
#include "gSKI_ErrorIds.h"
#include "gSKI_Enumerations.h"
#include "gSKI_Events.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_OutputProcessor.h"
#include "IgSKI_OutputLink.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_WME.h"

#ifdef USE_TCL_DEBUGGER
//debugger #includes
//BADBAD: These should come out when we pull out the TgD debugger hack.
#include "TgD.h"
#include "tcl.h"

//TgDI directives
#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>
#define TGD_SLEEP Sleep
#else
#include <unistd.h>
#define TGD_SLEEP usleep
#endif

#endif // USE_TCL_DEBUGGER

using namespace sml ;
using namespace gSKI ;

// BADBAD: Not sure where this macro is coming from but I saw this
// in IgSKI_Symbol.h and it's needed for the GetObject() calls to compile.
#ifdef _WIN32
#undef GetObject
#undef SendMessage
#endif

static void DebugPrint(char const* pFilename, int line, char const* pMsg)
{
#ifdef _WIN32
#ifdef _DEBUG
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	_CrtDbgReport(_CRT_WARN, pFilename, line, "KernelSML", pMsg);
#endif
#endif
}

void KernelSML::BuildCommandMap()
{
	m_CommandMap[sml_Names::kCommand_CreateAgent]		= KernelSML::HandleCreateAgent ;
	m_CommandMap[sml_Names::kCommand_LoadProductions]	= KernelSML::HandleLoadProductions ;
	m_CommandMap[sml_Names::kCommand_GetInputLink]		= KernelSML::HandleGetInputLink ;
	m_CommandMap[sml_Names::kCommand_Input]				= KernelSML::HandleInput ;

	m_CommandMap[sml_Names::kCommand_CommandLine]		= KernelSML::HandleCommandLine ;
	m_CommandMap[sml_Names::kCommand_CheckForIncomingCommands] = KernelSML::HandleCheckForIncomingCommands ;
}

/*************************************************************
* @brief	A command handler (SML message->appropriate gSKI handling).
*
* @param pAgent			The agent this command is for (can be NULL if the command is not agent specific)
* @param pCommandName	The SML command name (so one handler can handle many incoming calls if we wish)
* @param pConnection	The connection this command came in on
* @param pIncoming		The incoming, analyzed message.
* @param pResponse		The partially formed response.  This handler needs to fill in more of this.
* @param pError			A gSKI error object, which gSKI will fill in if there are errors.
* @returns False if we had an error and wish to generate a generic error message (based on the incoming call + pError)
*          True if the call succeeded or we generated another more specific error already.
*************************************************************/
bool KernelSML::HandleCreateAgent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pAgent) ; unused(pResponse) ;

	// Get the parameters
	char const* pName = pIncoming->GetArgValue(sml_Names::kParamName) ;

	if (!pName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Agent name missing") ;
	}

	// Make the call.
	IAgent* pResult = GetKernel()->GetAgentManager()->AddAgent(pName, NULL, false, gSKI_O_SUPPORT_MODE_4, pError) ;

	// Register for output from this agent
	if (pResult)
	{
		// Create a listener for the callback
		OutputListener* pListener = new OutputListener(this, pConnection) ;
		m_OutputListeners.push_back(pListener) ;

		// Listen for output callback events
		pResult->GetOutputLink()->GetOutputMemory()->AddWorkingMemoryListener(gSKIEVENT_OUTPUT_PHASE_CALLBACK, pListener, pError) ;
	}

#ifdef USE_TCL_DEBUGGER
	// BADBAD: Now create the Tcl debugger.  This is a complete hack and can come out once we have some way to debug this stuff.
	if (m_Debugger == NULL)
	{
		m_Debugger = CreateTgD(pResult, GetKernel(), GetKernelFactory()->GetKernelVersion(), TgD::TSI40, "") ;
		m_Debugger->Init() ;
	}	
#endif

	// Return true if we got an agent constructed.
	// If not, pError should contain the error and returning false
	// means we'll pass that back to the caller.
	return (pResult != NULL) ;
}

// Gives some cycles to the Tcl debugger
bool KernelSML::HandleCheckForIncomingCommands(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pAgent) ; unused(pCommandName) ; unused(pError) ; unused(pIncoming) ;

	// Need to return false if we're not using the debugger, which looks
	// the same to the caller as if we had the debugger up and the user has quit from it.
	bool keepGoing = false ;

#ifdef USE_TCL_DEBUGGER
	if (m_Debugger)
	{
		// The update function returns false if the user exits the debugger.
		keepGoing = TgD::TgD::Update(false, m_Debugger) ;
	}
#endif

	return this->ReturnBoolResult(pConnection, pResponse, keepGoing) ;
}

bool KernelSML::HandleLoadProductions(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pResponse) ;

	if (!pAgent)
		return false ;

	// Get the parameters
	char const* pFilename = pIncoming->GetArgValue(sml_Names::kParamFilename) ;

	if (!pFilename)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Filename missing") ;
	}

	// Make the call.
	bool ok = pAgent->GetProductionManager()->LoadSoarFile(pFilename, pError) ;

	return ok ;
}

bool KernelSML::HandleGetInputLink(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pConnection) ; unused(pIncoming) ;

	if (!pAgent)
		return false ;

	// We want the input link's id
	// Start with the root object for the input link
	IWMObject* pRootObject = NULL ;
	pAgent->GetInputLink()->GetRootObject(&pRootObject, pError) ;

	if (pRootObject == NULL)
		return false ;

	// Get the symbol for the id of this object
	ISymbol const* pID = pRootObject->GetId(pError) ;

	if (pID == NULL)
	{
		pRootObject->Release() ;
		return false ;
	}

	// Turn the id symbol into an actual string
	char const* id = pID->GetString(pError) ;
	
	if (id)
	{
		// Fill in the id string as the result of this command
		this->ReturnResult(pConnection, pResponse, id) ;
	}

	// No need to release the pID
	// because it's returned as a const by GetId().
	// pID->Release() ;

	// Clean up
	pRootObject->Release() ;

	// We succeeded if we got an id string
	return (id != NULL) ;
}

bool KernelSML::AddInputWME(gSKI::IAgent* pAgent, char const* pID, char const* pAttribute, char const* pValue, char const* pType, char const* pTimeTag, gSKI::Error* pError)
{
	IWorkingMemory* pInputWM = pAgent->GetInputLink()->GetInputLinkMemory(pError) ;

	// First get the object which will own this new wme
	// Because we build from the top down, this should always exist by the
	// time we wish to add structure beneath it.
	IWMObject* pParentObject = NULL ;
	pInputWM->GetObjectById(pID, &pParentObject) ;

	// Soar also allows the environment to modify elements on the output link.
	// This is a form of backdoor input, so we need to check on the output side too
	// if we don't find our parent on the input side.
	if (!pParentObject)
	{
		pInputWM = pAgent->GetOutputLink()->GetOutputMemory(pError) ;
		pInputWM->GetObjectById(pID, &pParentObject) ;
	}

	// Failed to find the parent.
	if (!pParentObject)
		return false ;

	IWme* pWME = NULL ;

	if (IsStringEqual(sml_Names::kTypeString, pType))
	{
		// Add a WME with a string value
		pWME = pInputWM->AddWmeString(pParentObject, pAttribute, pValue, pError) ;
	}
	else if (IsStringEqual(sml_Names::kTypeID, pType))
	{
		// Add a WME with an identifier value
		pWME = pInputWM->AddWmeNewObject(pParentObject, pAttribute, pError) ;
		
		if (pWME)
		{
			// We need to record the id that the kernel assigned to this object and match it against the id the
			// client is using, so that in future we can map the client's id to the kernel's.
			char const* pKernelID = pWME->GetValue()->GetString() ;
			RecordIDMapping(pValue, pKernelID) ;
		}
	}
	else if (IsStringEqual(sml_Names::kTypeInt, pType))
	{
		// Add a WME with an int value
		int value = atoi(pValue) ;
		pWME = pInputWM->AddWmeInt(pParentObject, pAttribute, value, pError) ;
	}
	else if (IsStringEqual(sml_Names::kTypeDouble, pType))
	{
		// Add a WME with a float value
		double value = atof(pValue) ;
		pWME = pInputWM->AddWmeDouble(pParentObject, pAttribute, value, pError) ;
	}

	if (!pWME)
	{
		pParentObject->Release() ;
		return false ;
	}

	// Well here's a surprise.  The kernel doesn't support a direct lookup from timeTag to wme.
	// So we need to maintain our own map out here so we can find the WME's quickly for removal.
	// So where we had planned to map from client time tag to kernel time tag, we'll instead
	// map from client time tag to IWme*.
	// That means we need to be careful to delete the IWme* objects later.
	RecordTimeTag(pTimeTag, pWME) ;

	// We'll release this when the table of time tags is eventually destroyed or
	// when the wme is deleted.
//	pWME->Release() ;

	pParentObject->Release() ;

	return true ;
}

bool KernelSML::RemoveInputWME(gSKI::IAgent* pAgent, char const* pTimeTag, gSKI::Error* pError)
{
	IWorkingMemory* pInputWM = pAgent->GetInputLink()->GetInputLinkMemory(pError) ;

	// Get the wme that matches this time tag
	IWme* pWME = ConvertTimeTag(pTimeTag) ;

	// Failed to find the wme--that shouldn't happen.
	if (!pWME)
		return false ;

	// Remove the object from the time tag table
	RemoveTimeTag(pTimeTag) ;

	// If this is an identifier, need to remove it from the ID mapping table too.
	if (pWME->GetValue()->GetType() == gSKI_OBJECT)
	{
		// BUGBUG: I think we need a reverse ID mapping table, to get back
		// from the kernel id to the client id, so we can do cleanup.
		// Otherwise, I don't see how to do the cleanup.
	}

	// Remove the wme from working memory
	pInputWM->RemoveWme(pWME, pError) ;

	// I'm not sure if we release pWME 
	pWME->Release() ;

	return true ;
}

// Add or remove a list of wmes we've been sent
bool KernelSML::HandleInput(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pResponse) ; unused(pConnection) ;

	if (!pAgent)
		return false ;

	// Get the command tag which contains the list of wmes
	ElementXML const* pCommand = pIncoming->GetCommandTag() ;

	int nChildren = pCommand->GetNumberChildren() ;

	ElementXML wmeXML(NULL) ;
	ElementXML* pWmeXML = &wmeXML ;

	bool ok = true ;

	for (int i = 0 ; i < nChildren ; i++)
	{
		pCommand->GetChild(&wmeXML, i) ;

		// Ignore tags that aren't wmes.
		if (!pWmeXML->IsTag(sml_Names::kTagWME))
			continue ;

		// Find out if this is an add or a remove
		char const* pAction = pWmeXML->GetAttribute(sml_Names::kWME_Action) ;

		if (!pAction)
			continue ;

		bool add = IsStringEqual(pAction, sml_Names::kValueAdd) ;
		bool remove = IsStringEqual(pAction, sml_Names::kValueRemove) ;

		if (add)
		{
			char const* pID			= pWmeXML->GetAttribute(sml_Names::kWME_Id) ;	// May be a client side id value (e.g. "o3" not "O3")
			char const* pAttribute  = pWmeXML->GetAttribute(sml_Names::kWME_Attribute) ;
			char const* pValue		= pWmeXML->GetAttribute(sml_Names::kWME_Value) ;
			char const* pType		= pWmeXML->GetAttribute(sml_Names::kWME_ValueType) ;	// Can be NULL (=> string)
			char const* pTimeTag	= pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// May be a client side time tag (e.g. -3 not +3)

			// Set the default value
			if (!pType)
				pType = sml_Names::kTypeString ;

			// Check we got everything we need
			if (!pID || !pAttribute || !pValue || !pTimeTag)
				continue ;

			// Map the ID from client side to kernel side (if the id is already a kernel side id it's returned unchanged)
			std::string id ;
			ConvertID(pID, &id) ;

			// Add the wme
			ok = AddInputWME(pAgent, id.c_str(), pAttribute, pValue, pType, pTimeTag, pError) && ok ;
		}
		else if (remove)
		{
			char const* pTimeTag = pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// May be (will be?) a client side time tag (e.g. -3 not +3)

			// Remove the wme
			ok = RemoveInputWME(pAgent, pTimeTag, pError) && ok ;
		}
	}

	// Returns false if any of the adds/removes fails
	return ok ;
}

bool KernelSML::HandleCommandLine(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ;

   if (!pAgent)
      return false ;

	// Get the parameters
	char const* pLine = pIncoming->GetArgValue(sml_Names::kParamLine) ;

	if (!pLine)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Command line missing") ;
	}

	// Make the call.
	return m_CommandLineInterface.DoCommand(pConnection, pAgent, pLine, pResponse, pError) ;
}

