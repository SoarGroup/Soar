/////////////////////////////////////////////////////////////////
//sml_Names
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : July 2004
//
// The names (identifiers) we use in SML.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_NAMESH
#define SML_NAMESH

namespace sml
{

class sml_Names
{
public:
	// <sml> tag identifiers
	static char const* kTagSML ;
	static char const* kID ;
	static char const* kAck ;
	static char const* kDocType ;
	static char const* kDocType_Call ;
	static char const* kDocType_Response ;
	static char const* kDocType_Notify ;
	static char const* kSoarVersion ;
	static char const* kSMLVersion ;
	static char const* kSMLVersionValue ;
	static char const* kSoarVersionValue ;

	// <command> tag identifiers
	static char const* kTagCommand ;
	static char const* kCommandName ;
	static char const* kCommandOutput ;
	static char const* kRawOutput ;
	static char const* kStructuredOutput ;

	// <arg> tag identifiers
	static char const* kTagArg ;
	static char const* kArgParam ;
	static char const* kArgType ;

	// <error> tag identifiers
	static char const* kTagError ;
	static char const* kErrorCode ;

	// <result> tag identifiers
	static char const* kTagResult ;

	// input values (for update param)
	static char const* kValueDelta ;
	static char const* kValueFull ;

	// <wme> tag identifiers
	static char const* kTagWME ;
	static char const* kWME_TimeTag ;
	static char const* kWME_Id ;
	static char const* kWME_Attribute ;
	static char const* kWME_Value ;
	static char const* kWME_ValueType ;
	static char const* kWME_Action ;
	static char const* kValueAdd	;
	static char const* kValueRemove ;

	// Types
	static char const* kTypeString ;
	static char const* kTypeInt ;
	static char const* kTypeDouble ;
	static char const* kTypeChar ;
	static char const* kTypeBoolean ;
	static char const* kTypeID ;
	static char const* kTypeVariable ;

	// Params
	static char const* kParamAgent ;
	static char const* kParamKernel ;
	static char const* kParamThis ;
	static char const* kParamName ;
	static char const* kParamFilename ;
	static char const* kParamLearning ;
	static char const* kParamOSupportMode ;
	static char const* kParamValue ;
	static char const* kParamWme ;
	static char const* kParamWmeObject ;
	static char const* kParamAttribute ;
	static char const* kParamCount ;
	static char const* kParamLength ;
	static char const* kParamThread	;
	static char const* kParamProcess ;
	static char const* kParamLine ;
	static char const* kParamLocation ;
	static char const* kParamLogLocation ;
	static char const* kParamLogLevel ;
	static char const* kParamInputProducer ;
	static char const* kParamOutputProcessor ;
	static char const* kParamWorkingMemory ;
	static char const* kParamAttributePath ;
	static char const* kParamUpdate ;

	// Values (these are not case sensitive unlike the rest)
	static char const* kTrue ;
	static char const* kFalse ;

	// sgio style commands
	static char const* kCommand_CreateAgent ;
	static char const* kCommand_LoadProductions ;
	static char const* kCommand_GetInputLink ;
	static char const* kCommand_GetOutputLink ;
	static char const* kCommand_Run ;
	static char const* kCommand_Input ;

	// Command line interface
	static char const* kCommand_CommandLine ;

	// gSKI style commands
	static char const* kgSKI_CreateKernelFactory ;
	static char const* kgSKI_IRelease_Release ;
	static char const* kgSKI_IKernelFactory_Create ;
	static char const* kgSKI_IKernelFactory_DestroyKernel ;
	static char const* kgSKI_IKernel_GetAgentManager ;
	static char const* kgSKI_IInputLink_GetInputLinkMemory ;
	static char const* kgSKI_IAgent_GetInputLink ;
	static char const* kgSKI_IAgentManager_AddAgent	;
	static char const* kgSKI_IAgentManager_RemoveAgent ;
	static char const* kgSKI_IWme_GetValue ;
	static char const* kgSKI_ISymbol_GetObject ;
	static char const* kgSKI_ISymbol_GetString ;
	static char const* kgSKI_ISymbol_GetInt	;
	static char const* kgSKI_IWorkingMemory_ReplaceStringWme ;
	static char const* kgSKI_IWorkingMemory_ReplaceIntWme ;
	static char const* kgSKI_IWorkingMemory_ReplaceDoubleWme ;
	static char const* kgSKI_IWorkingMemory_AddWmeNewObject ;
	static char const* kgSKI_IWorkingMemory_AddWmeString ;
	static char const* kgSKI_IWorkingMemory_AddWmeInt ;
	static char const* kgSKI_IWorkingMemory_AddWmeDouble ;
	static char const* kgSKI_IWorkingMemory_RemoveObject ;
	static char const* kgSKI_IWorkingMemory_GetAgent ;
	static char const* kgSKI_IAgent_RunInClientThread ;
	static char const* kgSKI_IWMObject_GetWMEs ;
	static char const* kgSKI_IInputLink_AddInputProducer ;
	static char const* kgSKI_IInputLink_GetRootObject ;
	static char const* kgSKI_IInputProducer_Update ;
	static char const* kgSKI_IOutputProcessor_ProcessOutput ;
	static char const* kgSKI_IAgent_GetOutputLink ;
	static char const* kgSKI_IOutputLink_AddOutputProcessor ;
	static char const* kgSKI_IOutputLink_SetAutomaticUpdate ;
	static char const* kgSKI_IOutputLink_GetRootObject ;
	static char const* kgSKI_IOutputLink_GetOutputLinkMemory ;

	// These should work for all pointer iterators (which is almost all iterators)
	static char const* kgSKI_IIterator_Pointer_IsValid ;
	static char const* kgSKI_IIterator_Pointer_GetVal ;
	static char const* kgSKI_IIterator_Pointer_Next ;
} ;

}

#endif // SML_NAMESH