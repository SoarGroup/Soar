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

#include "sml_ClientTraceXML.h"
#include "sml_Names.h"

using namespace sml ;

// These methods provide access to specific attributes and tags without
// the client needing to pass in/know the strings.  They're all very simple.

// Trace tag contains everything else
bool ClientTraceXML::IsTagTrace() const						{ return IsTag(sml_Names::kTagTrace) ; } 

// Write commands on right hand side of productions generate output
// which are collected here.
bool ClientTraceXML::IsTagRhsWrite() const					{ return IsTag(sml_Names::kTagRHS_write) ; }
char const* ClientTraceXML::GetString() const				{ return GetAttribute(sml_Names::kRHS_String) ; }

// State tag attributes
bool ClientTraceXML::IsTagState() const						{ return IsTag(sml_Names::kTagState) ; }
char const* ClientTraceXML::GetDecisionCycleCount() const	{ return GetAttribute(sml_Names::kState_DecisionCycleCt) ; }
char const* ClientTraceXML::GetStateID() const				{ return GetAttribute(sml_Names::kState_ID) ; }
char const* ClientTraceXML::GetImpasseObject() const		{ return GetAttribute(sml_Names::kState_ImpasseObject) ; }
char const* ClientTraceXML::GetImpasseType() const 			{ return GetAttribute(sml_Names::kState_ImpasseType) ; }
char const* ClientTraceXML::GetStackLevel() const			{ return GetAttribute(sml_Names::kState_StackLevel) ; }

// Operator tag attributes
bool ClientTraceXML::IsTagOperator() const					{ return IsTag(sml_Names::kTagOperator) ; }
char const* ClientTraceXML::GetOperatorID() const			{ return GetAttribute(sml_Names::kOperator_ID) ; }
char const* ClientTraceXML::GetOperatorName() const			{ return GetAttribute(sml_Names::kOperator_Name) ; }

// Phase tag attributes
bool ClientTraceXML::IsTagPhase() const						{ return IsTag(sml_Names::kTagPhase) ; }
char const* ClientTraceXML::GetPhaseName() const			{ return GetAttribute(sml_Names::kPhase_Name) ; }
char const* ClientTraceXML::GetPhaseStatus() const			{ return GetAttribute(sml_Names::kPhase_Status) ; }
char const* ClientTraceXML::GetFiringType() const			{ return GetAttribute(sml_Names::kPhase_FiringType) ; }

// Subphase (firing productions/changing wm) tag
bool ClientTraceXML::IsTagSubphase() const							{ return IsTag(sml_Names::kTagSubphase) ; }
bool ClientTraceXML::IsSubphaseNameFiringProductions() const		{ char const* pName = GetPhaseName() ; return (pName != NULL) && !strcmp(sml_Names::kSubphaseName_FiringProductions, pName) ; }
bool ClientTraceXML::IsSubphaseNameChangingWorkingMemory() const	{ char const* pName = GetPhaseName() ; return (pName != NULL) && !strcmp(sml_Names::kSubphaseName_ChangingWorkingMemory, pName) ; }

// Firing-production tag, contains production
bool ClientTraceXML::IsTagFiringProduction() const			{ return IsTag(sml_Names::kTagProduction_Firing) ; }
bool ClientTraceXML::IsTagRetractingProduction() const		{ return IsTag(sml_Names::kTagProduction_Retracting) ; }
bool ClientTraceXML::IsTagLearning() const					{ return IsTag(sml_Names::kTagLearning) ; }

// Production
/*
<production prod_name="my*prod" documentation="my doc string" 
type="[:chunk|:default|:justification ;# not reloadable]" 
declared-support="[:i-support|:o-support]">
   <conditions>
      <conjunctive-negation-condition>
         <condition test="[state|impasse]" id="<s1>" condition="^foo bar ^hello <world>"></condition>
         ...
      </conjunctive-negation-condition>
      <condition ...></condition>
      ...
   </conditions>
   <actions>
      <action id="<s1>" action="^foo2 bar2 ^what ever"></action>
      <action function="some function string"></action>
   </actions>
</production>
*/
bool ClientTraceXML::IsTagProduction() const				{ return IsTag(sml_Names::kTagProduction) ; }
char const* ClientTraceXML::GetProductionName() const		{ return GetAttribute(sml_Names::kProduction_Name) ; }
char const* ClientTraceXML::GetProductionDoc() const		{ return GetAttribute(sml_Names::kProductionDocumentation) ; }
char const* ClientTraceXML::GetProductionType() const		{ return GetAttribute(sml_Names::kProductionType) ; }
char const* ClientTraceXML::GetProductionDeclaredSupport() const	{ return GetAttribute(sml_Names::kProductionDeclaredSupport) ; }

