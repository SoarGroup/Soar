#include "sml_ClientWME.h"
#include "sml_ClientRelease.h"
#include "sml_Connection.h"

using namespace sml;

WME::WME(char const* pID, ClientSML* pClientSML)
{
	SetId(pID) ;
	SetClientSML(pClientSML) ;
}
	
//WME::WME(IdentifierSymbol* id, ISymbol* attrib, ISymbol* val) : identifier(id), attribute(attrib), value(val) {}

WME::~WME()
{
}

void WME::Release(gSKI::Error* err)
{
	Release::ReleaseObject(this) ;

	delete this ;
}

const ISymbol* WME::GetValue() const
{
	AnalyzeXML response ;
	ISymbol* pValue = NULL ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IWme_GetValue, GetId()))
	{
		// NOTE: The caller is responsible for deleting this.  This matches gSKI's definition.
		pValue = new Symbol(response.GetResultString(), GetClientSML()) ;
	}

	return pValue ;
}

/*
void WME::SetAttribute(ISymbol* attribute){	this->attribute = attribute;}

void WME::SetValue(ISymbol* value){	this->value = value;}

//const IdentifierSymbol* WME::GetIdentifier() const {	return identifier;}

const ISymbol* WME::GetAttribute() const {	return attribute;}

const ISymbol* WME::GetValue() const {	return value;}

unsigned long WME::GetTimeTag() const {	return timeTag;}
*/