
#ifndef STRING_SYMBOL_H
#define STRING_SYMBOL_H

#include <string>

#include "Symbol.h"

namespace EpmemNS {

class SymbolFactory;

class StringSymbol: public Symbol
{
	friend class SymbolFactory;

	public:
		Symbol::SymbolType GetType();
		bool IsConst();

	private:
		StringSymbol( SymbolUID newUID, const char* newValue );
};
}

#endif
