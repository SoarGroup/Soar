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

	// Firing-production tag, contains production
	bool IsTagFiringProduction() const ;
	bool IsTagRetractingProduction() const ;

	// Production
	bool IsTagProduction() const ;
	char const* GetProductionName() const ;

	// Add-wme contains wme
	bool IsTagAddWme() const ;
	bool IsTagRemoveWme() const ;

	// Wme tag attributes
	bool IsTagWme() const ;
	char const* GetWmeID() const ;
	char const* GetWmeAttribute() const ;
	char const* GetWmeValue() const ;
	char const* GetWmeTimeTag() const ;

	// Preference tag
	bool IsTagPreference() const ;
	char const* GetPreferenceID() const ;
	char const* GetPreferenceAttribute() const ;
	char const* GetPreferenceValue() const ;
	char const* GetPreferenceType() const ;
	char const* GetPreferenceTimeTag() const ;

} ;

} //closes namespace

#endif //SML_CLIENT_TRACE_XML_H
