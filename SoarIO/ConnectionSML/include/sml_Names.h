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

	// <wme> tag identifiers
	static char const* const kTagWME ;
	static char const* const kWME_TimeTag ;
	static char const* const kWME_Id ;
	static char const* const kWME_Attribute ;
	static char const* const kWME_Value ;
	static char const* const kWME_ValueType ;
	static char const* const kWME_Action ;
	static char const* const kValueAdd	;
	static char const* const kValueRemove ;

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
	static char const* const kParamSeconds;
	static char const* const kParamWarningsSetting;
	static char const* const kParamPhase ;
	static char const* const kParamInstance ;
	static char const* const kParamTimers;
	static char const* const kParamMessage;
	static char const* const kParamAlias;
	static char const* const kParamAliasedCommand;
	static char const* const kParamIndifferentSelectionMode;
	static char const* const kParamNumericIndifferentMode;
	static char const* const kParamRunResult;
	static char const* const kParamVersionMajor;
	static char const* const kParamVersionMinor;
	static char const* const kParamVersionMicro;
	static char const* const kParamWaitSNC;
	static char const* const kParamFunction ;
	static char const* const kParamChunkNamePrefix;
	static char const* const kParamChunkCount;
	static char const* const kParamChunkLongFormat;
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
	static char const* const kParamStatsKernelTimeTotal;
	static char const* const kParamStatsMatchTimeInputPhase;
	static char const* const kParamStatsMatchTimeDetermineLevelPhase;
	static char const* const kParamStatsMatchTimePreferencePhase;
	static char const* const kParamStatsMatchTimeWorkingMemoryPhase;
	static char const* const kParamStatsMatchTimeOutputPhase;
	static char const* const kParamStatsMatchTimeDecisionPhase;
	static char const* const kParamStatsOwnershipTimeInputPhase;
	static char const* const kParamStatsOwnershipTimeDetermineLevelPhase;
	static char const* const kParamStatsOwnershipTimePreferencePhase;
	static char const* const kParamStatsOwnershipTimeWorkingMemoryPhase;
	static char const* const kParamStatsOwnershipTimeOutputPhase;
	static char const* const kParamStatsOwnershipTimeDecisionPhase;
	static char const* const kParamStatsChunkingTimeInputPhase;
	static char const* const kParamStatsChunkingTimeDetermineLevelPhase;
	static char const* const kParamStatsChunkingTimePreferencePhase;
	static char const* const kParamStatsChunkingTimeWorkingMemoryPhase;
	static char const* const kParamStatsChunkingTimeOutputPhase;
	static char const* const kParamStatsChunkingTimeDecisionPhase;
	static char const* const kParamStatsMemoryUsageMiscellaneous;
	static char const* const kParamStatsMemoryUsageHash;
	static char const* const kParamStatsMemoryUsageString;
	static char const* const kParamStatsMemoryUsagePool;
	static char const* const kParamStatsMemoryUsageStatsOverhead;


	// Values (these are not case sensitive unlike the rest)
	static char const* const kTrue ;
	static char const* const kFalse ;

	// sgio style commands
	static char const* const kCommand_CreateAgent ;
	static char const* const kCommand_DestroyAgent ;
	static char const* const kCommand_GetAgentList ;
	static char const* const kCommand_LoadProductions ;
	static char const* const kCommand_GetInputLink ;
	static char const* const kCommand_GetOutputLink ;
	static char const* const kCommand_Run ;
	static char const* const kCommand_Input ;
	static char const* const kCommand_Output ;
	static char const* const kCommand_CheckForIncomingCommands ;
	static char const* const kCommand_StopOnOutput ;
	static char const* const kCommand_RegisterForEvent ;
	static char const* const kCommand_UnregisterForEvent ;
	static char const* const kCommand_Event ;			// Just passes event id

	// Command line interface
	static char const* const kCommand_CommandLine ;
	static char const* const kCommand_ExpandCommandLine ;
} ;

}

#endif // SML_NAMESH
