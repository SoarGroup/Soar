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
bool ClientTraceXML::IsTagState() const { return IsTag(sml_Names::kTagState) ; }

// State tag attributes
char const* ClientTraceXML::GetDecisionCycleCount() const	{ return GetAttribute(sml_Names::kState_DecisionCycleCt) ; }
char const* ClientTraceXML::GetStateID() const				{ return GetAttribute(sml_Names::kState_ID) ; }
char const* ClientTraceXML::GetImpasseObject() const		{ return GetAttribute(sml_Names::kState_ImpasseObject) ; }
char const* ClientTraceXML::GetImpasseType() const 			{ return GetAttribute(sml_Names::kState_ImpasseType) ; }

// Phase tag attributes
bool ClientTraceXML::IsTagPhase() const						{ return IsTag(sml_Names::kTagPhase) ; }
char const* ClientTraceXML::GetPhaseName() const			{ return GetAttribute(sml_Names::kPhase_Name) ; }
char const* ClientTraceXML::GetPhaseStatus() const			{ return GetAttribute(sml_Names::kPhase_Status) ; }

// Add wme attributes
bool ClientTraceXML::IsTagAddWme() const					{ return IsTag(sml_Names::kTagWMEAdd) ; }

// Wme attributes
bool ClientTraceXML::IsTagWme() const						{ return IsTag(sml_Names::kTagWME) ; }
char const* ClientTraceXML::GetWmeID() const				{ return GetAttribute(sml_Names::kWME_Id) ; }
char const* ClientTraceXML::GetWmeAttribute() const			{ return GetAttribute(sml_Names::kWME_Attribute) ; }
char const* ClientTraceXML::GetWmeValue() const				{ return GetAttribute(sml_Names::kWME_Attribute) ; }
