#include <string.h>
#include "Symbol.h"

using namespace EpmemNS;

SymbolUID Symbol::GetUID()
{
	return UID;
}

const char* Symbol::GetString()
{
	return str;
}

void Symbol::InitSymbol( SymbolUID newUID, const char* newStr )
{
	UID = newUID;
	str = strdup( newStr );
}
