/////////////////////////////////////////////////////////////////
// TagFilter class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : April 2006
//
// Represents a command that is being sent/received by a command line filter.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_TAG_FILTER_H
#define SML_TAG_FILTER_H

#include "sml_ElementXML.h"

class TagFilter : public sml::ElementXML
{
public:
	TagFilter(void) ;

	void SetCommand(char const* pCommandLine) ;
};

#endif	// SML_TAG_FILTER_H
