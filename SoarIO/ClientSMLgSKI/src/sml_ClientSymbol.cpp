#include "sml_ClientSymbol.h"
#include "sml_ClientRelease.h"
#include "sml_Connection.h"

using namespace sml;

Symbol::Symbol(char const* pID, ClientSML* pClientSML)
{
	SetId(pID) ;
	SetClientSML(pClientSML) ;
}

const char* Symbol::GetString(gSKI::Error* err) const
{
	AnalyzeXML response ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_ISymbol_GetString, GetId()))
	{
		// This is a bit nasty.  We have to declare the function as "const" to be consistent with
		// the gSKI interface (changing this would break a lot of client code) but we also need
		// to own the string we return (the client doesn't release it), so we need to cast away
		// our const here and then modify our internal data even though we said we wouldn't.
		Symbol* pThis = (Symbol*)this ;
		pThis->m_StringValue = response.GetResultString() ;
	}

	return m_StringValue.c_str() ;
}

IWMObject* Symbol::GetObject(gSKI::Error* err) const
{
	AnalyzeXML response ;
	IWMObject* pResult = NULL ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_ISymbol_GetObject, GetId()))
	{
		pResult = new WMObject(response.GetResultString(), GetClientSML()) ;
	}

	return pResult ;
}

int Symbol::GetInt(gSKI::Error* err) const
{
	AnalyzeXML response ;
	int result = 0 ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_ISymbol_GetInt, GetId()))
	{
		result = response.GetResultInt(0) ;
	}

	return result ;
}

void Symbol::Release(gSKI::Error* err)
{
	Release::ReleaseObject(this) ;

	delete this ;
}

/*
StringSymbol::StringSymbol(char* inString)
{
	this->internalString = StringCopy(inString) ;
}

StringSymbol::~StringSymbol()
{
	delete [] internalString;
	internalString = 0;
}

const char* StringSymbol::GetSymbol() const {	return internalString;}




FloatSymbol::FloatSymbol(float inFloat) :  internalFloat(inFloat) {}
	
FloatSymbol::~FloatSymbol(){}

float FloatSymbol::GetSymbol() const {	return internalFloat;}



IntSymbol::IntSymbol(int inInteger) : internalInt(inInteger) {}
	
IntSymbol::~IntSymbol() {}

int IntSymbol::GetSymbol() const {	return internalInt;}
*/