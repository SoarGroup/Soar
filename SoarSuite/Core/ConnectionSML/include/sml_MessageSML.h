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

#ifndef SML_NODESMLH
#define SML_NODESMLH

#include "sml_ElementXML.h"
#include "sml_StringOps.h"
#include "sml_Names.h"

namespace sml
{

class MessageSML : public ElementXML
{
public:
	enum DocType { kCall = 0, kResponse = 1, kNotify = 2 } ;

	MessageSML() ;
	MessageSML(DocType type, int id) ;

	virtual ~MessageSML() { }

	char const* GetID()
	{
		char const* pID = this->GetAttribute(sml_Names::kID) ;
		return pID ;
	}

	void SetID(int id)
	{
		char buffer[kMinBufferSize] ;
		Int2String(id, buffer, kMinBufferSize) ;

		this->AddAttributeFast(sml_Names::kID, buffer) ;
	}

	void SetDocType(char const* pType)
	{
		this->AddAttributeFast(sml_Names::kDocType, CopyString(pType), false) ;
	}

	char const* GetDocType()
	{
		char const* pDocType = this->GetAttribute(sml_Names::kDocType) ;
		return pDocType ;
	}

	bool IsCall()
	{
		char const* pDocType = GetDocType() ;
		return IsStringEqual(sml_Names::kDocType_Call, pDocType) ;
	}

	bool IsResponse()
	{
		char const* pDocType = GetDocType() ;
		return IsStringEqual(sml_Names::kDocType_Response, pDocType) ;
	}

	bool IsNotify()
	{
		char const* pDocType = GetDocType() ;
		return IsStringEqual(sml_Names::kDocType_Notify, pDocType) ;
	}

} ;

}

#endif	// SML_NODESMLH
