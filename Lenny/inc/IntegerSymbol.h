
#ifndef INTEGER_SYMBOL_H
#define INTEGER_SYMBOL_H

#include "Symbol.h"

namespace EpmemNS {

class SymbolFactory;

class IntegerSymbol: public Symbol
{
	friend class SymbolFactory;

	public:
		long GetValue();
		Symbol::SymbolType GetType();
		bool IsConst();

	private:
		long value;

		IntegerSymbol( SymbolUID newUID, long newValue );
};
}

#endif
