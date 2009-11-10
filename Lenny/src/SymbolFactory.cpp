
#include "IntegerSymbol.h"
#include "FloatSymbol.h"
#include "StringSymbol.h"
#include "IdentifierSymbol.h"

#include "SymbolFactory.h"

using namespace EpmemNS;

Symbol* SymbolFactory::GetSymbol( long val )
{
	return GetIntegerSymbol( val );
}

Symbol* SymbolFactory::GetSymbol( double val )
{
	return GetFloatSymbol( val );
}

Symbol* SymbolFactory::GetSymbol( const char* val )
{
	return GetStringSymbol( val );
}

Symbol* SymbolFactory::GetSymbol( char letter, long number )
{
	return GetIdentifierSymbol( letter, number );
}

//

IntegerSymbol* SymbolFactory::NewIntegerSymbol( SymbolUID newUID, long newValue )
{
	return new IntegerSymbol( newUID, newValue );
}

FloatSymbol* SymbolFactory::NewFloatSymbol( SymbolUID newUID, double newValue )
{
	return new FloatSymbol( newUID, newValue );
}

StringSymbol* SymbolFactory::NewStringSymbol( SymbolUID newUID, const char* newValue )
{
	return new StringSymbol( newUID, newValue );
}

IdentifierSymbol* SymbolFactory::NewIdentifierSymbol( SymbolUID newUID, char newLetter, long newNumber )
{
	return new IdentifierSymbol( newUID, newLetter, newNumber );
}
