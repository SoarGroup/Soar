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
char const* const sml_Names::kSoarVersionValue	= "8.6.1" ;			// Hard-coding this rather than asking kernel boosts performance
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

//for RHS output
char const* const sml_Names::kTagRHS_write	= "rhs_write" ;
char const* const sml_Names::kRHS_String	= "string" ;

// Tags defined for Trace output at each watch level:

// <trace> contains the rest.
char const* const sml_Names::kTagTrace		= "trace" ;

// <context> tag identifiers for Watch level 1
char const* const sml_Names::kTagState				= "state" ;
char const* const sml_Names::kTagOperator			= "operator" ;
char const* const sml_Names::kState_ID				= "current_state_id" ;
char const* const sml_Names::kState_Name			= "name" ;
char const* const sml_Names::kState_DecisionCycleCt	= "decision_cycle_count" ;
char const* const sml_Names::kState_ImpasseObject	= "impasse_object" ;
char const* const sml_Names::kState_ImpasseType		= "impasse_type" ;
char const* const sml_Names::kState_StackLevel		= "stack_level" ;
char const* const sml_Names::kOperator_ID			= "current_operator_id" ;
char const* const sml_Names::kOperator_Name			= "name" ;
char const* const sml_Names::kOperator_DecisionCycleCt = "decision_cycle_count" ;
char const* const sml_Names::kOperator_StackLevel	= "stack_level" ;

// <phase> tag identifiers for Watch level 2
char const* const sml_Names::kTagPhase  	= "phase" ;
char const* const sml_Names::kPhase_Name  	= "name" ;
char const* const sml_Names::kPhase_Status  	= "status" ;
char const* const sml_Names::kPhase_FiringType 	= "firing_type" ;
char const* const sml_Names::kPhaseName_Input  	= "input" ;
char const* const sml_Names::kPhaseName_Pref  	= "preference" ;
char const* const sml_Names::kPhaseName_WM  	= "workingmemory" ;
char const* const sml_Names::kPhaseName_Decision= "decision" ;
char const* const sml_Names::kPhaseName_Output 	= "output" ;
// next two are new phase names
char const* const sml_Names::kPhaseName_Propose	= "propose" ;
char const* const sml_Names::kPhaseName_Apply  	= "apply" ;
char const* const sml_Names::kPhaseName_Unknown	= "unknown" ;
char const* const sml_Names::kPhaseStatus_Begin	= "begin" ;
char const* const sml_Names::kPhaseStatus_End	= "end" ;
char const* const sml_Names::kPhaseFiringType_IE= "IE" ;
char const* const sml_Names::kPhaseFiringType_PE= "PE" ;

// <prod-firing> tag identifiers for Watch level 3
char const* const sml_Names::kTagProduction		= "production" ;
char const* const sml_Names::kProduction_Name  	= "prodname" ;
char const* const sml_Names::kTagProduction_Firing  	= "firing_production" ;
char const* const sml_Names::kTagProduction_Retracting  = "retracting_production" ;

// <wme> tag identifiers, also for Watch level 4
char const* const sml_Names::kTagWME		= "wme" ;
char const* const sml_Names::kWME_TimeTag	= "tag" ;
char const* const sml_Names::kWME_Id		= "id" ;
char const* const sml_Names::kWME_Attribute	= "attr" ;
char const* const sml_Names::kWME_Value		= "value" ;
char const* const sml_Names::kWME_ValueType	= "type" ;
char const* const sml_Names::kWME_Preference= "preference";
char const* const sml_Names::kWME_Action	= "action" ;
// kjc question:  should the next entry be kWMEAction_Add?
char const* const sml_Names::kValueAdd		= "add" ;
char const* const sml_Names::kValueRemove	= "remove" ;
char const* const sml_Names::kTagWMERemove	= "removing_wme" ;
char const* const sml_Names::kTagWMEAdd 	= "adding_wme" ;

// <preference> tag identifiers, also Watch level 5
char const* const sml_Names::kTagPreference		= "preference" ;
char const* const sml_Names::kPreference_Type	= "pref_type" ;

char const* const sml_Names::kTagWarning		= "warning" ;
// Tag warning has attribute kTypeString

