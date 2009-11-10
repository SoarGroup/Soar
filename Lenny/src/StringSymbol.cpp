
#include <string>

#include "StringSymbol.h"

using namespace EpmemNS;

StringSymbol::StringSymbol( SymbolUID newUID, const char* newValue )
{
	InitSymbol( newUID, newValue );
}

Symbol::SymbolType StringSymbol::GetType()
{
	return StrSym;
}

bool StringSymbol::IsConst()
{
	return true;
}
