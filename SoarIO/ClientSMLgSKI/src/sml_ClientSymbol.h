/////////////////////////////////////////////////////////////////////////////
// Symbol classes
//
// Author: Bob Marinier, Cory Dunham, Devvan Stokes, University of Michigan
// Date  : August 2004
//
// These symbol classes wrap up the strings (and identifiers), floats, and 
// ints that WMEs are composed of. 
/////////////////////////////////////////////////////////////////////////////

#ifndef SML_SYMBOL_H
#define SML_SYMBOL_H

#include "sml_ClientWMObject.h"
#include "sml_ClientISymbol.h"

namespace sml
{

class WME;

class Symbol : public ISymbol
{
public:
	Symbol(char const* pID, ClientSML* pClientSML) ;

	virtual ~Symbol() { } ;

	virtual const char* GetString(gSKI::Error* err = 0) const ;
      
	virtual sml::IWMObject* GetObject(gSKI::Error* err = 0) const ;

	virtual int GetInt(gSKI::Error* err = 0) const ;

	virtual void Release(gSKI::Error* err = 0) ;

protected:
	std::string m_StringValue ;
} ;

/*
class StringSymbol : public ISymbol
{
	char* internalString;
public:
	StringSymbol(char* inString);
	~StringSymbol();
	const char* GetSymbol() const;
};

//This inheritance structure allows Indentifier to be treated as a Symbol while
//inheriting its implementation from String
class IdentifierSymbol : virtual private StringSymbol, virtual public ISymbol
{
public:
	IdentifierSymbol(char* inIdentifier) : StringSymbol(inIdentifier){}
};


class FloatSymbol : public ISymbol
{
	float internalFloat;

public:	
	~FloatSymbol();
	FloatSymbol(float inFloat);
	float GetSymbol() const;
};


class IntSymbol : public ISymbol
{
	int internalInt;

public:
	IntSymbol(int inInteger);
	~IntSymbol();
	int GetSymbol() const;
};
*/
}//closes namespace

#endif //SML_SYMBOL_H