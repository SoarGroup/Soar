/////////////////////////////////////////////////////////////////
// KernelSMLgSKI class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This is a set of commands used to send and receive
// primitive gSKI objects.  Currently these are all deprecated.
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef _WIN32
#include <crtdbg.h>
#endif

#include "sml_KernelSMLgSKI.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"

#include "gSKI.h"
#include <iostream>
#include <fstream>
#include <map>
#include <stdlib.h>

#include "IgSKI_KernelFactory.h"
#include "gSKI_Stub.h"
#include "IgSKI_Kernel.h"
#include "../../gSKI/src/gSKI_Error.h"
#include "gSKI_ErrorIds.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_OutputProcessor.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_Wme.h"

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

using namespace sml ;
using namespace gSKI ;

// BADBAD: Not sure where this macro is coming from but I saw this
// in IgSKI_Symbol.h and it's needed for the GetObject() calls to compile.
#ifdef _WIN32
#undef GetObject
#undef SendMessage
#endif

// These types need to remain in scope as we just store the pointer.
// (Note--this information is only used in the debug builds).
static char const* kIKernelFactory	= "IKernelFactory" ;
static char const* kIKernel			= "IKernel" ;
static char const* kIAgentManager	= "IAgentManager" ;
static char const* kIAgent			= "IAgent" ;
static char const* kIInputLink		= "IInputLink" ;
static char const* kIOutputLink		= "IOutputLink" ;
static char const* kIInputProducer	= "IInputProducer" ;
static char const* kIOutputProcessor= "IOutputProcessor" ;
static char const* kIWorkingMemory	= "IWorkingMemory" ;
static char const* kIWMObject		= "IWMObject" ;
static char const* kIWme			= "IWme" ;
static char const* kISymbol			= "ISymbol" ;
static char const* kIIteratorPointer= "IIteratorPointer" ;
static char const* kVoid			= "void*" ;

KernelSMLgSKI* KernelSMLgSKI::s_pKernel = NULL ;

static void DebugPrint(char const* pFilename, int line, char const* pMsg)
{
#ifdef _WIN32
#ifdef _DEBUG
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	_CrtDbgReport(_CRT_WARN, pFilename, line, "KernelSMLgSKI", pMsg);
#else
	unused(pFilename) ;
	unused(line) ;
	unused(pMsg) ;
#endif
#endif
}


static ElementXML* AddErrorMsg(Connection* pConnection, ElementXML* pResponse, char const* pErrorMsg, int errorCode = -1)
{
	pConnection->AddErrorToSMLResponse(pResponse, pErrorMsg, errorCode) ;
	return pResponse ;
}

static ElementXML* AddResultID(Connection* pConnection, ElementXML* pResponse, char const* pID)
{
	pConnection->AddSimpleResultToSMLResponse(pResponse, pID) ;
	return pResponse ;
}

/*
  ==================================
  SML input processor
  ==================================
*/
class sml_InputProducer_gSKI: public IInputProducer
{
public:

   // Simple constructor
   sml_InputProducer_gSKI(KernelSMLgSKI* pKernelSML, sml::Connection* pConnection, char const* pID)
   {
	   m_KernelSML	= pKernelSML ;
	   m_ID			= pID ;
	   m_Connection = pConnection ;
   }
   
   // Virtual destructor for the usual reasons
   virtual ~sml_InputProducer_gSKI() 
   {
   }
   
   // The update function required by the IInputProducer interface
   // (Responsible for updating the state of working memory)
   virtual void Update(IWorkingMemory* wmemory,
                       IWMObject* obj)
   {
	   char id1[KernelSMLgSKI::kMaxIDLen] ;
	   char id2[KernelSMLgSKI::kMaxIDLen] ;

	   AnalyzeXML response ;

	   m_Connection->SendClassCommand(&response, sml_Names::kgSKI_IInputProducer_Update, m_ID.c_str(), 
			sml_Names::kParamWorkingMemory, m_KernelSML->GenerateID(wmemory, id1),
			sml_Names::kParamWmeObject, m_KernelSML->GenerateID(obj, id2)) ;
   }

private:
	sml::KernelSMLgSKI*	 m_KernelSML ;
	std::string		 m_ID ;
	sml::Connection* m_Connection ;
};

/*
  ==================================
  SML output processor
  ==================================
*/
class sml_OutputProcessor: public IOutputProcessor
{
public:

   // Simple constructor
   sml_OutputProcessor(KernelSMLgSKI* pKernelSML, sml::Connection* pConnection, char const* pID)
   {
	   m_KernelSML	= pKernelSML ;
	   m_ID			= pID ;
	   m_Connection = pConnection ;
   }
   
   // Virtual destructor for the usual reasons
   virtual ~sml_OutputProcessor() 
   {
   }
   
   // The update function required by the IInputProducer interface
   // (Responsible for updating the state of working memory)
   virtual void ProcessOutput(IWorkingMemory* wmemory,
							  IWMObject* obj)
   {
	   AnalyzeXML response ;

	   // During output, we may get a new wmobject, so we need to record it so
	   // we can find it again later on if the user makes use of this value.
	   // I guess the same could be true for the working memory possibly.
	   char const* pID1 = m_KernelSML->StoreObject(wmemory, kIWorkingMemory, false) ;
	   char const* pID2 = m_KernelSML->StoreObject(obj, kIWMObject, true) ;

	   m_Connection->SendClassCommand(&response, sml_Names::kgSKI_IOutputProcessor_ProcessOutput, m_ID.c_str(), 
			sml_Names::kParamWorkingMemory, pID1,
			sml_Names::kParamWmeObject, pID2) ;
   }

private:
	sml::KernelSMLgSKI*	 m_KernelSML ;
	std::string		 m_ID ;
	sml::Connection* m_Connection ;
};

