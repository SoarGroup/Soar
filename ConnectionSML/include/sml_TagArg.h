/////////////////////////////////////////////////////////////////
// TagArg class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Represents an SML node which represents an argument for a command and has
// the tag <arg>.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_TAG_ARG_H
#define SML_TAG_ARG_H

#include "sml_ElementXML.h"
#include "sml_StringOps.h"
#include "sml_Names.h"

namespace sml
{

class TagArg : public ElementXML
{
public:
	TagArg() ;

	void SetParam(char const* pName)
	{
		this->AddAttributeFast(sml_Names::kArgParam, CopyString(pName), false) ;
	}

	// NOTE: Be careful with this one.  If you call this, you must keep pName in scope
	// for the life of this object, which generally means it needs to be a static constant.
	void SetParamFast(char const* pName)
	{
		this->AddAttributeFastFast(sml_Names::kArgParam, pName) ;
	}

	void SetType(char const* pType)
	{
		this->AddAttributeFast(sml_Names::kArgType, CopyString(pType), false) ;
	}

	// NOTE: Be careful with this one.  If you call this, you must keep pType in scope
	// for the life of this object, which generally means it needs to be a static constant.
	void SetTypeFast(char const* pType)
	{
		this->AddAttributeFastFast(sml_Names::kArgType, pType) ;
	}

	void SetValue(char const* pValue)
	{
		this->SetCharacterData(pValue) ;
	}
} ;

}

#endif	// SML_TAG_COMMAND
