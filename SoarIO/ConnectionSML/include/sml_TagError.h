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

#ifndef SML_TAG_ERROR_H
#define SML_TAG_ERROR_H

#include "sml_ElementXML.h"
#include "sml_StringOps.h"
#include "sml_Names.h"

namespace sml {

class TagError : public ElementXML
{
public:
	TagError(void);
	~TagError(void);

	void SetDescription(char const* pErrorMsg)
	{
		this->SetCharacterData(pErrorMsg) ;
	}

	void SetErrorCode(int error)
	{
		char errorBuffer[kMinBufferSize] ;
		Int2String(error, errorBuffer, kMinBufferSize) ;

		this->AddAttributeFast(sml_Names::kErrorCode, errorBuffer) ;
	}

};

}

#endif	// SML_TAG_ERROR_H