bool ClientTraceXML::IsTagConditions() const				{ return IsTag(sml_Names::kTagConditions) ; }
bool ClientTraceXML::IsTagCondition() const					{ return IsTag(sml_Names::kTagCondition) ; }
bool ClientTraceXML::IsTagConjunctiveNegationCondition() const	{ return IsTag(sml_Names::kTagConjunctive_Negation_Condition) ; }
bool ClientTraceXML::IsTagActions() const					{ return IsTag(sml_Names::kTagActions) ; }
bool ClientTraceXML::IsTagAction() const					{ return IsTag(sml_Names::kTagAction) ; }

// Condition attributes: Note this form allows for multiple unparsed conditions within the "condition" attribute.
char const* ClientTraceXML::GetConditionTest() const		{ return GetAttribute(sml_Names::kConditionTest) ; }
char const* ClientTraceXML::GetConditionId() const			{ return GetAttribute(sml_Names::kConditionId) ; }
char const* ClientTraceXML::GetCondition() const			{ return GetAttribute(sml_Names::kCondition) ; }

// Action attributes
char const* ClientTraceXML::GetActionId() const			{ return GetAttribute(sml_Names::kActionId) ; }
char const* ClientTraceXML::GetAction() const			{ return GetAttribute(sml_Names::kAction) ; }
char const* ClientTraceXML::GetFunction() const			{ return GetAttribute(sml_Names::kActionFunction) ; }

// Add wme attributes
bool ClientTraceXML::IsTagAddWme() const					{ return IsTag(sml_Names::kTagWMEAdd) ; }
bool ClientTraceXML::IsTagRemoveWme() const					{ return IsTag(sml_Names::kTagWMERemove) ; }

// Wme attributes
bool ClientTraceXML::IsTagWme() const						{ return IsTag(sml_Names::kTagWME) ; }
char const* ClientTraceXML::GetWmeID() const				{ return GetAttribute(sml_Names::kWME_Id) ; }
char const* ClientTraceXML::GetWmeAttribute() const			{ return GetAttribute(sml_Names::kWME_Attribute) ; }
char const* ClientTraceXML::GetWmeValue() const				{ return GetAttribute(sml_Names::kWME_Value) ; }
char const* ClientTraceXML::GetWmeTimeTag() const			{ return GetAttribute(sml_Names::kWME_TimeTag) ; }
char const* ClientTraceXML::GetWmePreference() const		{ return GetAttribute(sml_Names::kWME_Preference) ; }

// Preference tag
bool ClientTraceXML::IsTagPreference() const				{ return IsTag(sml_Names::kTagPreference) ; }
char const* ClientTraceXML::GetPreferenceID() const			{ return GetAttribute(sml_Names::kWME_Id) ; }
char const* ClientTraceXML::GetPreferenceAttribute() const	{ return GetAttribute(sml_Names::kWME_Attribute) ; }
char const* ClientTraceXML::GetPreferenceValue() const		{ return GetAttribute(sml_Names::kWME_Value) ; }
char const* ClientTraceXML::GetPreferenceTimeTag() const	{ return GetAttribute(sml_Names::kWME_TimeTag) ; }
char const* ClientTraceXML::GetPreferenceType() const		{ return GetAttribute(sml_Names::kPreference_Type) ; }
char const* ClientTraceXML::GetPreferenceOSupported() const	{ return GetAttribute(sml_Names::kOSupported) ; }
// For binary prefs the other object
char const* ClientTraceXML::GetPreferenceReferent() const	{ return GetAttribute(sml_Names::kReferent) ; }

// Marker between LHS matches and RHS results in trace
bool ClientTraceXML::IsTagActionSideMarker() const			{ return IsTag(sml_Names::kTagActionSideMarker) ; }

