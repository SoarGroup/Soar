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
char const* ClientTraceXML::GetDecisionCycleCount() const { return GetAttribute(sml_Names::kState_DecisionCycleCt) ; }
char const* ClientTraceXML::GetStateID() const			  { return GetAttribute(sml_Names::kState_ID) ; }
char const* ClientTraceXML::GetImpasseObject() const 	  { return GetAttribute(sml_Names::kState_ImpasseObject) ; }
char const* ClientTraceXML::GetImpasseType() const 		  { return GetAttribute(sml_Names::kState_ImpasseType) ; }
