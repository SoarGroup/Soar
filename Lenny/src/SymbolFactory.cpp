
#include "IntegerSymbol.h"
#include "FloatSymbol.h"
#include "StringSymbol.h"
#include "IdentifierSymbol.h"

#include "SymbolFactory.h"

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
