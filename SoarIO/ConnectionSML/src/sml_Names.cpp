/////////////////////////////////////////////////////////////////
//sml_Names
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : July 2004
//
// The names (identifiers) we use in SML.
//
/////////////////////////////////////////////////////////////////

#include "sml_Names.h"

using namespace sml ;

// <sml> tag identifiers
char const* sml_Names::kTagSML			= "sml" ;
char const* sml_Names::kID				= "id" ;
char const* sml_Names::kAck				= "ack" ;
char const* sml_Names::kDocType			= "doctype" ;
char const* sml_Names::kDocType_Call	= "call" ;
char const* sml_Names::kDocType_Response= "response" ;
char const* sml_Names::kDocType_Notify	= "notify" ;
char const* sml_Names::kSoarVersion		= "soarversion" ;
char const* sml_Names::kSMLVersion		= "smlversion" ;
char const* sml_Names::kSMLVersionValue	= "8.6.0" ;
char const* sml_Names::kSoarVersionValue= "1.0" ;

// <command> tag identifiers
char const* sml_Names::kTagCommand		= "command" ;
char const* sml_Names::kCommandName		= "name" ;
char const* sml_Names::kCommandOutput	= "output" ;
char const* sml_Names::kRawOutput		= "raw" ;
char const* sml_Names::kStructuredOutput = "structured" ;

// <arg> tag identifiers
char const* sml_Names::kTagArg			= "arg" ;
char const* sml_Names::kArgParam		= "param" ;
char const* sml_Names::kArgType			= "type" ;

// <error> tag identifiers
char const* sml_Names::kTagError		= "error" ;
char const* sml_Names::kErrorCode		= "code" ;

// <result> tag identifiers
char const* sml_Names::kTagResult		= "result" ;

// Types
char const* sml_Names::kTypeString	= "string" ;
char const* sml_Names::kTypeInt		= "int" ;
char const* sml_Names::kTypeDouble	= "double" ;
char const* sml_Names::kTypeChar	= "char" ;
char const* sml_Names::kTypeBoolean = "boolean" ;
char const* sml_Names::kTypeID		= "id" ;
char const* sml_Names::kTypeVariable = "variable" ;

// Parameter names
char const* sml_Names::kParamAgent		= "agent" ;
char const* sml_Names::kParamKernel		= "kernel" ;
char const* sml_Names::kParamThis		= "this" ;
char const* sml_Names::kParamName		= "name" ;
char const* sml_Names::kParamFilename	= "filename" ;
char const* sml_Names::kParamLearning	= "learning" ;
char const* sml_Names::kParamOSupportMode= "osupportmode" ;
char const* sml_Names::kParamValue		= "value" ;
char const* sml_Names::kParamWme		= "wme" ;
char const* sml_Names::kParamWmeObject	= "wmeobject" ;
char const* sml_Names::kParamAttribute	= "att" ;
char const* sml_Names::kParamLength		= "length" ;
char const* sml_Names::kParamCount		= "count" ;
char const* sml_Names::kParamThread		= "thread" ;
char const* sml_Names::kParamProcess	= "process" ;
char const* sml_Names::kParamLocation	= "location" ;
char const* sml_Names::kParamLogLocation= "loglocation" ;
char const* sml_Names::kParamLogLevel	= "loglevel" ;
char const* sml_Names::kParamInputProducer	= "inputproducer" ;
char const* sml_Names::kParamWorkingMemory	= "workingmemory" ;
char const* sml_Names::kParamOutputProcessor = "outputprocessor" ;
char const* sml_Names::kParamAttributePath   = "attpath" ;
char const* sml_Names::kParamUpdate		= "update" ;

// Values (these are not case sensitive unlike the rest)
char const* sml_Names::kTrue	= "true" ;
char const* sml_Names::kFalse	= "false" ;

// sgio style commands
char const* sml_Names::kCommand_CreateAgent		= "create_agent" ;
char const* sml_Names::kCommand_LoadProductions = "load_productions" ;
char const* sml_Names::kCommand_GetInputLink	= "get_input_link" ;
char const* sml_Names::kCommand_GetOutputLink	= "get_output_link" ;
char const* sml_Names::kCommand_Run				= "run" ;
char const* sml_Names::kCommand_Input			= "input" ;

