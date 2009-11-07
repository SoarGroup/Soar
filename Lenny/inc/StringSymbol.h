
#ifndef STRING_SYMBOL_H
#define STRING_SYMBOL_H

#include <string>

#include "Symbol.h"

class SymbolFactory;

class StringSymbol: public Symbol
{
	friend class SymbolFactory;

	public:
		Symbol::SymbolType GetType();
		bool IsConst();

	private:
		StringSymbol( long newUID, const char* newValue );
};

#endif