// Backtrace tags
/*
1) the backtrace for each of the results
2) a trace of the "locals"
3) a trace of the grounded potentials
4) a trace of the ungrounded potentials

<local>
<backtrace prod_name="my*name" already-backtraced="true">
   <grounds>
      <wme></wme>
      ...
   </grounds>
   <potentials>
      <wme></wme>
      ...
   </potentials>
   <locals>
      <wme></wme>
      ...
   </locals>
   <negated>
      <condition></condition>
      ...
   </negated>
   <nots>
      <not symbol1="s1" symbol2="s2"></not>
      ...
   </nots>
</backtrace>
</local>

The not tags get printed as "s1 <> s2" in the text trace.

*** locals (#2) ***

<locals>
  <local>
    <wme></wme>
    <backtrace>...</backtrace>
    <prohibit-preference>
      <preference></preference>
      <backtrace>...</backtrace>
    </prohibit-preference>
    -OR-
    <add-to-potentials></add-to-potentials>
  </local>
  <local>...</local>
  ...
</locals>

There might be a prohibit-preference or an add-to-potentials, but not both. 
Also, if there is no backtrace child of local, then the print trace outputs "...no trace, can't BT". 
Add-to-potentials is an empty tag just to mark when the text trace prints " --> make it a potential."

*** trace grounded potentials (#3) ***

<grounded-potentials>
   <wme></wme>
   ...
</grounded-potentials>

*** trace ungrounded potentials (#4) ***

<ungrounded-potentials>
   <ungrounded-potential>
      <wme></wme>
      <backtrace>...</backtrace>
      <prohibit-preference>
         <preference></preference>
         <backtrace>...</backtrace>
      <prohibit-preference>
   </ungrounded-potential>
   <ungrounded-potential>...</ungrounded-potential>
   ...
</ungrounded-potentials>
*/

bool ClientTraceXML::IsTagLocal() const					{ return IsTag(sml_Names::kTagLocal) ; }
bool ClientTraceXML::IsTagLocals() const				{ return IsTag(sml_Names::kTagLocals) ; }
bool ClientTraceXML::IsTagGrounds() const				{ return IsTag(sml_Names::kTagGrounds) ; }
bool ClientTraceXML::IsTagNegated() const				{ return IsTag(sml_Names::kTagNegated) ; }
bool ClientTraceXML::IsTagNot() const					{ return IsTag(sml_Names::kTagNot) ; }
bool ClientTraceXML::IsTagNots() const					{ return IsTag(sml_Names::kTagNots) ; }
bool ClientTraceXML::IsTagPotentials() const			{ return IsTag(sml_Names::kTagPotentials) ; }
bool ClientTraceXML::IsTagGroundedPotentials() const	{ return IsTag(sml_Names::kTagGroundedPotentials) ; }
bool ClientTraceXML::IsTagUngroundedPotentials() const	{ return IsTag(sml_Names::kTagUngroundedPotentials) ; }
bool ClientTraceXML::IsTagUngroundedPotential() const	{ return IsTag(sml_Names::kTagUngroundedPotential) ; }
bool ClientTraceXML::IsTagBacktrace() const				{ return IsTag(sml_Names::kTagBacktrace) ; }
bool ClientTraceXML::IsTagAddToPotentials() const		{ return IsTag(sml_Names::kTagAddToPotentials) ; }
bool ClientTraceXML::IsTagProhibitPreference() const	{ return IsTag(sml_Names::kTagProhibitPreference) ; }
bool ClientTraceXML::IsTagBacktraceResult() const		{ return IsTag(sml_Names::kTagBacktraceResult) ; }

char const* ClientTraceXML::GetBacktraceAlreadyBacktraced() const { return GetAttribute(sml_Names::kBacktracedAlready) ; }
char const* ClientTraceXML::GetBacktraceSymbol1() const			  { return GetAttribute(sml_Names::kBacktraceSymbol1) ; }
char const* ClientTraceXML::GetBacktraceSymbol2() const			  { return GetAttribute(sml_Names::kBacktraceSymbol2) ; }

// Numeric indifferent preferences
//<candidate name="O1" type="[sum|avg]" value="123.4"></candidate>
//The text output looks something like:
//Candidate O1:   Value (Avg) = 123.4
bool ClientTraceXML::IsTagCandidate() const				{ return IsTag(sml_Names::kTagCandidate) ; }
char const* ClientTraceXML::GetCandidateName() const	{ return GetAttribute(sml_Names::kCandidateName) ; }
char const* ClientTraceXML::GetCandidateType() const	{ return GetAttribute(sml_Names::kCandidateType) ; }
char const* ClientTraceXML::GetCandidateValue() const	{ return GetAttribute(sml_Names::kCandidateValue) ; }

// Warnings, errors, messages and other tags
bool ClientTraceXML::IsTagError() const				{ return IsTag(sml_Names::kTagError) ; }
bool ClientTraceXML::IsTagWarning() const			{ return IsTag(sml_Names::kTagWarning) ; }
bool ClientTraceXML::IsTagMessage() const			{ return IsTag(sml_Names::kTagMessage) ; }
bool ClientTraceXML::IsTagVerbose() const			{ return IsTag(sml_Names::kTagVerbose) ; }