// gSKI commands
char const* sml_Names::kgSKI_CreateKernelFactory			= "gski::createkernelfactory" ;
char const* sml_Names::kgSKI_IRelease_Release				= "gski::irelease::release" ;
char const* sml_Names::kgSKI_IKernelFactory_Create			= "gski::ikernelfactory::create" ;
char const* sml_Names::kgSKI_IKernelFactory_DestroyKernel	= "gski::ikernelfactory::destroykernel" ;
char const* sml_Names::kgSKI_IKernel_GetAgentManager		= "gski::ikernel::getagentmanager" ;
char const* sml_Names::kgSKI_IInputLink_GetInputLinkMemory	= "gski::iinputlink::getinputlinkmemory" ;
char const* sml_Names::kgSKI_IInputLink_GetRootObject		= "gski::iinputlink::getrootobject" ;
char const* sml_Names::kgSKI_IAgent_GetInputLink			= "gski::iagent::getinputlink" ;
char const* sml_Names::kgSKI_IAgentManager_AddAgent			= "gski::iagentmanager::addagent" ;
char const* sml_Names::kgSKI_IAgentManager_RemoveAgent		= "gski::iagentmanager::removeagent" ;
char const* sml_Names::kgSKI_IWme_GetValue					= "gski::iwme::getvalue" ;
char const* sml_Names::kgSKI_ISymbol_GetObject				= "gski::isymbol::getobject" ;
char const* sml_Names::kgSKI_ISymbol_GetString				= "gski::isymbol::getstring" ;
char const* sml_Names::kgSKI_ISymbol_GetInt					= "gski::isymbol::getint" ;
char const* sml_Names::kgSKI_IWorkingMemory_ReplaceStringWme= "gski::iworkingmemory::replacestringwme" ;
char const* sml_Names::kgSKI_IWorkingMemory_ReplaceIntWme	= "gski::iworkingmemory::replaceintwme" ;
char const* sml_Names::kgSKI_IWorkingMemory_ReplaceDoubleWme= "gski::iworkingmemory::replacedoublewme" ;
char const* sml_Names::kgSKI_IWorkingMemory_AddWmeNewObject = "gski::iworkingmemory::addwmenewobject" ;
char const* sml_Names::kgSKI_IWorkingMemory_AddWmeString	= "gski::iworkingmemory::addwmestring" ;
char const* sml_Names::kgSKI_IWorkingMemory_AddWmeInt		= "gski::iworkingmemory::addwmeint" ;
char const* sml_Names::kgSKI_IWorkingMemory_RemoveObject	= "gski::iworkingmemory::removeobject" ;
char const* sml_Names::kgSKI_IWorkingMemory_GetAgent		= "gski::iworkingmemory::getagent" ;
char const* sml_Names::kgSKI_IAgent_RunInClientThread		= "gski::iagent::runinclientthread" ;
char const* sml_Names::kgSKI_IWMObject_GetWMEs				= "gski::iwmobject::getwmes" ;
char const* sml_Names::kgSKI_IInputLink_AddInputProducer	= "gski::iinputlink::addinputproducer" ;
char const* sml_Names::kgSKI_IInputProducer_Update			= "gski::iinputproducer::update" ;
char const* sml_Names::kgSKI_IOutputProcessor_ProcessOutput	= "gski::ioutputprocessor::processoutput" ;
char const* sml_Names::kgSKI_IAgent_GetOutputLink			= "gski::iagent::getoutputlink" ;
char const* sml_Names::kgSKI_IOutputLink_AddOutputProcessor = "gski::ioutputlink::addoutputprocessor" ;
char const* sml_Names::kgSKI_IOutputLink_SetAutomaticUpdate = "gski::ioutput::setautomaticupdate" ;
char const* sml_Names::kgSKI_IOutputLink_GetRootObject		= "gski::ioutputlink::getrootobject" ;
char const* sml_Names::kgSKI_IOutputLink_GetOutputLinkMemory= "gski::ioutputlink::getoutputlinkmemory" ;

// These should work for all pointer iterators (which is almost all iterators)
char const* sml_Names::kgSKI_IIterator_Pointer_IsValid		= "gski::iiterator_pointer_isvalid" ;
char const* sml_Names::kgSKI_IIterator_Pointer_GetVal		= "gski::iiterator_pointer_getval" ;
char const* sml_Names::kgSKI_IIterator_Pointer_Next			= "gski::iiterator_pointer_next" ;

