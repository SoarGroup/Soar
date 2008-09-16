#include <portability.h>

/////////////////////////////////////////////////////////////////
// TagCommand class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Represents an SML node which represents a command and has
// the tag <command>.
//
/////////////////////////////////////////////////////////////////

#include "sml_TagCommand.h"

using namespace sml ;

TagCommand::TagCommand()
{
	SetTagNameFast(sml_Names::kTagCommand) ;
}
