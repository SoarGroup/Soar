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

	// State tag attributes
	bool IsTagState() const ;
	char const* GetDecisionCycleCount() const ;
	char const* GetStateID() const ;
	char const* GetImpasseObject() const ;
	char const* GetImpasseType() const ;

	// Phase tag attributes
	bool IsTagPhase() const ;
	char const* GetPhaseName() const ;
	char const* GetPhaseStatus() const ;

	// Add-wme contains wme
	bool IsTagAddWme() const ;

	// Wme tag attributes
	bool IsTagWme() const ;
	char const* GetWmeID() const ;
	char const* GetWmeAttribute() const ;
	char const* GetWmeValue() const ;
} ;

} //closes namespace

#endif //SML_CLIENT_TRACE_XML_H
