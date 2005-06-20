#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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
//  partners in sml_Names.h/cpp. Those files may actually
//  define more constants (that is ok), but the ones here must
//  also be there and have the same values.
//
/////////////////////////////////////////////////////////////////
/*
#include "xmlTraceNames.h"

using namespace xmlTraceNames;

//for RHS output
char const* const kTagRHS_write	= "rhs_write" ;

char const* const kRHS_String	= "string" ;

// Tags defined for Trace output at each watch level:

// <trace> contains the rest.
char const* const kTagTrace		= "trace" ;

// <context> tag identifiers for Watch level 1
char const* const kTagState				= "state" ;
char const* const kTagOperator			= "operator" ;
char const* const kState_ID				= "current_state_id" ;
char const* const kState_Name			= "name" ;
char const* const kState_DecisionCycleCt	= "decision_cycle_count" ;
char const* const kState_ImpasseObject	= "impasse_object" ;
char const* const kState_ImpasseType		= "impasse_type" ;
char const* const kState_StackLevel		= "stack_level" ;
char const* const kOperator_ID			= "current_operator_id" ;
char const* const kOperator_Name			= "name" ;
char const* const kOperator_DecisionCycleCt = "decision_cycle_count" ;
char const* const kOperator_StackLevel	= "stack_level" ;

// <phase> tag identifiers for Watch level 2
char const* const kTagPhase  	= "phase" ;
char const* const kPhase_Name  	= "name" ;
char const* const kPhase_Status  	= "status" ;
char const* const kPhase_FiringType 	= "firing_type" ;
char const* const kPhaseName_Input  	= "input" ;
char const* const kPhaseName_Pref  	= "preference" ;
char const* const kPhaseName_WM  	= "workingmemory" ;
char const* const kPhaseName_Decision= "decision" ;
char const* const kPhaseName_Output 	= "output" ;
// next two are new phase names
char const* const kPhaseName_Propose	= "propose" ;
char const* const kPhaseName_Apply  	= "apply" ;
char const* const kPhaseName_Unknown	= "unknown" ;
char const* const kPhaseStatus_Begin	= "begin" ;
char const* const kPhaseStatus_End	= "end" ;
char const* const kPhaseFiringType_IE= "IE" ;
char const* const kPhaseFiringType_PE= "PE" ;

// <prod-firing> tag identifiers for Watch level 3
char const* const kTagProduction		= "production" ;
char const* const kProduction_Name  	= "prodname" ;
char const* const kTagProduction_Firing  	= "firing_production" ;
char const* const kTagProduction_Retracting  = "retracting_production" ;

// <wme> tag identifiers, also for Watch level 4
char const* const kTagWME		= "wme" ;
char const* const kWME_TimeTag	= "tag" ;
char const* const kWME_Id		= "id" ;
char const* const kWME_Attribute	= "attr" ;
char const* const kWME_Value		= "value" ;
char const* const kWME_ValueType	= "type" ;
char const* const kWMEPreference = "preference";
char const* const kWME_Action	= "action" ;
// kjc question:  should the next entry be kWMEAction_Add?
char const* const kValueAdd		= "add" ;
char const* const kValueRemove	= "remove" ;
char const* const kTagWMERemove	= "removing_wme" ;
char const* const kTagWMEAdd 	= "adding_wme" ;

// <preference> tag identifiers, also Watch level 5
char const* const kTagPreference		= "preference" ;
char const* const kPreference_Type	= "pref_type" ;
char const* const kOSupported		= "o_supported" ;
char const* const kReferent			= "referent" ;

char const* const kTagWarning		= "warning" ;
// Tag warning has attribute kTypeString

char const* const kTagError		= "error" ;
// Tag error has attribute kTypeString

// learning stuff
char const* const kTagLearning	= "learning" ;

//production printing
char const* const kTagConditions                 	= "conditions" ;
char const* const kTagConjunctive_Negation_Condition	= "conjunctive-negation-condition" ;
char const* const kTagCondition	                    = "condition" ;
char const* const kTagActions	                    = "actions" ;
char const* const kTagAction 	                    = "action" ;
char const* const kProductionDocumentation           = "documentation" ;
char const* const kProductionType                    = "type" ;
char const* const kProductionTypeDefault             = ":default" ;
char const* const kProductionTypeChunk               = ":chunk" ;
char const* const kProductionTypeJustification       = ":justification ;# not reloadable" ;
char const* const kProductionDeclaredSupport         = "declared-support" ;
char const* const kProductionDeclaredOSupport        = ":o-support" ;
char const* const kProductionDeclaredISupport        = ":i-support" ;
char const* const kConditionId                       = "id" ;
char const* const kConditionTest                     = "test";
char const* const kConditionTestState                = "state";
char const* const kConditionTestImpasse              = "impasse";
char const* const kCondition                         = "condition" ;
char const* const kAction                            = "action" ;
char const* const kActionFunction                    = "function" ;
char const* const kActionId                          = "id" ;

//backtrace stuff
char const* const kTagBacktrace              = "backtrace" ;
char const* const kTagGrounds                = "grounds" ;
char const* const kTagPotentials             = "potentials" ;
char const* const kTagLocals                 = "locals" ;
char const* const kTagLocal                  = "local";
char const* const kTagBacktraceResult        = "result";
char const* const kTagProhibitPreference     = "prohibit-preference";
char const* const kTagAddToPotentials        = "add-to-potentials";
char const* const kTagNegated                = "negated" ;
char const* const kTagNots                   = "nots" ;
char const* const kTagNot                   = "not" ;
char const* const kTagGroundedPotentials     = "grounded-potentials";
char const* const kTagUngroundedPotentials   = "ungrounded-potentials";
char const* const kTagUngroundedPotential    = "ungrounded-potential";
char const* const kBacktracedAlready         = "already-backtraced";
char const* const kBacktraceSymbol1          = "symbol1";
char const* const kBacktraceSymbol2          = "symbol2";

// numeric indifference stuff
char const* const kTagCandidate      = "candidate";
char const* const kCandidateName     = "name";
char const* const kCandidateType     = "type";
char const* const kCandidateTypeSum  = "sum";
char const* const kCandidateTypeAvg  = "avg";
char const* const kCandidateValue    = "value";

// output for the verbose command
char const* const kTagVerbose    = "verbose";
// Tag message has attribute kTypeString

// support for printing random messages
char const* const kTagMessage    = "message";
// Tag message has attribute kTypeString

// marker for showing beginning of action-side
char const* const kTagActionSideMarker	= "actionsidemarker";

// XML function types for XML output event
char const* const kFunctionBeginTag		= "begintag";
char const* const kFunctionEndTag		= "endtag";
char const* const kFunctionAddAttribute	= "addattribute";

char const* const kTypeString	= "string" ;
*/
