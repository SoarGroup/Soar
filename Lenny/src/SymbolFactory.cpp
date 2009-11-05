
#include "IntegerSymbol.h"
#include "FloatSymbol.h"
#include "StringSymbol.h"
#include "IdentifierSymbol.h"

#include "SymbolFactory.h"

IntegerSymbol* SymbolFactory::NewIntegerSymbol( long newUID, long newValue )
{
	return new IntegerSymbol( newUID, newValue );
}

FloatSymbol* SymbolFactory::NewFloatSymbol( long newUID, double newValue )
{
	return new FloatSymbol( newUID, newValue );
}

StringSymbol* SymbolFactory::NewStringSymbol( long newUID, const char* newValue )
{
	return new StringSymbol( newUID, newValue );
}

IdentifierSymbol* SymbolFactory::NewIdentifierSymbol( long newUID, char newLetter, long newNumber )
{
	return new IdentifierSymbol( newUID, newLetter, newNumber );
}
