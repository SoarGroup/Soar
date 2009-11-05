
#ifndef SYMBOLFACTORY_H
#define SYMBOLFACTORY_H

#include "Symbol.h"

class SymbolFactory
{
	public:
		virtual Symbol* GetIntegerSymbol( long val ) = 0;
		virtual Symbol* GetFloatSymbol( double val ) = 0;
		virtual Symbol* GetStringSymbol( const char* val ) = 0;
		virtual Symbol* GetIdentifierSymbol( char letter, long number ) = 0;

	protected:
		Symbol* NewSymbol( long newUID, const char* newStr, bool newCnst );

};

#endif