/*************************************************************
* @brief	Returns the singleton kernel object.
*************************************************************/
KernelSMLgSKI* KernelSMLgSKI::GetKernel()
{
	if (s_pKernel == NULL)
		s_pKernel = new KernelSMLgSKI() ;

	return s_pKernel ;
}

KernelSMLgSKI::KernelSMLgSKI()
{
	// Create a Kernel Factory
	m_pKernelFactory = gSKI_CreateKernelFactory();
   
	// Create the kernel instance
	m_pIKernel = m_pKernelFactory->Create();

	m_AllocationCounter = 0 ;

#ifdef USE_TCL_DEBUGGER
	m_Debugger = NULL ;
	m_DebuggerKernel = NULL ;
	m_DebuggerKernelFactory = NULL ;
#endif

	// Create the map from command name to handler function
	BuildCommandMap() ; 
}

KernelSMLgSKI::~KernelSMLgSKI()
{
	if (m_pKernelFactory && m_pIKernel)
		m_pKernelFactory->DestroyKernel(m_pIKernel);

	ObjectMapConstIter mapIter = m_ObjectMap.begin() ;

	// When we're done, this map should be empty because each
	// object created should have been released.
	while (mapIter != m_ObjectMap.end())
	{
		char* pID = mapIter->first ;

// If we've recorded the extra information, we can report on it now.
#ifdef USE_KERNEL_OBJECT_INFO
		KernelObjectInfo* pInfo = (KernelObjectInfo*)mapIter->second ;

		char buffer[20] ;

		std::string msg = "Type " ;
		msg += pInfo->m_pType ;
		msg += " (" ;
		msg += pID ;
		msg += ") {" ;
		msg += itoa(pInfo->m_AllocationCount, buffer, sizeof(buffer)) ;
		msg += "} " ;

		if (pInfo->m_ShouldBeReleased)
			msg += " does not appear to have been deleted.  Probably a missed IRelease call in the client.\n" ;
		else
			msg += " was not released and I believe this is correct.\n" ;

		DebugPrint(__FILE__, __LINE__, msg.c_str());

		delete pInfo ;
#endif

		// We always need to delete the id string
		delete[] pID ;
		mapIter++ ;
	}

	InputProducerListIter_t iter = m_InputProducers.begin() ;

	// Delete all of the input producers ever created
	while (iter != m_InputProducers.end())
	{
		IInputProducer* pProd = *iter ;
		delete pProd ;

		iter++ ;
	}

	OutputProcessorListIter_t iter1 = m_OutputProcessors.begin() ;

	// Delete all of the output processors ever created
	while (iter1 != m_OutputProcessors.end())
	{
		IOutputProcessor* pProcess = *iter1 ;
		delete pProcess ;

		iter1++ ;
	}
}

/*
IKernelFactory
IKernel
IAgentManager
IAgent
IInputLink
IOutputLink
IInputProducer
IOutputProcessor
IWorkingMemory
IWMObject
IWme

<command name="IKernelFactory::Create">
	<arg name="this">0x5544</arg>
	<arg> ...
</command>
*/


