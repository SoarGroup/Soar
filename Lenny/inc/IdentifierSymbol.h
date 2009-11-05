
#ifndef IDENTIFIER_SYMBOL_H
#define IDENTIFIER_SYMBOL_H

#include "Symbol.h"

class SymbolFactory;

class IdentifierSymbol: public Symbol
{
	friend class SymbolFactory;

	public:
		long GetNumber();
		char GetLetter();

	private:
		long number;
		char letter;

		IdentifierSymbol( long newUID, char newLetter, long newNumber );
};

#endif
