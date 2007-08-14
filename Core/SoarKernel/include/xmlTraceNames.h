/////////////////////////////////////////////////////////////////
//xmlTraceNames
//
// Author: Bob Marinier, University of Michigan
// Date  : May 2005
//
// The names (identifiers) we use in the XML trace.
// These were originally created by Karen and Doug in sml_Names.h
//  but we decided to copy them here to prevent the kernel from
//  needing ConnectionSML stuff to build (or vice versa).
//
// NOTE: It is very important that these constants match their
//  partners in sml_Names.h/cpp (in ConnectionSML). Those files
//  may actually define more constants (that is ok), but the ones
//  here must also be there and have the same values.
//
/////////////////////////////////////////////////////////////////

#ifndef XML_TRACE_NAMESH
#define XML_TRACE_NAMESH

namespace xmlTraceNames
{
	//for RHS output
	static char const* const kTagRHS_write	= "rhs_write" ;

	static char const* const kRHS_String	= "string" ;

	// Tags defined for Trace output at each watch level:

	// Types
	static char const* const kTypeString	= "string" ;
	static char const* const kTypeInt		= "int" ;
	static char const* const kTypeDouble	= "double" ;
	static char const* const kTypeChar		= "char" ;
	static char const* const kTypeBoolean	= "boolean" ;
	static char const* const kTypeID		= "id" ;
	static char const* const kTypeVariable	= "variable" ;

	// <trace> contains the rest.
	static char const* const kTagTrace		= "trace" ;

	// <context> tag identifiers for Watch level 1
	static char const* const kTagState				= "state" ;
	static char const* const kTagOperator			= "operator" ;
	static char const* const kState_ID				= "current_state_id" ;
	static char const* const kState_Name			= "name" ;
	static char const* const kState_DecisionCycleCt	= "decision_cycle_count" ;
	static char const* const kState_ImpasseObject	= "impasse_object" ;
	static char const* const kState_ImpasseType		= "impasse_type" ;
	static char const* const kState_StackLevel		= "stack_level" ;
	static char const* const kOperator_ID			= "current_operator_id" ;
	static char const* const kOperator_Name			= "name" ;
	static char const* const kOperator_DecisionCycleCt = "decision_cycle_count" ;
	static char const* const kOperator_StackLevel	= "stack_level" ;

	// <phase> tag identifiers for Watch level 2
	static char const* const kTagPhase  	= "phase" ;
	static char const* const kPhase_Name  	= "name" ;
	static char const* const kPhase_Status  	= "status" ;
	static char const* const kPhase_FiringType 	= "firing_type" ;
	static char const* const kPhaseName_Input  	= "input" ;
	static char const* const kPhaseName_Pref  	= "preference" ;
	static char const* const kPhaseName_WM  	= "workingmemory" ;
	static char const* const kPhaseName_Decision= "decision" ;
	static char const* const kPhaseName_Output 	= "output" ;

	// next are new phase names
	static char const* const kPhaseName_Propose	= "propose" ;
	static char const* const kPhaseName_Apply  	= "apply" ;
	static char const* const kPhaseName_Unknown	= "unknown" ;
	static char const* const kPhaseStatus_Begin	= "begin" ;
	static char const* const kPhaseStatus_End	= "end" ;
	static char const* const kPhaseFiringType_IE= "IE" ;
	static char const* const kPhaseFiringType_PE= "PE" ;

	static char const* const kTagSubphase							= "subphase";
	static char const* const kSubphaseName_FiringProductions		= "firingprods";
	static char const* const kSubphaseName_ChangingWorkingMemory	= "changingwm";

	// <prod-firing> tag identifiers for Watch level 3
	static char const* const kTagProduction		= "production" ;
	static char const* const kProduction_Name  	= "prodname" ;
	static char const* const kTagProduction_Firing  	= "firing_production" ;
	static char const* const kTagProduction_Retracting  = "retracting_production" ;

	// <wme> tag identifiers, also for Watch level 4
	static char const* const kTagWME		= "wme" ;
	static char const* const kWME_TimeTag	= "tag" ;
	static char const* const kWME_Id		= "id" ;
	static char const* const kWME_Attribute	= "attr" ;
	static char const* const kWME_Value		= "value" ;
	static char const* const kWME_AttributeType	= "attrtype" ;
	static char const* const kWME_ValueType	= "valtype" ;
	static char const* const kWMEPreference = "preference";
	static char const* const kWME_Action	= "action" ;
	// kjc question:  should the next entry be kWMEAction_Add?
	static char const* const kValueAdd		= "add" ;
	static char const* const kValueRemove	= "remove" ;
	static char const* const kTagWMERemove	= "removing_wme" ;
	static char const* const kTagWMEAdd 	= "adding_wme" ;

