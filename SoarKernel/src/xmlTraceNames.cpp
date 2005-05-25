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

#include "xmlTraceNames.h"

//for RHS output
char const* const xmlTraceNames::kTagRHS_write	= "rhs_write" ;
char const* const xmlTraceNames::kRHS_String	= "string" ;

// Tags defined for Trace output at each watch level:

// <trace> contains the rest.
char const* const xmlTraceNames::kTagTrace		= "trace" ;

// <context> tag identifiers for Watch level 1
char const* const xmlTraceNames::kTagState				= "state" ;
char const* const xmlTraceNames::kTagOperator			= "operator" ;
char const* const xmlTraceNames::kState_ID				= "current_state_id" ;
char const* const xmlTraceNames::kState_Name			= "name" ;
char const* const xmlTraceNames::kState_DecisionCycleCt	= "decision_cycle_count" ;
char const* const xmlTraceNames::kState_ImpasseObject	= "impasse_object" ;
char const* const xmlTraceNames::kState_ImpasseType		= "impasse_type" ;
char const* const xmlTraceNames::kState_StackLevel		= "stack_level" ;
char const* const xmlTraceNames::kOperator_ID			= "current_operator_id" ;
char const* const xmlTraceNames::kOperator_Name			= "name" ;
char const* const xmlTraceNames::kOperator_DecisionCycleCt = "decision_cycle_count" ;
char const* const xmlTraceNames::kOperator_StackLevel	= "stack_level" ;

// <phase> tag identifiers for Watch level 2
char const* const xmlTraceNames::kTagPhase  	= "phase" ;
char const* const xmlTraceNames::kPhase_Name  	= "name" ;
char const* const xmlTraceNames::kPhase_Status  	= "status" ;
char const* const xmlTraceNames::kPhase_FiringType 	= "firing_type" ;
char const* const xmlTraceNames::kPhaseName_Input  	= "input" ;
char const* const xmlTraceNames::kPhaseName_Pref  	= "preference" ;
char const* const xmlTraceNames::kPhaseName_WM  	= "workingmemory" ;
char const* const xmlTraceNames::kPhaseName_Decision= "decision" ;
char const* const xmlTraceNames::kPhaseName_Output 	= "output" ;
// next two are new phase names
char const* const xmlTraceNames::kPhaseName_Propose	= "propose" ;
char const* const xmlTraceNames::kPhaseName_Apply  	= "apply" ;
char const* const xmlTraceNames::kPhaseName_Unknown	= "unknown" ;
char const* const xmlTraceNames::kPhaseStatus_Begin	= "begin" ;
char const* const xmlTraceNames::kPhaseStatus_End	= "end" ;
char const* const xmlTraceNames::kPhaseFiringType_IE= "IE" ;
char const* const xmlTraceNames::kPhaseFiringType_PE= "PE" ;

// <prod-firing> tag identifiers for Watch level 3
char const* const xmlTraceNames::kTagProduction		= "production" ;
char const* const xmlTraceNames::kProduction_Name  	= "prodname" ;
char const* const xmlTraceNames::kTagProduction_Firing  	= "firing_production" ;
char const* const xmlTraceNames::kTagProduction_Retracting  = "retracting_production" ;

// <wme> tag identifiers, also for Watch level 4
char const* const xmlTraceNames::kTagWME		= "wme" ;
char const* const xmlTraceNames::kWME_TimeTag	= "tag" ;
char const* const xmlTraceNames::kWME_Id		= "id" ;
char const* const xmlTraceNames::kWME_Attribute	= "attr" ;
char const* const xmlTraceNames::kWME_Value		= "value" ;
char const* const xmlTraceNames::kWME_ValueType	= "type" ;
char const* const xmlTraceNames::kWMEPreference = "preference";
char const* const xmlTraceNames::kWME_Action	= "action" ;
// kjc question:  should the next entry be kWMEAction_Add?
char const* const xmlTraceNames::kValueAdd		= "add" ;
char const* const xmlTraceNames::kValueRemove	= "remove" ;
char const* const xmlTraceNames::kTagWMERemove	= "removing_wme" ;
char const* const xmlTraceNames::kTagWMEAdd 	= "adding_wme" ;

