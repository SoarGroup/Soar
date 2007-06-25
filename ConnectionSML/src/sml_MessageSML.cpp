#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

/////////////////////////////////////////////////////////////////
// MessageSML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Represents an SML node which is the top-level document in an SML message
// and has the tag <sml>.
//
/////////////////////////////////////////////////////////////////

#include "sml_MessageSML.h"

using namespace sml ;

MessageSML::MessageSML()
{
	SetTagNameFast(sml_Names::kTagSML) ;
	AddAttributeFastFast(sml_Names::kSoarVersion, sml_Names::kSoarVersionValue) ;
	AddAttributeFastFast(sml_Names::kSMLVersion,  sml_Names::kSMLVersionValue) ;
}

MessageSML::MessageSML(DocType type, int id)
{
	SetTagNameFast(sml_Names::kTagSML) ;
	AddAttributeFastFast(sml_Names::kSoarVersion, sml_Names::kSoarVersionValue) ;
	AddAttributeFastFast(sml_Names::kSMLVersion,  sml_Names::kSMLVersionValue) ;

	char const* pDocType = sml_Names::kDocType_Call ;
	if (type == kResponse)		pDocType = sml_Names::kDocType_Response ;
	else if (type == kNotify)	pDocType = sml_Names::kDocType_Notify ;

	// Note: This version requires that pDocType never go out of scope, which is fine
	// as long as we set it to a static constant as we've done here.  Do not change this to
	// accept a string from the user without changing the call to not be to "FastFast" method.
	AddAttributeFastFast(sml_Names::kDocType, pDocType) ;

	// This is the only place where we need to allocate a string.  Up to here, the rest is
	// setting pointers to constants, so very fast.
	SetID(id) ;
}
