/////////////////////////////////////////////////////////////////////////////
//WME class
//
//Author:	Cory Dunham, Bob Marinier, Devvan Stokes, University of Michigan
//Date:		August 2004
//
//WME is a client-side representation of the identifier/attribute/value
//triples that exist in Soar's working memory
/////////////////////////////////////////////////////////////////////////////

#ifndef SML_IWME_H
#define SML_IWME_H

#include "sml_ClientObject.h"
#include "sml_ClientIRelease.h"

enum sml_WMEErrorCode
{
	WME_ERROR_NONE = 0,	
	//WME_ERROR_OTHERSGOHERE
};

namespace sml {

class ISymbol ;

class IWme : public IRelease
{
public:
	virtual ~IWme() { } ;

//	virtual void SetAttribute(ISymbol* attribute) = 0 ;

//	virtual void SetValue(ISymbol* value) = 0;

//	virtual const ISymbol* GetIdentifier() const = 0;

//	virtual const ISymbol* GetAttribute() const = 0;

	virtual const ISymbol* GetValue() const = 0;

//	virtual unsigned long GetTimeTag() const = 0;

};
}//closes namespace

#endif //SML_WME_H