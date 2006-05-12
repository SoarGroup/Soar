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
char const* const sml_Names::kSMLVersionValue	= "1.1.0" ;
char const* const sml_Names::kOutputLinkName	= "output-link" ;

// Other places to change when increasing this value:
// 1) Debugger file Doc/Document.java -- kVersion string
// 2) Kernel's kernel.h -- MAJOR_VERSION_NUMBER, MINOR_VERSION_NUMBER, MICRO_VERSION_NUMBER
char const* const sml_Names::kSoarVersionValue	= "8.6.2" ;			// Hard-coding this rather than asking kernel boosts performance

// <command> tag identifiers
char const* const sml_Names::kTagCommand		= "command" ;
char const* const sml_Names::kCommandName		= "name" ;
char const* const sml_Names::kCommandOutput		= "output" ;
char const* const sml_Names::kRawOutput			= "raw" ;
char const* const sml_Names::kStructuredOutput  = "structured" ;

// <connection> tag identifiers
char const* const sml_Names::kTagConnection		= "connection" ;
char const* const sml_Names::kConnectionId		= "id" ;
char const* const sml_Names::kConnectionName	= "name" ;
char const* const sml_Names::kConnectionStatus	= "status" ;
char const* const sml_Names::kAgentStatus	    = "agent-status" ;
char const* const sml_Names::kStatusCreated		= "created" ;	// Initial status -- simply means connection exists
char const* const sml_Names::kStatusNotReady	= "not-ready" ;	// Connection not ready (work needs to be done still)
char const* const sml_Names::kStatusReady		= "ready" ;		// Connection ready (registered for events etc.)
char const* const sml_Names::kStatusClosing		= "closing" ;	// Connection about to shut down

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

// next are new phase names
char const* const sml_Names::kPhaseName_Propose	= "propose" ;
char const* const sml_Names::kPhaseName_Apply  	= "apply" ;
char const* const sml_Names::kPhaseName_Unknown	= "unknown" ;
char const* const sml_Names::kPhaseStatus_Begin	= "begin" ;
char const* const sml_Names::kPhaseStatus_End	= "end" ;
char const* const sml_Names::kPhaseFiringType_IE= "IE" ;
char const* const sml_Names::kPhaseFiringType_PE= "PE" ;

char const* const sml_Names::kTagSubphase						= "subphase";
char const* const sml_Names::kSubphaseName_FiringProductions		= "firingprods";
char const* const sml_Names::kSubphaseName_ChangingWorkingMemory	= "changingwm";

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
char const* const sml_Names::kOSupported		= "o_supported" ;
char const* const sml_Names::kReferent			= "referent" ;

char const* const sml_Names::kTagWarning		= "warning" ;
// Tag warning has attribute kTypeString

// learning stuff
char const* const sml_Names::kTagLearning	= "learning" ;
// Tag learning has attribute kTypeString

// filter support
char const* const sml_Names::kTagFilter			= "filter" ;
char const* const sml_Names::kFilterCommand		= "command" ;
char const* const sml_Names::kFilterError		= "error" ;
char const* const sml_Names::kFilterOutput		= "output" ;
char const* const sml_Names::kFilterName		= "_sml::filter" ;	// This name goes into the RHS function name space, so want it to be unlikely to be used by clients
char const* const sml_Names::kParamNoFiltering	= "nofilter" ;

//production printing
char const* const sml_Names::kTagConditions                 	= "conditions" ;
char const* const sml_Names::kTagConjunctive_Negation_Condition	= "conjunctive-negation-condition" ;
char const* const sml_Names::kTagCondition	                    = "condition" ;
char const* const sml_Names::kTagActions	                    = "actions" ;
char const* const sml_Names::kTagAction 	                    = "action" ;
char const* const sml_Names::kProductionDocumentation           = "documentation" ;
char const* const sml_Names::kProductionType                    = "type" ;
char const* const sml_Names::kProductionTypeDefault             = ":default" ;
char const* const sml_Names::kProductionTypeChunk               = ":chunk" ;
char const* const sml_Names::kProductionTypeJustification       = ":justification ;# not reloadable" ;
char const* const sml_Names::kProductionDeclaredSupport         = "declared-support" ;
char const* const sml_Names::kProductionDeclaredOSupport        = ":o-support" ;
char const* const sml_Names::kProductionDeclaredISupport        = ":i-support" ;
char const* const sml_Names::kConditionId                       = "id" ;
char const* const sml_Names::kConditionTest                     = "test";
char const* const sml_Names::kConditionTestState                = "state";
char const* const sml_Names::kConditionTestImpasse              = "impasse";
char const* const sml_Names::kCondition                         = "condition" ;
char const* const sml_Names::kAction                            = "action" ;
char const* const sml_Names::kActionFunction                    = "function" ;
char const* const sml_Names::kActionId                          = "id" ;

