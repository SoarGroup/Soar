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

#ifndef SML_TAG_COMMANDH
#define SML_TAG_COMMANDH

#include "sml_ElementXML.h"
#include "sml_StringOps.h"
#include "sml_Names.h"

namespace sml
{

class TagCommand : public ElementXML
{
public:
	TagCommand() ;

	char const* GetName()
	{
		char const* pID = this->GetAttribute(sml_Names::kCommandName) ;
		return pID ;
	}

	void SetName(char const* pName)
	{
		this->AddAttributeFast(sml_Names::kCommandName, CopyString(pName), false) ;
	}

	// NOTE: Be careful with this one.  If you call this, you must keep pName in scope
	// for the life of this object, which generally means it needs to be a static constant.
	void SetNameFast(char const* pName)
	{
		this->AddAttributeFastFast(sml_Names::kCommandName, pName) ;
	}
} ;

}

#endif	// SML_TAG_COMMAND
