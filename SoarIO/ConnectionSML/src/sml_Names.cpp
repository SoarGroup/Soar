#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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
char const* const sml_Names::kTagSML			= "sml" ;
char const* const sml_Names::kID				= "id" ;
char const* const sml_Names::kAck				= "ack" ;
char const* const sml_Names::kDocType			= "doctype" ;
char const* const sml_Names::kDocType_Call		= "call" ;
char const* const sml_Names::kDocType_Response	= "response" ;
char const* const sml_Names::kDocType_Notify	= "notify" ;
char const* const sml_Names::kSoarVersion		= "soarversion" ;
char const* const sml_Names::kSMLVersion		= "smlversion" ;
char const* const sml_Names::kSMLVersionValue	= "1.0" ;
char const* const sml_Names::kSoarVersionValue	= "8.6.0" ;
char const* const sml_Names::kOutputLinkName	= "output-link" ;

// <command> tag identifiers
char const* const sml_Names::kTagCommand		= "command" ;
char const* const sml_Names::kCommandName		= "name" ;
char const* const sml_Names::kCommandOutput		= "output" ;
char const* const sml_Names::kRawOutput			= "raw" ;
char const* const sml_Names::kStructuredOutput  = "structured" ;

// <arg> tag identifiers
char const* const sml_Names::kTagArg			= "arg" ;
char const* const sml_Names::kArgParam			= "param" ;
char const* const sml_Names::kArgType			= "type" ;

// <error> tag identifiers
char const* const sml_Names::kTagError		= "error" ;
char const* const sml_Names::kErrorCode		= "code" ;

// <name> tag identifiers
char const* const sml_Names::kTagName		= "name" ;

// <result> tag identifiers
char const* const sml_Names::kTagResult		= "result" ;

// input values (for update param)
char const* const sml_Names::kValueDelta	= "delta" ;
char const* const sml_Names::kValueFull		= "full" ;

// <wme> tag identifiers
char const* const sml_Names::kTagWME		= "wme" ;
char const* const sml_Names::kWME_TimeTag	= "tag" ;
char const* const sml_Names::kWME_Id		= "id" ;
char const* const sml_Names::kWME_Attribute	= "att" ;
char const* const sml_Names::kWME_Value		= "value" ;
char const* const sml_Names::kWME_ValueType	= "type" ;
char const* const sml_Names::kWME_Action	= "action" ;
char const* const sml_Names::kValueAdd		= "add" ;
char const* const sml_Names::kValueRemove	= "remove" ;

// Types
char const* const sml_Names::kTypeString	= "string" ;
char const* const sml_Names::kTypeInt		= "int" ;
char const* const sml_Names::kTypeDouble	= "double" ;
char const* const sml_Names::kTypeChar		= "char" ;
char const* const sml_Names::kTypeBoolean	= "boolean" ;
char const* const sml_Names::kTypeID		= "id" ;
char const* const sml_Names::kTypeVariable	= "variable" ;

// Parameter names
char const* const sml_Names::kParamAgent			= "agent" ;
char const* const sml_Names::kParamKernel			= "kernel" ;
char const* const sml_Names::kParamThis				= "this" ;
char const* const sml_Names::kParamName				= "name" ;
char const* const sml_Names::kParamFilename			= "filename" ;
char const* const sml_Names::kParamLearning			= "learning" ;
char const* const sml_Names::kParamOSupportMode		= "osupportmode" ;
char const* const sml_Names::kParamValue			= "value" ;
char const* const sml_Names::kParamWme				= "wme" ;
char const* const sml_Names::kParamWmeObject		= "wmeobject" ;
char const* const sml_Names::kParamAttribute		= "att" ;
char const* const sml_Names::kParamLength			= "length" ;
char const* const sml_Names::kParamCount			= "count" ;
char const* const sml_Names::kParamThread			= "thread" ;
char const* const sml_Names::kParamProcess			= "process" ;
char const* const sml_Names::kParamLine				= "line" ;
char const* const sml_Names::kParamLocation			= "location" ;
char const* const sml_Names::kParamLogLocation		= "loglocation" ;
char const* const sml_Names::kParamLogLevel			= "loglevel" ;
char const* const sml_Names::kParamInputProducer	= "inputproducer" ;
char const* const sml_Names::kParamWorkingMemory	= "workingmemory" ;
char const* const sml_Names::kParamOutputProcessor	= "outputprocessor" ;
char const* const sml_Names::kParamAttributePath	= "attpath" ;
char const* const sml_Names::kParamUpdate			= "update" ;
char const* const sml_Names::kParamLearnSetting		= "learnsetting";
char const* const sml_Names::kParamLearnOnlySetting = "learnonlysetting";
char const* const sml_Names::kParamLearnExceptSetting = "learnexceptsetting";
char const* const sml_Names::kParamLearnAllLevelsSetting = "learnalllevelssetting";
char const* const sml_Names::kParamLearnForceLearnStates = "learnforcelearnstates";
char const* const sml_Names::kParamLearnDontLearnStates = "learndontlearnstates";
char const* const sml_Names::kParamLogSetting		= "log_setting";
char const* const sml_Names::kParamDirectory		= "directory";
char const* const sml_Names::kParamSeconds			= "seconds";
char const* const sml_Names::kParamWarningsSetting	= "warnings_setting";
char const* const sml_Names::kParamEventID			= "eventid" ;
char const* const sml_Names::kParamPhase			= "phase" ;
char const* const sml_Names::kParamInstance			= "instance" ;
char const* const sml_Names::kParamTimers			= "timers";
char const* const sml_Names::kParamMessage			= "message";
char const* const sml_Names::kParamAlias			= "alias";
char const* const sml_Names::kParamAliasedCommand		= "aliasedcommand";
char const* const sml_Names::kParamIndifferentSelectionMode = "indifferentselectionmode";
char const* const sml_Names::kParamNumericIndifferentMode	= "numericindifferentmode";
char const* const sml_Names::kParamRunResult		= "runresult";
char const* const sml_Names::kParamVersionMajor		= "versionmajor";
char const* const sml_Names::kParamVersionMinor		= "versionminor";
char const* const sml_Names::kParamWaitSNC			= "waitsnc";
char const* const sml_Names::kParamFunction			= "function" ;

// Values (these are not case sensitive unlike the rest)
char const* const sml_Names::kTrue	= "true" ;
char const* const sml_Names::kFalse	= "false" ;

// sgio style commands
char const* const sml_Names::kCommand_CreateAgent	= "create_agent" ;
char const* const sml_Names::kCommand_DestroyAgent	= "destroy_agent" ;
char const* const sml_Names::kCommand_GetAgentList	= "get_agent_list" ;
char const* const sml_Names::kCommand_LoadProductions = "load_productions" ;
char const* const sml_Names::kCommand_GetInputLink	= "get_input_link" ;
char const* const sml_Names::kCommand_GetOutputLink	= "get_output_link" ;
char const* const sml_Names::kCommand_Run			= "run" ;
char const* const sml_Names::kCommand_Input			= "input" ;
char const* const sml_Names::kCommand_Output		= "output" ;
char const* const sml_Names::kCommand_CheckForIncomingCommands = "check_for_incoming_commands" ;
char const* const sml_Names::kCommand_StopOnOutput		 = "stop_on_output" ;
char const* const sml_Names::kCommand_RegisterForEvent	 = "register_for_event" ;
char const* const sml_Names::kCommand_UnregisterForEvent = "unregister_for_event" ;
char const* const sml_Names::kCommand_Event				 = "event" ;	// Just passes event id

// command line interface
char const* const sml_Names::kCommand_CommandLine  = "cmdline" ;
