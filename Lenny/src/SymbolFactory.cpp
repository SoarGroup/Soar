
#include "SymbolFactory.h"

Symbol* SymbolFactory::NewSymbol( long newUID, const char* newStr, bool newCnst )
{
	return new Symbol( newUID, newStr, newCnst );
}
