
#include <string>

#include "WME.h"
#include "misc.h"

using namespace std;

WME::WME( IdentifierSymbol *newId, Symbol *newAttr, Symbol *newVal, long newUid )
: id( newId ), attr( newAttr ), val( newVal ), uid( newUid )
{
	string uidStr;
	toString( uid, uidStr );

	str.assign( "(" + uidStr + ": " + id->GetString() + " ^" + attr->GetString() + " " + val->GetString() + ")" );
}

IdentifierSymbol* WME::GetId()
{
	return id;
}

Symbol* WME::GetAttr()
{
	return attr;
}

Symbol* WME::GetVal()
{
	return val;
}

long WME::GetUID()
{
	return uid;
}

string WME::GetString()
{
	return str;
}
