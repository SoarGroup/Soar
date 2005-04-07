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

// Firing-production tag, contains production
bool ClientTraceXML::IsTagFiringProduction() const			{ return IsTag(sml_Names::kTagProduction_Firing) ; }
bool ClientTraceXML::IsTagRetractingProduction() const		{ return IsTag(sml_Names::kTagProduction_Retracting) ; }

// Production
bool ClientTraceXML::IsTagProduction() const				{ return IsTag(sml_Names::kTagProduction) ; }
char const* ClientTraceXML::GetProductionName() const		{ return GetAttribute(sml_Names::kProduction_Name) ; }

// Add wme attributes
bool ClientTraceXML::IsTagAddWme() const					{ return IsTag(sml_Names::kTagWMEAdd) ; }
bool ClientTraceXML::IsTagRemoveWme() const					{ return IsTag(sml_Names::kTagWMERemove) ; }

// Wme attributes
bool ClientTraceXML::IsTagWme() const						{ return IsTag(sml_Names::kTagWME) ; }
char const* ClientTraceXML::GetWmeID() const				{ return GetAttribute(sml_Names::kWME_Id) ; }
char const* ClientTraceXML::GetWmeAttribute() const			{ return GetAttribute(sml_Names::kWME_Attribute) ; }
char const* ClientTraceXML::GetWmeValue() const				{ return GetAttribute(sml_Names::kWME_Value) ; }
char const* ClientTraceXML::GetWmeTimeTag() const			{ return GetAttribute(sml_Names::kWME_TimeTag) ; }

// Preference tag
bool ClientTraceXML::IsTagPreference() const				{ return IsTag(sml_Names::kTagPreference) ; }
char const* ClientTraceXML::GetPreferenceID() const			{ return GetAttribute(sml_Names::kWME_Id) ; }
char const* ClientTraceXML::GetPreferenceAttribute() const	{ return GetAttribute(sml_Names::kWME_Attribute) ; }
char const* ClientTraceXML::GetPreferenceValue() const		{ return GetAttribute(sml_Names::kWME_Value) ; }
char const* ClientTraceXML::GetPreferenceTimeTag() const	{ return GetAttribute(sml_Names::kWME_TimeTag) ; }
char const* ClientTraceXML::GetPreferenceType() const		{ return GetAttribute(sml_Names::kPreference_Type) ; }
