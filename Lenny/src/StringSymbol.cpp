
#include <string>

#include "StringSymbol.h"

using namespace std;

StringSymbol::StringSymbol( long newUID, const char* newValue )
{
	InitSymbol( newUID, StrSym, newValue, true );
}