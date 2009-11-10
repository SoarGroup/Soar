
#ifndef IDENTIFIER_SYMBOL_H
#define IDENTIFIER_SYMBOL_H

#include "Symbol.h"

namespace EpmemNS {

class SymbolFactory;

class IdentifierSymbol: public Symbol
{
	friend class SymbolFactory;

	public:
		long GetNumber();
		char GetLetter();

		Symbol::SymbolType GetType();
		bool IsConst();

	private:
		long number;
		char letter;

		IdentifierSymbol( SymbolUID newUID, char newLetter, long newNumber );
};
}

#endif