	// <preference> tag identifiers, also Watch level 5
	static char const* const kTagPreference		= "preference" ;
	static char const* const kPreference_Type	= "pref_type" ;
	static char const* const kOSupported		= "o_supported" ;
	static char const* const kReferent			= "referent" ;

	static char const* const kTagWarning		= "warning" ;
	// Tag warning has attribute kTypeString

	static char const* const kTagError		= "error" ;
	// Tag error has attribute kTypeString

	// learning stuff
	static char const* const kTagLearning	= "learning" ;

	//production printing
	static char const* const kTagConditions                 	= "conditions" ;
	static char const* const kTagConjunctive_Negation_Condition	= "conjunctive-negation-condition" ;
	static char const* const kTagCondition	                    = "condition" ;
	static char const* const kTagActions	                    = "actions" ;
	static char const* const kTagAction 	                    = "action" ;
	static char const* const kProductionDocumentation           = "documentation" ;
	static char const* const kProductionType                    = "type" ;
	static char const* const kProductionTypeDefault             = ":default" ;
	static char const* const kProductionTypeChunk               = ":chunk" ;
	static char const* const kProductionTypeJustification       = ":justification ;# not reloadable" ;
	static char const* const kProductionDeclaredSupport         = "declared-support" ;
	static char const* const kProductionDeclaredOSupport        = ":o-support" ;
	static char const* const kProductionDeclaredISupport        = ":i-support" ;
	static char const* const kConditionId                       = "id" ;
	static char const* const kConditionTest                     = "test";
	static char const* const kConditionTestState                = "state";
	static char const* const kConditionTestImpasse              = "impasse";
	static char const* const kCondition                         = "condition" ;
	static char const* const kAction                            = "action" ;
	static char const* const kActionFunction                    = "function" ;
	static char const* const kActionId                          = "id" ;
	static char const* const kValue                             = "value" ;
	static char const* const kAttribute                         = "attribute" ;

	//backtrace stuff
	static char const* const kTagBacktrace              = "backtrace" ;
	static char const* const kTagGrounds                = "grounds" ;
	static char const* const kTagPotentials             = "potentials" ;
	static char const* const kTagLocals                 = "locals" ;
	static char const* const kTagLocal                  = "local";
	static char const* const kTagBacktraceResult        = "result";
	static char const* const kTagProhibitPreference     = "prohibit-preference";
	static char const* const kTagAddToPotentials        = "add-to-potentials";
	static char const* const kTagNegated                = "negated" ;
	static char const* const kTagNots                   = "nots" ;
	static char const* const kTagNot                   = "not" ;
	static char const* const kTagGroundedPotentials     = "grounded-potentials";
	static char const* const kTagUngroundedPotentials   = "ungrounded-potentials";
	static char const* const kTagUngroundedPotential    = "ungrounded-potential";
	static char const* const kBacktracedAlready         = "already-backtraced";
	static char const* const kBacktraceSymbol1          = "symbol1";
	static char const* const kBacktraceSymbol2          = "symbol2";

	// numeric indifference stuff
	static char const* const kTagCandidate      = "candidate";
	static char const* const kCandidateName     = "name";
	static char const* const kCandidateType     = "type";
	static char const* const kCandidateTypeSum  = "sum";
	static char const* const kCandidateTypeAvg  = "avg";
	static char const* const kCandidateValue    = "value";
	static char const* const kCandidateExpValue = "exp";

	// output for the verbose command
	static char const* const kTagVerbose    = "verbose";
	// Tag message has attribute kTypeString

	// support for printing random messages
	static char const* const kTagMessage    = "message";
	// Tag message has attribute kTypeString

	// Values returned from commands
	static char const* const kName			= "name" ;
	static char const* const kGoal			= "goal" ;
	static char const* const kCount			= "count" ;
	static char const* const kOAssertions	= "O-assertions" ;
	static char const* const kIAssertions	= "I-assertions" ;
	static char const* const kAssertions	= "assertions" ;
	static char const* const kRetractions	= "retractions" ;
	static char const* const kMatches		= "matches" ;
	static char const* const kMatchCount	= "matchcount" ;
	static char const* const kTagLeftMatches  = "left-matches" ;
	static char const* const kTagRightMatches = "right-matches" ;
	static char const* const kTagToken		= "token" ;

	// marker for showing beginning of action-side
	static char const* const kTagActionSideMarker	= "actionsidemarker";

	// XML function types for XML output event
	static char const* const kFunctionBeginTag		= "begintag";
	static char const* const kFunctionEndTag		= "endtag";
	static char const* const kFunctionAddAttribute	= "addattribute";
};

#endif // XML_TRACE_NAMESH