// <preference> tag identifiers, also Watch level 5
char const* const xmlTraceNames::kTagPreference		= "preference" ;
char const* const xmlTraceNames::kPreference_Type	= "pref_type" ;
char const* const xmlTraceNames::kOSupported		= "o_supported" ;
char const* const xmlTraceNames::kReferent			= "referent" ;

char const* const xmlTraceNames::kTagWarning		= "warning" ;
// Tag warning has attribute kTypeString

// learning stuff
char const* const xmlTraceNames::kTagLearning	= "learning" ;

//production printing
char const* const xmlTraceNames::kTagConditions                 	= "conditions" ;
char const* const xmlTraceNames::kTagConjunctive_Negation_Condition	= "conjunctive-negation-condition" ;
char const* const xmlTraceNames::kTagCondition	                    = "condition" ;
char const* const xmlTraceNames::kTagActions	                    = "actions" ;
char const* const xmlTraceNames::kTagAction 	                    = "action" ;
char const* const xmlTraceNames::kProductionDocumentation           = "documentation" ;
char const* const xmlTraceNames::kProductionType                    = "type" ;
char const* const xmlTraceNames::kProductionTypeDefault             = ":default" ;
char const* const xmlTraceNames::kProductionTypeChunk               = ":chunk" ;
char const* const xmlTraceNames::kProductionTypeJustification       = ":justification ;# not reloadable" ;
char const* const xmlTraceNames::kProductionDeclaredSupport         = "declared-support" ;
char const* const xmlTraceNames::kProductionDeclaredOSupport        = ":o-support" ;
char const* const xmlTraceNames::kProductionDeclaredISupport        = ":i-support" ;
char const* const xmlTraceNames::kConditionId                       = "id" ;
char const* const xmlTraceNames::kConditionTest                     = "test";
char const* const xmlTraceNames::kConditionTestState                = "state";
char const* const xmlTraceNames::kConditionTestImpasse              = "impasse";
char const* const xmlTraceNames::kCondition                         = "condition" ;
char const* const xmlTraceNames::kAction                            = "action" ;
char const* const xmlTraceNames::kActionFunction                    = "function" ;
char const* const xmlTraceNames::kActionId                          = "id" ;

//backtrace stuff
char const* const xmlTraceNames::kTagBacktrace              = "backtrace" ;
char const* const xmlTraceNames::kTagGrounds                = "grounds" ;
char const* const xmlTraceNames::kTagPotentials             = "potentials" ;
char const* const xmlTraceNames::kTagLocals                 = "locals" ;
char const* const xmlTraceNames::kTagLocal                  = "local";
char const* const xmlTraceNames::kTagBacktraceResult        = "result";
char const* const xmlTraceNames::kTagProhibitPreference     = "prohibit-preference";
char const* const xmlTraceNames::kTagAddToPotentials        = "add-to-potentials";
char const* const xmlTraceNames::kTagNegated                = "negated" ;
char const* const xmlTraceNames::kTagNots                   = "nots" ;
char const* const xmlTraceNames::kTagGroundedPotentials     = "grounded-potentials";
char const* const xmlTraceNames::kTagUngroundedPotentials   = "ungrounded-potentials";
char const* const xmlTraceNames::kTagUngroundedPotential    = "ungrounded-potential";
char const* const xmlTraceNames::kBacktracedAlready         = "already-backtraced";
char const* const xmlTraceNames::kBacktraceSymbol1          = "symbol1";
char const* const xmlTraceNames::kBacktraceSymbol2          = "symbol2";

// numeric indifference stuff
char const* const xmlTraceNames::kTagCandidate      = "candidate";
char const* const xmlTraceNames::kCandidateName     = "name";
char const* const xmlTraceNames::kCandidateType     = "type";
char const* const xmlTraceNames::kCandidateTypeSum  = "sum";
char const* const xmlTraceNames::kCandidateTypeAvg  = "avg";
char const* const xmlTraceNames::kCandidateValue    = "value";

// XML function types for XML output event
char const* const xmlTraceNames::kFunctionBeginTag		= "begintag";
char const* const xmlTraceNames::kFunctionEndTag		= "endtag";
char const* const xmlTraceNames::kFunctionAddAttribute	= "addattribute";

char const* const xmlTraceNames::kTypeString	= "string" ;