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
	static char const* const kTagSML ;
	static char const* const kID ;
	static char const* const kAck ;
	static char const* const kDocType ;
	static char const* const kDocType_Call ;
	static char const* const kDocType_Response ;
	static char const* const kDocType_Notify ;
	static char const* const kSoarVersion ;
	static char const* const kSMLVersion ;
	static char const* const kSMLVersionValue ;
	static char const* const kSoarVersionValue ;
	static char const* const kOutputLinkName ;

	// <command> tag identifiers
	static char const* const kTagCommand ;
	static char const* const kCommandName ;
	static char const* const kCommandOutput ;
	static char const* const kRawOutput ;
	static char const* const kStructuredOutput ;

	// <connection> tag identifiers
	static char const* const kTagConnection ;
	static char const* const kConnectionId ;
	static char const* const kConnectionName ;
	static char const* const kConnectionStatus ;
	static char const* const kAgentStatus ;
	static char const* const kStatusCreated	;	// Initial status -- simply means connection exists
	static char const* const kStatusNotReady ;	// Connection not ready (work needs to be done still)
	static char const* const kStatusReady ;		// Connection ready (registered for events etc.)
	static char const* const kStatusClosing ;	// Connection about to shut down

	// <arg> tag identifiers
	static char const* const kTagArg ;
	static char const* const kArgParam ;
	static char const* const kArgType ;

	// <error> tag identifiers
	static char const* const kTagError ;
	static char const* const kErrorCode ;

	// <name> tag identifiers
	static char const* const kTagName ;

	// <result> tag identifiers
	static char const* const kTagResult ;

	// input values (for update param)
	static char const* const kValueDelta ;
	static char const* const kValueFull ;

	//for RHS output
	static char const* const kTagRHS_write ;
	static char const* const kRHS_String ;

	// Tags defined for Trace output at each watch level:
	// <trace> contains the rest.
	static char const* const kTagTrace ;

	// <context> tag identifiers for Watch level 1
	static char const* const kTagState ;
	static char const* const kTagOperator ;
	static char const* const kState_ID;
	static char const* const kState_Name;
	static char const* const kState_DecisionCycleCt;
	static char const* const kState_ImpasseObject;
	static char const* const kState_ImpasseType;
	static char const* const kState_StackLevel ;
	static char const* const kOperator_ID;
	static char const* const kOperator_Name;
	static char const* const kOperator_DecisionCycleCt;
	static char const* const kOperator_StackLevel ;
 
	// <phase> tag identifiers for Watch level 2
	static char const* const kTagPhase ;
	static char const* const kPhase_Name ;
	static char const* const kPhase_Status ;
	static char const* const kPhase_FiringType ;
	static char const* const kPhaseName_Input ;
	static char const* const kPhaseName_Pref ;
	static char const* const kPhaseName_WM ;
	static char const* const kPhaseName_Decision ;
	static char const* const kPhaseName_Output ;
	static char const* const kPhaseName_Propose ;
	static char const* const kPhaseName_Apply ;
	static char const* const kPhaseName_Unknown;
	static char const* const kPhaseStatus_Begin ;
	static char const* const kPhaseStatus_End ;
 	static char const* const kPhaseFiringType_IE ;
	static char const* const kPhaseFiringType_PE ;

	static char const* const kTagSubphase;
	static char const* const kSubphaseName_FiringProductions;
	static char const* const kSubphaseName_ChangingWorkingMemory;

	// <prod-firing> tag identifiers for Watch level 3
	static char const* const kTagProduction ;
	static char const* const kProduction_Name ;
 	static char const* const kTagProduction_Firing ;
	static char const* const kTagProduction_Retracting ;
	
	// <wme> tag identifiers, also Watch level 4
	static char const* const kTagWME ;
	static char const* const kWME_TimeTag ;
	static char const* const kWME_Id ;
	static char const* const kWME_Attribute ;
	static char const* const kWME_Value ;
	static char const* const kWME_ValueType ;
	static char const* const kWME_AttributeType	;
	static char const* const kWME_Preference;
	static char const* const kWME_Action ;
	// kjc question:  should the next entry be kWMEAction_Add?
	static char const* const kValueAdd	;
	static char const* const kValueRemove ;
	static char const* const kTagWMERemove ;
	static char const* const kTagWMEAdd ;

	// <preference> tag identifiers, also Watch level 5
	static char const* const kTagPreference ;
	static char const* const kPreference_Type ;
	static char const* const kOSupported ;
	static char const* const kReferent ;

	// for warnings controlled by WARNINGS_SYSPARAM
	static char const* const kTagWarning ;

	// XML function types for XML output event
	static char const* const kFunctionBeginTag;
	static char const* const kFunctionEndTag;
	static char const* const kFunctionAddAttribute;

	// learning stuff
	static char const* const kTagLearning;
	// Tag learning has attribute kTypeString

	// filter support
	static char const* const kTagFilter ;
	static char const* const kFilterCommand ;
	static char const* const kFilterError ;
	static char const* const kFilterOutput ;
	static char const* const kFilterName ;
	static char const* const kParamNoFiltering ;

    //production printing
    static char const* const kTagConditions;
    static char const* const kTagConjunctive_Negation_Condition;
    static char const* const kTagCondition;
    static char const* const kTagActions;
    static char const* const kTagAction;
	static char const* const kProductionDocumentation;
    static char const* const kProductionType;
    static char const* const kProductionTypeDefault;
    static char const* const kProductionTypeChunk;
    static char const* const kProductionTypeJustification;
    static char const* const kProductionDeclaredSupport;
    static char const* const kProductionDeclaredOSupport;
    static char const* const kProductionDeclaredISupport;
    static char const* const kConditionId;
    static char const* const kConditionTest;
    static char const* const kConditionTestState;
    static char const* const kConditionTestImpasse;
    static char const* const kCondition;
    static char const* const kAction;
    static char const* const kActionFunction;
    static char const* const kActionId;

    //backtrace stuff
    static char const* const kTagBacktrace;
    static char const* const kTagGrounds;
    static char const* const kTagPotentials;
    static char const* const kTagLocals;
    static char const* const kTagLocal;
	static char const* const kTagBacktraceResult;
    static char const* const kTagProhibitPreference;
    static char const* const kTagAddToPotentials;
    static char const* const kTagNegated;
    static char const* const kTagNots;
	static char const* const kTagNot;
    static char const* const kTagGroundedPotentials;
    static char const* const kTagUngroundedPotentials;
    static char const* const kTagUngroundedPotential;
    static char const* const kBacktracedAlready;
    static char const* const kBacktraceSymbol1;
    static char const* const kBacktraceSymbol2;

    // numeric indifference stuff
    static char const* const kTagCandidate;
    static char const* const kCandidateName;
    static char const* const kCandidateType;
    static char const* const kCandidateTypeSum;
    static char const* const kCandidateTypeAvg;
    static char const* const kCandidateValue;

    // output for the verbose command
    static char const* const kTagVerbose;

    // support for printing random messages
    static char const* const kTagMessage;

	// marker for showing beginning of action-side
	static char const* const kTagActionSideMarker;

	// end of tags for Trace output

	// Types
	static char const* const kTypeString ;
	static char const* const kTypeInt ;
	static char const* const kTypeDouble ;
	static char const* const kTypeChar ;
	static char const* const kTypeBoolean ;
	static char const* const kTypeID ;
	static char const* const kTypeVariable ;

	// Params
	static char const* const kParamAgent ;
	static char const* const kParamKernel ;
	static char const* const kParamThis ;
	static char const* const kParamName ;
	static char const* const kParamFilename ;
	static char const* const kParamLearning ;
	static char const* const kParamOSupportMode ;
	static char const* const kParamValue ;
	static char const* const kParamWme ;
	static char const* const kParamWmeObject ;
	static char const* const kParamAttribute ;
	static char const* const kParamCount ;
	static char const* const kParamLength ;
	static char const* const kParamThread	;
	static char const* const kParamProcess ;
	static char const* const kParamLine ;
	static char const* const kParamEcho ;
	static char const* const kParamLocation ;
	static char const* const kParamLogLocation ;
	static char const* const kParamLogLevel ;
	static char const* const kParamInputProducer ;
	static char const* const kParamOutputProcessor ;
	static char const* const kParamWorkingMemory ;
	static char const* const kParamAttributePath ;
	static char const* const kParamUpdate ;
	static char const* const kParamEventID ;
	static char const* const kParamLearnSetting;
	static char const* const kParamLearnOnlySetting;
	static char const* const kParamLearnExceptSetting;
	static char const* const kParamLearnAllLevelsSetting;
	static char const* const kParamLearnForceLearnStates;
	static char const* const kParamLearnDontLearnStates;
	static char const* const kParamLogSetting;
	static char const* const kParamDirectory;
	static char const* const kParamProcSeconds;
	static char const* const kParamRealSeconds;
	static char const* const kParamWarningsSetting;
	static char const* const kParamPhase ;
	static char const* const kParamDecision ;
	static char const* const kParamRunState ;
	static char const* const kParamInstance ;
	static char const* const kParamTimers;
	static char const* const kParamMessage;
	static char const* const kParamSelf ;
	static char const* const kParamAlias;
	static char const* const kParamAliasedCommand;
	static char const* const kParamIndifferentSelectionMode;
	static char const* const kParamNumericIndifferentMode;
	static char const* const kParamRunResult;
	static char const* const kParamVersionMajor;
	static char const* const kParamVersionMinor;
	static char const* const kParamVersionMicro;
	static char const* const kParamBuildDate;
	static char const* const kParamWaitSNC;
	static char const* const kParamFunction ;
	static char const* const kParamChunkNamePrefix;
	static char const* const kParamChunkCount;
	static char const* const kParamChunkLongFormat;

	// Parameter names for source command
	static char const* const kParamSourcedProductionCount;
	static char const* const kParamExcisedProductionCount;

	// Parameter names for stats command
	static char const* const kParamStatsProductionCountDefault;
	static char const* const kParamStatsProductionCountUser;
	static char const* const kParamStatsProductionCountChunk;
	static char const* const kParamStatsProductionCountJustification;
	static char const* const kParamStatsCycleCountDecision;
	static char const* const kParamStatsCycleCountElaboration;
	static char const* const kParamStatsProductionFiringCount;
	static char const* const kParamStatsWmeCountAddition;
	static char const* const kParamStatsWmeCountRemoval;
	static char const* const kParamStatsWmeCount;
	static char const* const kParamStatsWmeCountAverage;
	static char const* const kParamStatsWmeCountMax;
	static char const* const kParamStatsKernelCPUTime;
	static char const* const kParamStatsTotalCPUTime;
 	static char const* const kParamStatsPhaseTimeInputPhase; 
	static char const* const kParamStatsPhaseTimeProposePhase;
	static char const* const kParamStatsPhaseTimeDecisionPhase;
	static char const* const kParamStatsPhaseTimeApplyPhase;
	static char const* const kParamStatsPhaseTimeOutputPhase;
	static char const* const kParamStatsPhaseTimePreferencePhase;
	static char const* const kParamStatsPhaseTimeWorkingMemoryPhase;
	static char const* const kParamStatsMonitorTimeInputPhase;
	static char const* const kParamStatsMonitorTimeProposePhase;
	static char const* const kParamStatsMonitorTimeDecisionPhase;
	static char const* const kParamStatsMonitorTimeApplyPhase;
	static char const* const kParamStatsMonitorTimeOutputPhase;
	static char const* const kParamStatsMonitorTimePreferencePhase;
	static char const* const kParamStatsMonitorTimeWorkingMemoryPhase;
	static char const* const kParamStatsInputFunctionTime;
	static char const* const kParamStatsOutputFunctionTime;
 	static char const* const kParamStatsMatchTimeInputPhase;
	static char const* const kParamStatsMatchTimePreferencePhase;
	static char const* const kParamStatsMatchTimeWorkingMemoryPhase;
	static char const* const kParamStatsMatchTimeOutputPhase;
	static char const* const kParamStatsMatchTimeDecisionPhase;
	static char const* const kParamStatsMatchTimeProposePhase;
	static char const* const kParamStatsMatchTimeApplyPhase;
	static char const* const kParamStatsOwnershipTimeInputPhase;
 	static char const* const kParamStatsOwnershipTimePreferencePhase;
	static char const* const kParamStatsOwnershipTimeWorkingMemoryPhase;
	static char const* const kParamStatsOwnershipTimeOutputPhase;
	static char const* const kParamStatsOwnershipTimeDecisionPhase;
	static char const* const kParamStatsOwnershipTimeProposePhase;
	static char const* const kParamStatsOwnershipTimeApplyPhase;
	static char const* const kParamStatsChunkingTimeInputPhase;
 	static char const* const kParamStatsChunkingTimePreferencePhase;
	static char const* const kParamStatsChunkingTimeWorkingMemoryPhase;
	static char const* const kParamStatsChunkingTimeOutputPhase;
	static char const* const kParamStatsChunkingTimeDecisionPhase;
	static char const* const kParamStatsChunkingTimeProposePhase;
	static char const* const kParamStatsChunkingTimeApplyPhase;
	static char const* const kParamStatsMemoryUsageMiscellaneous;
	static char const* const kParamStatsMemoryUsageHash;
	static char const* const kParamStatsMemoryUsageString;
	static char const* const kParamStatsMemoryUsagePool;
	static char const* const kParamStatsMemoryUsageStatsOverhead;
	// Parameter names for watch command
	static char const* const kParamWatchDecisions;
	static char const* const kParamWatchPhases;
	static char const* const kParamWatchProductionDefault;
	static char const* const kParamWatchProductionUser;
	static char const* const kParamWatchProductionChunks;
	static char const* const kParamWatchProductionJustifications;
	static char const* const kParamWatchWMEDetail;
	static char const* const kParamWatchWorkingMemoryChanges;
	static char const* const kParamWatchPreferences;
	static char const* const kParamWatchLearning;
	static char const* const kParamWatchBacktracing;
	static char const* const kParamWatchIndifferentSelection;

	// Values (these are not case sensitive unlike the rest)
	static char const* const kTrue ;
	static char const* const kFalse ;

	// Main command set
	static char const* const kCommand_CreateAgent ;
	static char const* const kCommand_DestroyAgent ;
	static char const* const kCommand_GetAgentList ;
	static char const* const kCommand_LoadProductions ;
	static char const* const kCommand_GetInputLink ;
	static char const* const kCommand_GetOutputLink ;
	static char const* const kCommand_Run ;
	static char const* const kCommand_Input ;
	static char const* const kCommand_Output ;
	static char const* const kCommand_StopOnOutput ;
	static char const* const kCommand_RegisterForEvent ;
	static char const* const kCommand_UnregisterForEvent ;
	static char const* const kCommand_Event ;			// Just passes event id
	static char const* const kCommand_FireEvent ;
	static char const* const kCommand_SuppressEvent ;
	static char const* const kCommand_CheckForIncomingCommands ;
	static char const* const kCommand_SetInterruptCheckRate ;
	static char const* const kCommand_Shutdown ;
	static char const* const kCommand_GetVersion ;
	static char const* const kCommand_IsSoarRunning	;
	static char const* const kCommand_GetConnections ;
	static char const* const kCommand_SetConnectionInfo ;
	static char const* const kCommand_GetAllInput ;
	static char const* const kCommand_GetAllOutput ;
	static char const* const kCommand_GetRunState ;
	static char const* const kCommand_IsProductionLoaded	;
	static char const* const kCommand_SendClientMessage	;
	static char const* const kCommand_WasAgentOnRunList	;
	static char const* const kCommand_GetResultOfLastRun ;
	static char const* const kCommand_GetInitialTimeTag ;

	// Command line interface
	static char const* const kCommand_CommandLine ;
	static char const* const kCommand_ExpandCommandLine ;
} ;

}

#endif // SML_NAMESH
