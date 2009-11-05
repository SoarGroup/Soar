
#ifndef FLOAT_SYMBOL_H
#define FLOAT_SYMBOL_H

#include "Symbol.h"

class SymbolFactory;

class FloatSymbol: public Symbol
{
	friend class SymbolFactory;

	public:
		double GetValue();

	private:
		double value;

		FloatSymbol( long newUID, double newValue );
};

#endif