// XML function types for XML output event
char const* const sml_Names::kFunctionBeginTag		= "begintag";
char const* const sml_Names::kFunctionEndTag		= "endtag";
char const* const sml_Names::kFunctionAddAttribute	= "addattribute";

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
char const* const sml_Names::kParamVersionMicro		= "versionmicro";
char const* const sml_Names::kParamWaitSNC			= "waitsnc";
char const* const sml_Names::kParamFunction			= "function" ;
char const* const sml_Names::kParamChunkNamePrefix	= "chunknameprefix" ;
char const* const sml_Names::kParamChunkCount		= "chunkcount" ;
char const* const sml_Names::kParamChunkLongFormat	= "chunklongformat" ;
// Parameter names for stats command
char const* const sml_Names::kParamStatsProductionCountDefault				= "statsproductioncountdefault" ;
char const* const sml_Names::kParamStatsProductionCountUser					= "statsproductioncountuser" ;
char const* const sml_Names::kParamStatsProductionCountChunk				= "statsproductioncountchunk" ;
char const* const sml_Names::kParamStatsProductionCountJustification		= "statsproductioncountjustification" ;
char const* const sml_Names::kParamStatsCycleCountDecision					= "statscyclecountdecision" ;
char const* const sml_Names::kParamStatsCycleCountElaboration				= "statscyclecountelaboration" ;
char const* const sml_Names::kParamStatsProductionFiringCount				= "statsproductionfiringcount" ;
char const* const sml_Names::kParamStatsWmeCountAddition					= "statswmecountaddition" ;
char const* const sml_Names::kParamStatsWmeCountRemoval						= "statswmecountremoval" ;
char const* const sml_Names::kParamStatsWmeCount							= "statswmecount" ;
char const* const sml_Names::kParamStatsWmeCountAverage						= "statswmecountsverage" ;
char const* const sml_Names::kParamStatsWmeCountMax							= "statswecountmax" ;
char const* const sml_Names::kParamStatsKernelCPUTime						= "statskernelcputime" ;
char const* const sml_Names::kParamStatsTotalCPUTime						= "statstotalcputime" ;

char const* const sml_Names::kParamStatsPhaseTimeInputPhase 				= "statsphasetimeinputphase" ; 
char const* const sml_Names::kParamStatsPhaseTimeProposePhase 				= "statsphasetimeproposephase" ; 
char const* const sml_Names::kParamStatsPhaseTimeDecisionPhase 				= "statsphasetimedecisionphase" ; 
char const* const sml_Names::kParamStatsPhaseTimeApplyPhase 				= "statsphasetimeapplyphase" ; 
char const* const sml_Names::kParamStatsPhaseTimeOutputPhase				= "statsphasetimeoutputphase" ; 
char const* const sml_Names::kParamStatsPhaseTimePreferencePhase 			= "statsphasetimepreferencephase" ; 
char const* const sml_Names::kParamStatsPhaseTimeWorkingMemoryPhase 		= "statsphasetimeworkingmemoryphase" ; 

char const* const sml_Names::kParamStatsMonitorTimeInputPhase 				= "statsmonitortimeinputphase" ; 
char const* const sml_Names::kParamStatsMonitorTimeProposePhase 			= "statsmonitortimeproposephase" ; 
char const* const sml_Names::kParamStatsMonitorTimeDecisionPhase			= "statsmonitortimedecisionphase" ; 
char const* const sml_Names::kParamStatsMonitorTimeApplyPhase				= "statsmonitortimeapplyphase" ; 
char const* const sml_Names::kParamStatsMonitorTimeOutputPhase 				= "statsmonitortimeoutputphase" ; 
char const* const sml_Names::kParamStatsMonitorTimePreferencePhase 			= "statsmonitortimepreferencephase" ;
char const* const sml_Names::kParamStatsMonitorTimeWorkingMemoryPhase 		= "statsmonitortimeworkingmemoryphase" ; 

char const* const sml_Names::kParamStatsInputFunctionTime 					= "statsinputfunctiontime" ;
char const* const sml_Names::kParamStatsOutputFunctionTime 					= "statsoutputfunctiontime" ;

char const* const sml_Names::kParamStatsMatchTimeInputPhase					= "statsmatchtimeinputphase" ;
char const* const sml_Names::kParamStatsMatchTimePreferencePhase			= "statsmatchtimepreferencephase" ;
char const* const sml_Names::kParamStatsMatchTimeWorkingMemoryPhase			= "statsmatchtimeworkingmemoryphase" ;
char const* const sml_Names::kParamStatsMatchTimeOutputPhase				= "statsmatchtimeoutputphase" ;
char const* const sml_Names::kParamStatsMatchTimeDecisionPhase				= "statsmatchtimedecisionphase" ;
char const* const sml_Names::kParamStatsMatchTimeProposePhase				= "statsmatchtimedecisionphase" ;
char const* const sml_Names::kParamStatsMatchTimeApplyPhase				= "statsmatchtimedecisionphase" ;

