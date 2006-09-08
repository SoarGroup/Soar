#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

/////////////////////////////////////////////////////////////////
// OutputDeltaList class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class records the list of changes that have
// occured to the output-link since the client
// last asked for them.
//
/////////////////////////////////////////////////////////////////

#include "sml_OutputDeltaList.h"
#include "sml_ClientWMElement.h"

using namespace sml ;

WMDelta::~WMDelta()
{
	// If this is an item that's been removed then we own it.
	// (Others are still attached to the output link).
	if (m_ChangeType == kRemoved)
		delete m_pWME ;
}
