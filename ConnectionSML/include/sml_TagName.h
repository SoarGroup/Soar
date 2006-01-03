/////////////////////////////////////////////////////////////////
// TagName class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Represents an SML node which represents a named object and
// has the tag <name>
//
/////////////////////////////////////////////////////////////////

#ifndef SML_TAG_NAME_H
#define SML_TAG_NAME_H

#include "sml_ElementXML.h"
#include "sml_Names.h"

namespace sml
{

class TagName : public ElementXML
{
public:
	TagName() ;

	void SetName(char const* pName)
	{
		this->SetCharacterData(pName) ;
	}
} ;

}

#endif	// SML_TAG_NAME_H