char const* const sml_Names::kParamStatsOwnershipTimeInputPhase				= "statsownershiptimeinputphase" ;
char const* const sml_Names::kParamStatsOwnershipTimePreferencePhase		= "statsownershiptimepreferencephase" ;
char const* const sml_Names::kParamStatsOwnershipTimeWorkingMemoryPhase		= "statsownershiptimeworkingmemoryphase" ;
char const* const sml_Names::kParamStatsOwnershipTimeOutputPhase			= "statsownershiptimeoutputphase" ;
char const* const sml_Names::kParamStatsOwnershipTimeDecisionPhase			= "statsownershiptimedecisionphase" ;
char const* const sml_Names::kParamStatsOwnershipTimeProposePhase			= "statsownershiptimedecisionphase" ;
char const* const sml_Names::kParamStatsOwnershipTimeApplyPhase			= "statsownershiptimedecisionphase" ;

char const* const sml_Names::kParamStatsChunkingTimeInputPhase				= "statschunkingtimeinputphase" ;
char const* const sml_Names::kParamStatsChunkingTimePreferencePhase			= "statschunkingtimepreferencephase" ;
char const* const sml_Names::kParamStatsChunkingTimeWorkingMemoryPhase		= "statschunkingtimeworkingmemoryphase" ;
char const* const sml_Names::kParamStatsChunkingTimeOutputPhase				= "statschunkingtimeoutputphase" ;
char const* const sml_Names::kParamStatsChunkingTimeDecisionPhase			= "statschunkingtimedecisionphase" ;
char const* const sml_Names::kParamStatsChunkingTimeProposePhase			= "statschunkingtimedecisionphase" ;
char const* const sml_Names::kParamStatsChunkingTimeApplyPhase			= "statschunkingtimedecisionphase" ;

char const* const sml_Names::kParamStatsMemoryUsageMiscellaneous			= "statsmemoryusagemiscellaneous" ;
char const* const sml_Names::kParamStatsMemoryUsageHash						= "statsmemoryusagehash" ;
char const* const sml_Names::kParamStatsMemoryUsageString					= "statsmemoryusagestring" ;
char const* const sml_Names::kParamStatsMemoryUsagePool						= "statsmemoryusagepool" ;
char const* const sml_Names::kParamStatsMemoryUsageStatsOverhead			= "statsmemoryusagestatsoverhead" ;
// Parameter names for watch command
char const* const sml_Names::kParamWatchDecisions					= "watchdecisions";
char const* const sml_Names::kParamWatchPhases						= "watchphases";
char const* const sml_Names::kParamWatchProductionDefault			= "watchproductiondefault";
char const* const sml_Names::kParamWatchProductionUser				= "watchproductionuser";
char const* const sml_Names::kParamWatchProductionChunks			= "watchproductionchunks";
char const* const sml_Names::kParamWatchProductionJustifications	= "watchproductionjustifications";
char const* const sml_Names::kParamWatchWMEDetail					= "watchwmedetail";
char const* const sml_Names::kParamWatchWorkingMemoryChanges		= "watchworkingmemorychanges";
char const* const sml_Names::kParamWatchPreferences					= "watchpreferences";
char const* const sml_Names::kParamWatchLearning					= "watchlearning";
char const* const sml_Names::kParamWatchBacktracing					= "watchbacktracing";
char const* const sml_Names::kParamWatchIndifferentSelection		= "watchindifferentselection";

// Values (these are not case sensitive unlike the rest)
char const* const sml_Names::kTrue	= "true" ;
char const* const sml_Names::kFalse	= "false" ;

// sgio style commands
char const* const sml_Names::kCommand_CreateAgent			= "create_agent" ;
char const* const sml_Names::kCommand_DestroyAgent			= "destroy_agent" ;
char const* const sml_Names::kCommand_GetAgentList			= "get_agent_list" ;
char const* const sml_Names::kCommand_LoadProductions		= "load_productions" ;
char const* const sml_Names::kCommand_GetInputLink			= "get_input_link" ;
char const* const sml_Names::kCommand_GetOutputLink			= "get_output_link" ;
char const* const sml_Names::kCommand_Run					= "run" ;
char const* const sml_Names::kCommand_Input					= "input" ;
char const* const sml_Names::kCommand_Output				= "output" ;
char const* const sml_Names::kCommand_StopOnOutput			= "stop_on_output" ;
char const* const sml_Names::kCommand_RegisterForEvent		= "register_for_event" ;
char const* const sml_Names::kCommand_UnregisterForEvent	= "unregister_for_event" ;
char const* const sml_Names::kCommand_Event					= "event" ;	// Just passes event id
char const* const sml_Names::kCommand_FireEvent				= "fire_event" ;
char const* const sml_Names::kCommand_SuppressEvent			= "suppress_event" ;
char const* const sml_Names::kCommand_CheckForIncomingCommands = "check_for_incoming_commands" ;
char const* const sml_Names::kCommand_SetInterruptCheckRate	= "set_interrupt_check_rate" ;

// command line interface
char const* const sml_Names::kCommand_CommandLine		 = "cmdline" ;
char const* const sml_Names::kCommand_ExpandCommandLine  = "expandcmd" ;
