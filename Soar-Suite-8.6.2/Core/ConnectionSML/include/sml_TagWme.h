/////////////////////////////////////////////////////////////////
// TagWme class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// Represents an SML node which represents an argument for a command and has
// the tag <wme>.
//
// A WME is (id, att, value)
// plus it can have a time tag (unique int identifier)
// and type information for the value (which defaults to string)
// Can also specify an action (e.g. add or remove)
//
/////////////////////////////////////////////////////////////////

#ifndef SML_TAG_WME_H
#define SML_TAG_WME_H

#include "sml_ElementXML.h"
#include "sml_StringOps.h"
#include "sml_Names.h"

namespace sml
{

class TagWme : public ElementXML
{
public:
	TagWme() ;

	void SetIdentifier(char const* pIdentifier)
	{
		this->AddAttributeFast(sml_Names::kWME_Id, CopyString(pIdentifier), false) ;
	}

	void SetAttribute(char const* pAttribute)
	{
		this->AddAttributeFast(sml_Names::kWME_Attribute, CopyString(pAttribute), false) ;
	}

	void SetValue(char const* pValue, char const* pType)
	{
		this->AddAttributeFast(sml_Names::kWME_Value, CopyString(pValue), false) ;

		// The string type is the default, so we don't need to add it to the object
		// We do a direct pointer comparison here for speed, so if the user passes in "string" without using
		// sml_Names, we'll add it to the list of attributes (which does no harm).  This all just saves a little time.
		if (pType && pType != sml_Names::kTypeString)
			this->AddAttributeFast(sml_Names::kWME_ValueType, CopyString(pType), false) ;
	}

	void SetTimeTag(long timeTag)
	{
		char buffer[kMinBufferSize] ;
		Int2String(timeTag, buffer, sizeof(buffer)) ;

		this->AddAttributeFast(sml_Names::kWME_TimeTag, CopyString(buffer), false) ;
	}

	void SetActionAdd()
	{
		this->AddAttributeFastFast(sml_Names::kWME_Action, sml_Names::kValueAdd) ;
	}

	void SetActionRemove()
	{
		this->AddAttributeFastFast(sml_Names::kWME_Action, sml_Names::kValueRemove) ;
	}

} ;

}

#endif	// SML_TAG_WME
