/////////////////////////////////////////////////////////////////////////////
// Symbol classes
//
// Author: Bob Marinier, Cory Dunham, Devvan Stokes, University of Michigan
// Date  : August 2004
//
// These symbol classes wrap up the strings (and identifiers), floats, and 
// ints that WMEs are composed of. 
/////////////////////////////////////////////////////////////////////////////

#ifndef SML_ISYMBOL_H
#define SML_ISYMBOL_H

#include "sml_ClientObject.h"
#include "sml_ClientIRelease.h"

namespace sml
{
class IWMObject ;

class ISymbol : public IRelease
{
public:
	virtual ~ISymbol() { } ;

	virtual const char* GetString(gSKI::Error* err = 0) const = 0;
      
	virtual sml::IWMObject* GetObject(gSKI::Error* err = 0) const = 0;

	virtual int GetInt(gSKI::Error* err = 0) const = 0;
};

}//closes namespace

#endif //SML_ISYMBOL_H