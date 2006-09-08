#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

/////////////////////////////////////////////////////////////////
// TagWme class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// Represents an SML node which represents an argument for a command and has
// the tag <wme>.
//
/////////////////////////////////////////////////////////////////

#include "sml_TagWme.h"

using namespace sml ;

TagWme::TagWme()
{
	this->SetTagNameFast(sml_Names::kTagWME) ;
}
