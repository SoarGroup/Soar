/////////////////////////////////////////////////////////////////
// ClientTraceXML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2005
//
// This class is used to represent XML messages
// that contain trace output from a Soar.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_TRACE_XML_H
#define SML_CLIENT_TRACE_XML_H

#include "sml_ClientXML.h"

namespace sml {

class ClientTraceXML : public ClientXML
{
public:
	// These methods provide access to specific attributes and tags without
	// the client needing to pass in/know the strings.  They're all very simple.

	// Trace tag contains everything else
	bool IsTagTrace() const	; 

	// Write commands on right hand side of productions generate output
	// which are collected here.
	bool IsTagRhsWrite() const ;
	char const* GetString() const ;

	// State tag attributes
	bool IsTagState() const ;
	char const* GetDecisionCycleCount() const ;
	char const* GetStateID() const ;
	char const* GetImpasseObject() const ;
	char const* GetImpasseType() const ;
	char const* GetStackLevel() const ;

	// Operator tag attributes
	bool IsTagOperator() const ;
	char const* GetOperatorID() const ;
	char const* GetOperatorName() const	;
	// Included in tag, same as state
	// char const* GetDecisionCycleCount() const ;

	// Phase tag attributes
	bool IsTagPhase() const ;
	char const* GetPhaseName() const ;
	char const* GetPhaseStatus() const ;
	char const* GetFiringType() const ;

	// Subphase (firing productions/changing wm) tag
	bool IsTagSubphase() const ;
	bool IsSubphaseNameFiringProductions() const ;
	bool IsSubphaseNameChangingWorkingMemory() const ;

	// Firing-production tag, contains production
	bool IsTagFiringProduction() const ;
	bool IsTagRetractingProduction() const ;
	bool IsTagLearning() const ;

	// Production
	bool IsTagProduction() const ;
	char const* GetProductionName() const ;
	char const* GetProductionDoc() const ;
	char const* GetProductionType() const ;
	char const* GetProductionDeclaredSupport() const ;

	bool IsTagConditions() const ;
	bool IsTagCondition() const	;
	bool IsTagConjunctiveNegationCondition() const ;
	bool IsTagActions() const ;
	bool IsTagAction() const ;

	// Condition attributes: Note this form allows for multiple unparsed conditions within the "condition" attribute.
	char const* GetConditionTest() const ;
	char const* GetConditionId() const ;
	char const* GetCondition() const ;

	// Action attributes
	char const* GetActionId() const	;
	char const* GetAction() const ;
	char const* GetFunction() const	;

	// Add-wme contains wme
	bool IsTagAddWme() const ;
	bool IsTagRemoveWme() const ;

	// Wme tag attributes
	bool IsTagWme() const ;
	char const* GetWmeID() const ;
	char const* GetWmeAttribute() const ;
	char const* GetWmeValue() const ;
	char const* GetWmeTimeTag() const ;
	char const* GetWmePreference() const ;

	// Preference tag
	bool IsTagPreference() const ;
	char const* GetPreferenceID() const ;
	char const* GetPreferenceAttribute() const ;
	char const* GetPreferenceValue() const ;
	char const* GetPreferenceType() const ;
	char const* GetPreferenceTimeTag() const ;
	char const* GetPreferenceOSupported() const	;
	char const* GetPreferenceReferent() const ;

	// Marker between LHS matches and RHS results in trace
	bool IsTagActionSideMarker() const ;

	// Backtracing
	bool IsTagLocal() const	;
	bool IsTagLocals() const ;
	bool IsTagGrounds() const ;
	bool IsTagNegated() const ;
	bool IsTagNot() const ;
	bool IsTagNots() const ;
	bool IsTagPotentials() const ;
	bool IsTagGroundedPotentials() const ;
	bool IsTagUngroundedPotentials() const ;
	bool IsTagUngroundedPotential() const ;
	bool IsTagBacktrace() const	;
	bool IsTagAddToPotentials() const ;
	bool IsTagProhibitPreference() const ;
	bool IsTagBacktraceResult() const ;

	char const* GetBacktraceAlreadyBacktraced() const ;
	char const* GetBacktraceSymbol1() const	;
	char const* GetBacktraceSymbol2() const	;

	// Numeric indifferent preferences
	bool IsTagCandidate() const	;
	char const* GetCandidateName() const ;
	char const* GetCandidateType() const ;
	char const* GetCandidateValue() const ;
	char const* GetCandidateExpValue() const ;

	// Warnings, errors, messages and other tags
	bool IsTagError() const	;
	bool IsTagWarning() const ;
	bool IsTagMessage() const ;
	bool IsTagVerbose() const ;
} ;

} //closes namespace

#endif //SML_CLIENT_TRACE_XML_H
