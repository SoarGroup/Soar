
#ifndef INTEGER_SYMBOL_H
#define INTEGER_SYMBOL_H

#include "Symbol.h"

class SymbolFactory;

class IntegerSymbol: public Symbol
{
	friend class SymbolFactory;

	public:
		long GetValue();

	private:
		long value;

		IntegerSymbol( long newUID, long newValue );
};

#endif
