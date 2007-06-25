#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

/////////////////////////////////////////////////////////////////
// TagFilter class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : April 2006
//
// Represents a command that is being sent/received by a command line filter.
//
/////////////////////////////////////////////////////////////////

#include "sml_TagFilter.h"
#include "sml_Names.h"

using namespace sml ;
	
TagFilter::TagFilter()
{
	this->SetTagNameFast(sml_Names::kTagFilter) ;
}

void TagFilter::SetCommand(char const* pCommandLine)
{
	this->AddAttributeFast(sml_Names::kFilterCommand, pCommandLine) ;
}
