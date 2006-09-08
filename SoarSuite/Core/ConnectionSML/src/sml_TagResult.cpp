#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

/////////////////////////////////////////////////////////////////
// TagResult class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Represents an SML node which represents the result of a command with
// the tag <result>.
//
/////////////////////////////////////////////////////////////////

#include "sml_TagResult.h"

using namespace sml ;

TagResult::TagResult(void)
{
	SetTagNameFast(sml_Names::kTagResult) ;
}

TagResult::~TagResult(void)
{
}
