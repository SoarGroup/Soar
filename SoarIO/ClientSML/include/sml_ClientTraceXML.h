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
	bool IsTagState() const ;

	// State tag attributes
	char const* GetDecisionCycleCount() const ;
	char const* GetStateID() const ;
	char const* GetImpasseObject() const ;
	char const* GetImpasseType() const ;

} ;

} //closes namespace

#endif //SML_CLIENT_TRACE_XML_H