//backtrace stuff
char const* const sml_Names::kTagBacktrace              = "backtrace" ;
char const* const sml_Names::kTagGrounds                = "grounds" ;
char const* const sml_Names::kTagPotentials             = "potentials" ;
char const* const sml_Names::kTagLocals                 = "locals" ;
char const* const sml_Names::kTagLocal                  = "local";
char const* const sml_Names::kTagBacktraceResult        = "result";
char const* const sml_Names::kTagProhibitPreference     = "prohibit-preference";
char const* const sml_Names::kTagAddToPotentials        = "add-to-potentials";
char const* const sml_Names::kTagNegated                = "negated" ;
char const* const sml_Names::kTagNots                   = "nots" ;
char const* const sml_Names::kTagNot                    = "not" ;
char const* const sml_Names::kTagGroundedPotentials     = "grounded-potentials";
char const* const sml_Names::kTagUngroundedPotentials   = "ungrounded-potentials";
char const* const sml_Names::kTagUngroundedPotential    = "ungrounded-potential";
char const* const sml_Names::kBacktracedAlready         = "already-backtraced";
char const* const sml_Names::kBacktraceSymbol1          = "symbol1";
char const* const sml_Names::kBacktraceSymbol2          = "symbol2";

// numeric indifference stuff
char const* const sml_Names::kTagCandidate      = "candidate";
char const* const sml_Names::kCandidateName     = "name";
char const* const sml_Names::kCandidateType     = "type";
char const* const sml_Names::kCandidateTypeSum  = "sum";
char const* const sml_Names::kCandidateTypeAvg  = "avg";
char const* const sml_Names::kCandidateValue    = "value";

// output for the verbose command
char const* const sml_Names::kTagVerbose    = "verbose";
// Tag message has attribute kTypeString

// support for printing random messages
char const* const sml_Names::kTagMessage    = "message";
// Tag message has attribute kTypeString

// marker for showing beginning of action-side
char const* const sml_Names::kTagActionSideMarker	= "actionsidemarker";

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
char const* const sml_Names::kParamEcho				= "echo" ;
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
char const* const sml_Names::kParamProcSeconds		= "proc-seconds";
char const* const sml_Names::kParamRealSeconds		= "real-seconds";
char const* const sml_Names::kParamWarningsSetting	= "warnings_setting";
char const* const sml_Names::kParamEventID			= "eventid" ;
char const* const sml_Names::kParamPhase			= "phase" ;
char const* const sml_Names::kParamDecision			= "decision" ;
char const* const sml_Names::kParamInstance			= "instance" ;
char const* const sml_Names::kParamTimers			= "timers";
char const* const sml_Names::kParamMessage			= "message";
char const* const sml_Names::kParamSelf				= "self" ;
char const* const sml_Names::kParamAlias			= "alias";
char const* const sml_Names::kParamAliasedCommand		= "aliasedcommand";
char const* const sml_Names::kParamIndifferentSelectionMode = "indifferentselectionmode";
char const* const sml_Names::kParamNumericIndifferentMode	= "numericindifferentmode";
char const* const sml_Names::kParamRunResult		= "runresult";
char const* const sml_Names::kParamVersionMajor		= "versionmajor";
char const* const sml_Names::kParamVersionMinor		= "versionminor";
char const* const sml_Names::kParamVersionMicro		= "versionmicro";
char const* const sml_Names::kParamBuildDate		= "builddate";
char const* const sml_Names::kParamWaitSNC			= "waitsnc";
char const* const sml_Names::kParamFunction			= "function" ;
char const* const sml_Names::kParamChunkNamePrefix	= "chunknameprefix" ;
char const* const sml_Names::kParamChunkCount		= "chunkcount" ;
char const* const sml_Names::kParamChunkLongFormat	= "chunklongformat" ;

// Source command parameters
char const* const sml_Names::kParamSourcedProductionCount	= "sourced-production-count";
char const* const sml_Names::kParamExcisedProductionCount	= "excised-production-count";

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

// Main command set
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
char const* const sml_Names::kCommand_Shutdown				= "shutdown" ;
char const* const sml_Names::kCommand_GetVersion			= "version" ;
char const* const sml_Names::kCommand_IsSoarRunning			= "is_running" ;
char const* const sml_Names::kCommand_GetConnections		= "get_connections" ;
char const* const sml_Names::kCommand_SetConnectionInfo		= "set_connection_info" ;
char const* const sml_Names::kCommand_GetAllInput			= "get_all_input" ;
char const* const sml_Names::kCommand_GetAllOutput			= "get_all_output" ;
char const* const sml_Names::kCommand_GetRunState			= "get_run_state" ;
char const* const sml_Names::kCommand_IsProductionLoaded	= "is_production_loaded" ;
char const* const sml_Names::kCommand_SendClientMessage		= "send_client_message" ;
char const* const sml_Names::kCommand_WasAgentOnRunList		= "on_run_list" ;
char const* const sml_Names::kCommand_GetResultOfLastRun	= "last_run_result" ;


// command line interface
char const* const sml_Names::kCommand_CommandLine		 = "cmdline" ;
char const* const sml_Names::kCommand_ExpandCommandLine  = "expandcmd" ;
