/////////////////////////////////////////////////////////////////////////////
//WME class
//
//Author:	Cory Dunham, Bob Marinier, Devvan Stokes, University of Michigan
//Date:		August 2004
//
//WME is a client-side representation of the identifier/attribute/value
//triples that exist in Soar's working memory
/////////////////////////////////////////////////////////////////////////////

#ifndef SML_WME_H
#define SML_WME_H

#include "sml_ClientSymbol.h"
#include "sml_ClientError.h"
#include "sml_ClientIWME.h"

namespace sml {
class ClientSML ;

class WME : public IWme
{

public:
	WME(char const* pID, ClientSML* pClientSML);
	
//	WME(sml::IdentifierSymbol* id, sml::ISymbol* attrib, sml::ISymbol* val);
	
	virtual ~WME();

	virtual void Release(gSKI::Error *err = 0) ;

//	void SetAttribute(sml::ISymbol* attribute);

//	void SetValue(sml::ISymbol* value);

//	const sml::IdentifierSymbol* GetIdentifier() const;

//	const sml::ISymbol* GetAttribute() const;

	const sml::ISymbol* GetValue() const;

//	unsigned long GetTimeTag() const;

private:
/*
	unsigned long timeTag;

	sml::IdentifierSymbol* identifier;
	
	sml::ISymbol* attribute;

	sml::ISymbol* value;
*/
};
}//closes namespace

#endif //SML_WME_H