
#ifndef FLOAT_SYMBOL_H
#define FLOAT_SYMBOL_H

#include "Symbol.h"

namespace EpmemNS {

class SymbolFactory;

class FloatSymbol: public Symbol
{
	friend class SymbolFactory;

	public:
		double GetValue();
		Symbol::SymbolType GetType();
		bool IsConst();

	private:
		double value;

		FloatSymbol( SymbolUID newUID, double newValue );
};
}

#endif
