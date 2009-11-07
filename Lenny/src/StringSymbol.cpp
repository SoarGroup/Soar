
#include <string>

#include "StringSymbol.h"

using namespace std;

StringSymbol::StringSymbol( long newUID, const char* newValue )
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
