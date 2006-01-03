#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// TagError class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Represents an SML node which represents an error in a result with
// the tag <error>.
//
/////////////////////////////////////////////////////////////////

#include "sml_TagError.h"

using namespace sml ;

TagError::TagError(void)
{
	SetTagNameFast(sml_Names::kTagError) ;
}

TagError::~TagError(void)
{
}