/*************************************************************
* @brief	A command handler (SML message->gSKI function call).
*
* The method being called is e.g. IKernel::GetAgentManager()
* The this pointer in this case will be an IKernel* object.
*
* @param pThis			The 'this' pointer for this call (NULL if a static method is being called)
* @param pCommandName	The SML command name (so one handler can handle many incoming calls if we wish)
* @param pConnection	The connection this command came in on
* @param pIncoming		The incoming, analyzed message.
* @param pResponse		The partially formed response.  This handler needs to fill in more of this.
* @param pError			A gSKI error object, which gSKI will fill in if there are errors.
* @returns False if we had an error and wish to generate a generic error message (based on the incoming call + pError)
*          True if the call succeeded or we generated another more specific error already.
*************************************************************/
bool KernelSMLgSKI::IAgentManager_AddAgent(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	char const* pName				= pIncoming->GetArgValue(sml_Names::kParamName) ;
	char const* pProdFileName		= pIncoming->GetArgValue(sml_Names::kParamFilename) ;
	bool learningOn					= pIncoming->GetArgBool(sml_Names::kParamLearning, false) ;
	egSKIOSupportMode oSupportMode	= (egSKIOSupportMode)pIncoming->GetArgInt(sml_Names::kParamOSupportMode, gSKI_O_SUPPORT_MODE_4) ;

	if (!pName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Make the call
	IAgent* pResult = ((IAgentManager*)pThis)->AddAgent(pName, pProdFileName, learningOn, oSupportMode, pError) ;

#ifdef USE_TCL_DEBUGGER
	// BADBAD: Now create the Tcl debugger.  This is a complete hack and can come out once we have some way to debug this stuff.
	if (m_Debugger == NULL && m_DebuggerKernel && m_DebuggerKernelFactory)
	{
		m_Debugger = CreateTgD(pResult, m_DebuggerKernel, m_DebuggerKernelFactory->GetKernelVersion(), TgD::TSI40, "") ;
		m_Debugger->Init() ;
	}	
#endif

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIAgent, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IIterator_Pointer_IsValid(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	// BUGBUG? IIterator is a template.  Is it safe to access it like this with a specific concrete class?  Seems like this
	// should be OK, as long as the data type is a pointer.
	bool result = ((tIWmeIterator*)pThis)->IsValid(pError) ;

	// Return the pointer as an id to the caller
	return ReturnBoolResult(pConnection, pResponse, result) ;
}

bool KernelSMLgSKI::IIterator_Pointer_Next(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ; unused(pResponse) ; unused(pConnection) ;

	// Make the call
	// BUGBUG? IIterator is a template.  Is it safe to access it like this with a specific concrete class?  Seems like this
	// should be OK, as long as the data type is a pointer.
	((tIWmeIterator*)pThis)->Next(pError) ;

	// void call, so nothing returned
	return true ;
}

bool KernelSMLgSKI::IOutputLink_AddOutputProcessor(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Get the parameters
	char const* pAttributePath	   = pIncoming->GetArgValue(sml_Names::kParamAttributePath) ;
	char const* pOutputProcessorID = pIncoming->GetArgValue(sml_Names::kParamOutputProcessor) ;

	if (!pAttributePath || !pOutputProcessorID)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Create the local, kernel side OutputProcessor object
	sml_OutputProcessor* pOutputProcessor = new sml_OutputProcessor(this, pConnection, pOutputProcessorID) ;

	// We have to keep track of this object and delete it when
	// the KernelSMLgSKI object is deleted.  (We could add a mechanism for deleting them
	// when the client deletes its output processor, but not today).
	m_OutputProcessors.push_back(pOutputProcessor) ;

	// Make the call
	((IOutputLink*)pThis)->AddOutputProcessor(pAttributePath, pOutputProcessor, pError) ;

	// We don't return a <result> tag for void functions, so just return true here.
	return true ;
}

bool KernelSMLgSKI::IInputLink_AddInputProducer(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Get the parameters
	IWMObject* pWMobject		 = (IWMObject*)LookupObject(pIncoming->GetArgValue(sml_Names::kParamWmeObject), kIWMObject) ;
	char const* pInputProducerID = pIncoming->GetArgValue(sml_Names::kParamInputProducer) ;

	if (!pWMobject || !pInputProducerID)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Create the local, kernel side InputProducer object
	sml_InputProducer_gSKI* pInputProducer = new sml_InputProducer_gSKI(this, pConnection, pInputProducerID) ;

	// We have to keep track of this object and delete it when
	// the KernelSMLgSKI object is deleted.  (We could add a mechanism for deleting them
	// when the client deletes its input producer, but not today).
	m_InputProducers.push_back(pInputProducer) ;

	// Make the call
	((IInputLink*)pThis)->AddInputProducer(pWMobject, pInputProducer, pError) ;

	// We don't return a <result> tag for void functions, so just return true here.
	return true ;
}

bool KernelSMLgSKI::IIterator_Pointer_GetVal(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	// BUGBUG? IIterator is a template.  Is it safe to access it like this with a specific concrete class?  Seems like this
	// should be OK, as long as the data type is a pointer.
	void* pResult = ((tIWmeIterator*)pThis)->GetVal(pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kVoid, kShouldNotBeReleased) ;
}

bool KernelSMLgSKI::IWMObject_GetWMEs(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	char const* pAttr = pIncoming->GetArgValue(sml_Names::kParamAttribute) ;
	egSKISymbolType valueType = (egSKISymbolType)pIncoming->GetArgInt(sml_Names::kParamValue, gSKI_ANY_SYMBOL) ;

	// Make the call
	tIWmeIterator* pResult = ((IWMObject*)pThis)->GetWMEs(pAttr, valueType, pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIIteratorPointer, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IAgent_RunInClientThread(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	egSKIRunType runLength = (egSKIRunType)pIncoming->GetArgInt(sml_Names::kParamLength, gSKI_RUN_FOREVER) ;
	int count = pIncoming->GetArgInt(sml_Names::kParamCount, 1) ;

	// Run Soar
	egSKIRunResult result = ((IAgent*)pThis)->RunInClientThread(runLength, count, pError) ;

	if (pError && isError(*pError))
		return false ;

	return ReturnIntResult(pConnection, pResponse, result) ;
}

bool KernelSMLgSKI::IWorkingMemory_GetAgent(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	IAgent* pResult = ((IWorkingMemory*)pThis)->GetAgent(pError);

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIAgent, kShouldNotBeReleased) ;
}

bool KernelSMLgSKI::IWorkingMemory_RemoveObject(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	IWMObject* pWMobject = (IWMObject*)LookupObject(pIncoming->GetArgValue(sml_Names::kParamWmeObject), kIWMObject) ;

	if (!pWMobject)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Make the call
	((IWorkingMemory*)pThis)->RemoveObject(pWMobject, pError) ;

	// We don't return a <result> tag for void functions, so just return true here.
	return true ;
}

bool KernelSMLgSKI::IWorkingMemory_AddWmeInt(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	IWMObject* pWMobject = (IWMObject*)LookupObject(pIncoming->GetArgValue(sml_Names::kParamWmeObject), kIWMObject) ;
	char const* pAttr = pIncoming->GetArgValue(sml_Names::kParamAttribute) ;
	char const* pVal  = pIncoming->GetArgValue(sml_Names::kParamValue) ;

	if (!pWMobject || !pAttr || !pVal)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	int val = atoi(pVal) ;

	// Make the call
	IWme* pResult = ((IWorkingMemory*)pThis)->AddWmeInt(pWMobject, pAttr, val, pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWme, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IWorkingMemory_AddWmeObjectLink(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	IWMObject* pWMobject = (IWMObject*)LookupObject(pIncoming->GetArgValue(sml_Names::kParamWmeObject), kIWMObject) ;
	char const* pAttr = pIncoming->GetArgValue(sml_Names::kParamAttribute) ;
	IWMObject* pVal = (IWMObject*)LookupObject(pIncoming->GetArgValue(sml_Names::kParamValue), kIWMObject) ;

	if (!pWMobject || !pAttr || !pVal)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Make the call
	IWme* pResult = ((IWorkingMemory*)pThis)->AddWmeObjectLink(pWMobject, pAttr, pVal, pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWme, kShouldBeReleased) ;
}


bool KernelSMLgSKI::IWorkingMemory_AddWmeDouble(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	IWMObject* pWMobject = (IWMObject*)LookupObject(pIncoming->GetArgValue(sml_Names::kParamWmeObject), kIWMObject) ;
	char const* pAttr = pIncoming->GetArgValue(sml_Names::kParamAttribute) ;
	char const* pVal  = pIncoming->GetArgValue(sml_Names::kParamValue) ;

	if (!pWMobject || !pAttr || !pVal)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	double val = atof(pVal) ;

	// Make the call
	IWme* pResult = ((IWorkingMemory*)pThis)->AddWmeDouble(pWMobject, pAttr, val, pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWme, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IWorkingMemory_AddWmeString(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	IWMObject* pWMobject = (IWMObject*)LookupObject(pIncoming->GetArgValue(sml_Names::kParamWmeObject), kIWMObject) ;
	char const* pAttr = pIncoming->GetArgValue(sml_Names::kParamAttribute) ;
	char const* pVal  = pIncoming->GetArgValue(sml_Names::kParamValue) ;

	if (!pWMobject || !pAttr || !pVal)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Make the call
	IWme* pResult = ((IWorkingMemory*)pThis)->AddWmeString(pWMobject, pAttr, pVal, pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWme, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IWorkingMemory_AddWmeNewObject(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	IWMObject* pWMobject = (IWMObject*)LookupObject(pIncoming->GetArgValue(sml_Names::kParamWmeObject), kIWMObject) ;
	char const* pAttr = pIncoming->GetArgValue(sml_Names::kParamAttribute) ;

	if (!pWMobject || !pAttr)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Make the call
	IWme* pResult = ((IWorkingMemory*)pThis)->AddWmeNewObject(pWMobject, pAttr, pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWme, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IWorkingMemory_ReplaceStringWme(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	IWme* pOldWme = (IWme*)LookupObject(pIncoming->GetArgValue(sml_Names::kParamWme), kIWme) ;
	char const* pString = pIncoming->GetArgValue(sml_Names::kParamValue) ;

	if (!pOldWme || !pString)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Make the call
	IWme* pResult = ((IWorkingMemory*)pThis)->ReplaceStringWme(pOldWme, pString, pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWme, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IWorkingMemory_ReplaceIntWme(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	IWme* pOldWme = (IWme*)LookupObject(pIncoming->GetArgValue(sml_Names::kParamWme), kIWme) ;
	const char* pString = pIncoming->GetArgValue(sml_Names::kParamValue) ;

	if (!pOldWme || !pString)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Make the call
	IWme* pResult = ((IWorkingMemory*)pThis)->ReplaceIntWme(pOldWme, atoi(pString), pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWme, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IWorkingMemory_ReplaceDoubleWme(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	IWme* pOldWme = (IWme*)LookupObject(pIncoming->GetArgValue(sml_Names::kParamWme), kIWme) ;
	char const* pString = pIncoming->GetArgValue(sml_Names::kParamValue) ;

	if (!pOldWme || !pString)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Make the call
	IWme* pResult = ((IWorkingMemory*)pThis)->ReplaceDoubleWme(pOldWme, atof(pString), pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWme, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IWme_GetValue(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	ISymbol const* pResult = ((IWme*)pThis)->GetValue(pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kISymbol, kShouldBeReleased) ;
}

bool KernelSMLgSKI::ISymbol_GetObject(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	IWMObject* pResult = ((ISymbol*)pThis)->GetObject(pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWMObject, kShouldBeReleased) ;
}

bool KernelSMLgSKI::ISymbol_GetString(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	char const* pResult = ((ISymbol*)pThis)->GetString(pError) ;

	if (!pResult)
		return false ;

	pConnection->AddSimpleResultToSMLResponse(pResponse, pResult) ;

	return true ;
}

bool KernelSMLgSKI::ISymbol_GetInt(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	int result = ((ISymbol*)pThis)->GetInt(pError) ;

	if (pError && isError(*pError))
		return false ;

	return ReturnIntResult(pConnection, pResponse, result) ;
}

bool KernelSMLgSKI::IOutputLink_GetRootObject(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	IWMObject* pResult ;
	((IOutputLink*)pThis)->GetRootObject(&pResult, pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWMObject, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IInputLink_GetRootObject(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	IWMObject* pResult ;
	((IInputLink*)pThis)->GetRootObject(&pResult, pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWMObject, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IOutputLink_SetAutomaticUpdate(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pResponse) ; unused(pConnection) ;

	// Get the parameters
	bool state = pIncoming->GetArgBool(sml_Names::kParamValue, true) ;

	// Make the call
	((IOutputLink*)pThis)->SetAutomaticUpdate(state, pError) ;

	// No result
	return true ;
}

bool KernelSMLgSKI::IAgent_GetOutputLink(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	IOutputLink* pResult = ((IAgent*)pThis)->GetOutputLink(pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIOutputLink, kShouldNotBeReleased) ;
}

bool KernelSMLgSKI::IAgent_GetInputLink(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	IInputLink* pResult = ((IAgent*)pThis)->GetInputLink(pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIInputLink, kShouldNotBeReleased) ;
}

bool KernelSMLgSKI::IInputLink_GetInputLinkMemory(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	IWorkingMemory* pResult = ((IInputLink*)pThis)->GetInputLinkMemory(pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWorkingMemory, kShouldNotBeReleased) ;
}

bool KernelSMLgSKI::IOutputLink_GetOutputLinkMemory(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	IWorkingMemory* pResult = ((IOutputLink*)pThis)->GetOutputMemory(pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIWorkingMemory, kShouldNotBeReleased) ;
}

bool KernelSMLgSKI::IKernel_GetAgentManager(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Make the call
	IAgentManager* pResult = ((IKernel*)pThis)->GetAgentManager(pError) ;

	// Return the pointer as an id to the caller
	return ReturnResult(pConnection, pResponse, pResult, kIAgentManager, kShouldNotBeReleased) ;
}

bool KernelSMLgSKI::IRelease_Release(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pIncoming) ; unused(pError) ; unused(pResponse) ; unused(pConnection) ;

	// This destroys the object.
	((IRelease*)pThis)->Release() ;

	// So we need to also delete it from our table
	char const* pThisID = pIncoming->GetArgValue(sml_Names::kParamThis) ;
	DeleteObject(pThisID) ;

	return true ;
}

bool KernelSMLgSKI::IAgentManager_RemoveAgent(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the params
	char const* pAgentID = pIncoming->GetArgValue(sml_Names::kParamAgent) ;
	IAgent* pAgent = (IAgent*)LookupObject(pAgentID, kIAgent) ;

	if (!pAgent)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Make the call, destroying the kernel.
	((IAgentManager*)pThis)->RemoveAgent(pAgent, pError) ;

	// So we need to also delete it from our table
	DeleteObject(pAgentID) ;

	// This is a void call, so no data to return.
	return true ;
}

bool KernelSMLgSKI::IKernelFactory_Create(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the parameters
	char const* pName			= pIncoming->GetArgValue(sml_Names::kParamName) ;
	egSKIThreadingModel thread	= (egSKIThreadingModel)pIncoming->GetArgInt(sml_Names::kParamThread, gSKI_MULTI_THREAD) ;
	egSKIProcessType   process	= (egSKIProcessType)pIncoming->GetArgInt(sml_Names::kParamProcess, gSKI_ANY_PROCESS) ;
	char const* pLocation		= pIncoming->GetArgValue(sml_Names::kParamLocation) ;
	char const* pLogLocation	= pIncoming->GetArgValue(sml_Names::kParamLogLocation) ;
	egSKILogActivityLevel log	= (egSKILogActivityLevel)pIncoming->GetArgInt(sml_Names::kParamLogLevel, gSKI_LOG_ERRORS) ;
	
	// Make the call
	IKernel* pKernel = ((IKernelFactory*)pThis)->Create(pName, thread, process, pLocation, pLogLocation, log, pError) ;
	
#ifdef USE_TCL_DEBUGGER
	m_DebuggerKernel = pKernel ;
#endif

	return ReturnResult(pConnection, pResponse, pKernel, kIKernel, kShouldBeReleased) ;
}

bool KernelSMLgSKI::IKernelFactory_DestroyKernel(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ;

	// Get the params
	char const* pKernelID = pIncoming->GetArgValue(sml_Names::kParamKernel) ;
	IKernel* pKernel = (IKernel*)LookupObject(pKernelID, kIKernel) ;

	if (!pKernel)
	{
		return InvalidArg(pConnection, pResponse, pCommandName) ;
	}

	// Make the call, destroying the kernel.
	((IKernelFactory*)pThis)->DestroyKernel(pKernel, pError) ;

#ifdef USE_TCL_DEBUGGER
	if (m_DebuggerKernel == pKernel)
		m_DebuggerKernel = NULL ;
#endif

	// So we need to also delete it from our table
	DeleteObject(pKernelID) ;

	// This is a void call, so no data to return.
	return true ;
}

bool KernelSMLgSKI::CreateKernelFactory(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError)
{
	unused(pCommandName) ; unused(pError) ; unused(pIncoming) ;
	unused(pThis) ;	// This is a static function

	// Make the call
	IKernelFactory* pFactory = gSKI_CreateKernelFactory() ;

#ifdef USE_TCL_DEBUGGER
	m_DebuggerKernelFactory = pFactory ;
#endif

	return ReturnResult(pConnection, pResponse, pFactory, kIKernelFactory, kShouldBeReleased) ;
}

void KernelSMLgSKI::BuildCommandMap()
{
	m_CommandMap[sml_Names::kgSKI_IAgent_RunInClientThread]	= &sml::KernelSMLgSKI::IAgent_RunInClientThread ;

	m_CommandMap[sml_Names::kgSKI_IWMObject_GetWMEs]			= &sml::KernelSMLgSKI::IWMObject_GetWMEs ;

	m_CommandMap[sml_Names::kgSKI_IIterator_Pointer_IsValid]= &sml::KernelSMLgSKI::IIterator_Pointer_IsValid ;
	m_CommandMap[sml_Names::kgSKI_IIterator_Pointer_GetVal] = &sml::KernelSMLgSKI::IIterator_Pointer_GetVal ;
	m_CommandMap[sml_Names::kgSKI_IIterator_Pointer_Next]	= &sml::KernelSMLgSKI::IIterator_Pointer_Next ;

	m_CommandMap[sml_Names::kgSKI_IWorkingMemory_ReplaceStringWme]= &sml::KernelSMLgSKI::IWorkingMemory_ReplaceStringWme ;
	m_CommandMap[sml_Names::kgSKI_IWorkingMemory_AddWmeNewObject]= &sml::KernelSMLgSKI::IWorkingMemory_AddWmeNewObject ;
	m_CommandMap[sml_Names::kgSKI_IWorkingMemory_AddWmeString]	= &sml::KernelSMLgSKI::IWorkingMemory_AddWmeString ;
	m_CommandMap[sml_Names::kgSKI_IWorkingMemory_AddWmeInt]		= &sml::KernelSMLgSKI::IWorkingMemory_AddWmeInt ;
	m_CommandMap[sml_Names::kgSKI_IWorkingMemory_RemoveObject]	= &sml::KernelSMLgSKI::IWorkingMemory_RemoveObject ;
	m_CommandMap[sml_Names::kgSKI_IWorkingMemory_GetAgent]		= &sml::KernelSMLgSKI::IWorkingMemory_GetAgent ;
	m_CommandMap[sml_Names::kgSKI_IWorkingMemory_AddWmeObjectLink]	= &sml::KernelSMLgSKI::IWorkingMemory_AddWmeObjectLink ;

	m_CommandMap[sml_Names::kgSKI_ISymbol_GetObject]	 = &sml::KernelSMLgSKI::ISymbol_GetObject ;
	m_CommandMap[sml_Names::kgSKI_ISymbol_GetString]	 = &sml::KernelSMLgSKI::ISymbol_GetString ;
	m_CommandMap[sml_Names::kgSKI_ISymbol_GetInt]		 = &sml::KernelSMLgSKI::ISymbol_GetInt ;

	m_CommandMap[sml_Names::kgSKI_IWme_GetValue]		 = &sml::KernelSMLgSKI::IWme_GetValue ;

	m_CommandMap[sml_Names::kgSKI_IInputLink_GetInputLinkMemory]= &sml::KernelSMLgSKI::IInputLink_GetInputLinkMemory ;
	m_CommandMap[sml_Names::kgSKI_IInputLink_GetRootObject]		= &sml::KernelSMLgSKI::IInputLink_GetRootObject ;
	m_CommandMap[sml_Names::kgSKI_IInputLink_AddInputProducer]	= &sml::KernelSMLgSKI::IInputLink_AddInputProducer ;

	m_CommandMap[sml_Names::kgSKI_IAgentManager_AddAgent]		= &sml::KernelSMLgSKI::IAgentManager_AddAgent ;
	m_CommandMap[sml_Names::kgSKI_IAgentManager_RemoveAgent]	= &sml::KernelSMLgSKI::IAgentManager_RemoveAgent ;
	m_CommandMap[sml_Names::kgSKI_IAgent_GetInputLink]			= &sml::KernelSMLgSKI::IAgent_GetInputLink ;
	m_CommandMap[sml_Names::kgSKI_IKernel_GetAgentManager]		= &sml::KernelSMLgSKI::IKernel_GetAgentManager ;
	m_CommandMap[sml_Names::kgSKI_CreateKernelFactory]			= &sml::KernelSMLgSKI::CreateKernelFactory ;
	m_CommandMap[sml_Names::kgSKI_IKernelFactory_Create]		= &sml::KernelSMLgSKI::IKernelFactory_Create ;
	m_CommandMap[sml_Names::kgSKI_IKernelFactory_DestroyKernel] = &sml::KernelSMLgSKI::IKernelFactory_DestroyKernel ;
	m_CommandMap[sml_Names::kgSKI_IRelease_Release]				= &sml::KernelSMLgSKI::IRelease_Release ;

	m_CommandMap[sml_Names::kgSKI_IAgent_GetOutputLink]			  = &sml::KernelSMLgSKI::IAgent_GetOutputLink ;
	m_CommandMap[sml_Names::kgSKI_IOutputLink_AddOutputProcessor] = &sml::KernelSMLgSKI::IOutputLink_AddOutputProcessor ;
	m_CommandMap[sml_Names::kgSKI_IOutputLink_GetOutputLinkMemory]= &sml::KernelSMLgSKI::IOutputLink_GetOutputLinkMemory ;
	m_CommandMap[sml_Names::kgSKI_IOutputLink_SetAutomaticUpdate] = &sml::KernelSMLgSKI::IOutputLink_SetAutomaticUpdate ;
	m_CommandMap[sml_Names::kgSKI_IOutputLink_GetRootObject]	  = &sml::KernelSMLgSKI::IOutputLink_GetRootObject ;
}

/*************************************************************
* @brief	Return an object* to the caller.
*
* @param pResult	This is the object* to be returned
* @returns	False if the object* is NULL.
*************************************************************/
bool KernelSMLgSKI::ReturnResult(Connection* pConnection, ElementXML* pResponse, void const* pResult, const char* pType, bool shouldBeReleased)
{
	if (!pResult)
		return false ;

	// pResult is the object to be returned.
	// We do this by storing the pointer in our ObjectMap
	char const* pResultID = StoreObject(pResult, pType, shouldBeReleased) ;

	// and then returning a string version to the caller (by adding it to pResponse)
	AddResultID(pConnection, pResponse, pResultID) ;

	return true ;
}

/*************************************************************
* @brief	Return an integer result to the caller.
*************************************************************/
bool KernelSMLgSKI::ReturnIntResult(Connection* pConnection, ElementXML* pResponse, int result)
{
	char buffer[kMinBufferSize] ;
	Int2String(result, buffer, kMinBufferSize) ;

	pConnection->AddSimpleResultToSMLResponse(pResponse, buffer) ;

	return true ;
}

/*************************************************************
* @brief	Return a boolean result to the caller.
*************************************************************/
bool KernelSMLgSKI::ReturnBoolResult(Connection* pConnection, ElementXML* pResponse, bool result)
{
	char const* pResult = result ? sml_Names::kTrue : sml_Names::kFalse ;
	pConnection->AddSimpleResultToSMLResponse(pResponse, pResult) ;
	return true ;
}

/*************************************************************
* @brief	Return an invalid argument error to the caller.
*************************************************************/
bool KernelSMLgSKI::InvalidArg(Connection* pConnection, ElementXML* pResponse, char const* pCommandName)
{
	std::string msg = "Invalid arguments for command : " ;
	msg += pCommandName ;

	AddErrorMsg(pConnection, pResponse, msg.c_str()) ;
	
	// Return true because we've already added the error message.
	return true ;
}

/*************************************************************
* @brief	Generate an ID for an object, but don't store it.
*
* @param pObject	The object itself.
* @param pBuffer	Should be at least 20 bytes.
* @returns The ID we created for this object.  (Returns pointer to pBuffer)
*************************************************************/
inline char* KernelSMLgSKI::GenerateID(void const* pObject, char* pBuffer)
{
	// Create a hex id string
	sprintf(pBuffer, "0x%x", (int)pObject) ;

	// Return the ID
	return pBuffer ;
}

/*************************************************************
* @brief	Record an object so we can look it up later.
*
* @param pObject	The object itself.
* @param pType		A string for the type -- MUST be a static const, we just store the pointer
* @param shouldBeReleased	True if we expect client to eventually release this object.
* @returns The ID we created for this object.  Copy this to keep it.
*************************************************************/
inline char const* KernelSMLgSKI::StoreObject(void const* pObject, char const* pType, bool shouldBeReleased)
{
	// Create a hex id string
	char* pID = new char[kMaxIDLen] ;
	sprintf(pID, "0x%x", (int)pObject) ;

	// See if we've already stored this object.
	// This is important, otherwise we'd leak the id string and
	// the kernel object info (if we're using that).
	ObjectMapIter mapIter = m_ObjectMap.find(pID) ;

	if (mapIter != m_ObjectMap.end())
	{
		// Get rid of the id string we were just creating
		delete pID ;

		// Return the one that already exists
		return mapIter->first ;
	}

	// In debug mode we store a lot of extra stuff, not just the object
#ifdef USE_KERNEL_OBJECT_INFO
	m_AllocationCounter++ ;

	if (m_AllocationCounter == kStopOnAllocation)
	{
		// Set your break point here if not running in Windows.
#ifdef _WIN32
		DebugBreak() ;
#endif
	}

	KernelObjectInfo* pInfo = new KernelObjectInfo(pObject, pType, shouldBeReleased, m_AllocationCounter) ;
	pObject = pInfo ;
#endif

	// Store the object
	m_ObjectMap[pID] = (void*)pObject ;

	// Return the ID
	return pID ;
}

/*************************************************************
* @brief	Removes an object from our table.
*
* @param pID	The id for this object (e.g. "0x4545")
* @returns true if we find the object.
*************************************************************/
inline bool KernelSMLgSKI::DeleteObject(char const* pID)
{
	if (!pID)
		return false ;

	ObjectMapIter mapIter = m_ObjectMap.find((char*)pID) ;

	// This ID is not in the map
	if (mapIter == m_ObjectMap.end())
		return false ;

	char* pStr = mapIter->first ;
	delete[] pStr ;

#ifdef USE_KERNEL_OBJECT_INFO
	KernelObjectInfo* pInfo = (KernelObjectInfo*)mapIter->second ;
	delete pInfo ;
#endif

	m_ObjectMap.erase(mapIter) ;

	return true ;
}

/*************************************************************
* @brief	Looks up an object based on a string ID.
*
* @param pID		An id for this object (e.g. "0x45ad") (can be NULL, returns NULL)
* @param pType		The type of this object (can be NULL -- which means we don't care)
* @returns The object (or NULL if we don't find it in the map).
*************************************************************/
inline void* KernelSMLgSKI::LookupObject(char const* pID, char const* pType)
{
	if (pID == NULL)
		return NULL ;

	ObjectMapIter mapIter = m_ObjectMap.find((char*)pID) ;

	if (mapIter == m_ObjectMap.end())
		return NULL ;

	void* pResult = mapIter->second ;

#ifdef USE_KERNEL_OBJECT_INFO
	KernelObjectInfo* pInfo = (KernelObjectInfo*)mapIter->second ;
	pResult = (void*)pInfo->m_pObject ;

#ifdef _WIN32
	// If the types don't match, break into the debugger.
	if (pType != NULL && strcmp(pType, pInfo->m_pType) != 0)
		DebugBreak() ;
#endif

#endif

	return pResult ;
}

bool KernelSMLgSKI::ProcessCommand(char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Look up the function that handles this command
	CommandFunction pFunction = m_CommandMap[pCommandName] ;

	if (!pFunction)
	{
		// There is no handler for this command
		std::string msg = "Command " ;
		msg += pCommandName ;
		msg += " is not recognized by the kernel" ;

		AddErrorMsg(pConnection, pResponse, msg.c_str()) ;
		return false ;
	}

	// Special case for the only static gSKI function
	bool staticFunction = (pFunction == &sml::KernelSMLgSKI::CreateKernelFactory) ;
	void* pThis = NULL ;

	if (!staticFunction)
	{
		// Convert from the ID in the command to an actual object, which is
		// the "this" pointer for this call.
		char const* pThisID = pIncoming->GetArgValue(sml_Names::kParamThis) ;

		if (!pThisID)
		{
			AddErrorMsg(pConnection, pResponse, "No 'this' parameter found in this call") ;
			return false ;
		}

		// Go from id (e.g. "0x4545") to object.
		pThis = LookupObject(pThisID, NULL) ;

		if (!pThis)
		{
			// Failed to find this id in our ObjectMap.
			std::string msg = "The object with id: " ;
			msg += pThisID ;
			msg += " doesn't seem to exist." ;
			AddErrorMsg(pConnection, pResponse, msg.c_str()) ;
			return false ;
		}
	}

	// Create a blank error code
	gSKI::Error error ;
	ClearError(&error) ;

	// Call to the handler (this is a pointer to member call so it's a bit odd)
	bool result = (this->*pFunction)(pThis, pCommandName, pConnection, pIncoming, pResponse, &error) ;

	// If we return false, we report a generic error about the call.
	if (!result)
	{
		std::string msg = "The call " ;
		msg += pCommandName ;
		msg += " failed to execute correctly." ;
		if (isError(error))
		{
			msg += "gSKI error was: " ;
			msg += error.Text ;
			msg += " details: " ;
			msg += error.ExtendedMsg ;
		}

		AddErrorMsg(pConnection, pResponse, msg.c_str()) ;
		return false ;
	}

	return true ;
}

/*************************************************************
* @brief	Takes an incoming SML message and responds with
*			an appropriate response message.
*
* @param pConnection	The connection this message came in on.
* @param pIncoming		The incoming message
*************************************************************/
ElementXML* KernelSMLgSKI::ProcessIncomingSML(Connection* pConnection, ElementXML* pIncomingMsg)
{
	if (!pIncomingMsg || !pConnection)
		return NULL ;

#ifdef DEBUG
	// For debugging, it's helpful to be able to look at the incoming message as an XML string
	char* pIncomingXML = pIncomingMsg->GenerateXMLString(true) ;
#endif

	ElementXML* pResponse = pConnection->CreateSMLResponse(pIncomingMsg) ;

	// Fatal error creating the response
	if (!pResponse)
		return NULL ;

	// Analyze the message and find important tags
	AnalyzeXML msg ;
	msg.Analyze(pIncomingMsg) ;

	// Get the "name" attribute from the <command> tag
	char const* pCommandName = msg.GetCommandName() ;

	if (pCommandName)
	{
		ProcessCommand(pCommandName, pConnection, &msg, pResponse) ;
	}
	else
	{
		// The message wasn't something we recognize.
		if (!msg.GetCommandTag())
			AddErrorMsg(pConnection, pResponse, "Incoming message did not contain a <command> tag") ;
		else
			AddErrorMsg(pConnection, pResponse, "Incoming message did not contain a name attribute in the <command> tag") ;
	}

#ifdef DEBUG
	// For debugging, it's helpful to be able to look at the response as XML
	char* pResponseXML = pResponse->GenerateXMLString(true) ;

	// Set a break point on this next line if you wish to see the incoming
	// and outgoing as XML before they get deleted.
	ElementXML::DeleteString(pIncomingXML) ;
	ElementXML::DeleteString(pResponseXML) ;
#endif

	return pResponse ;
}

