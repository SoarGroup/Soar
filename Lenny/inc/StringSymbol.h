
#ifndef STRING_SYMBOL_H
#define STRING_SYMBOL_H

#include <string>

#include "Symbol.h"

class SymbolFactory;

class StringSymbol: public Symbol
{
	friend class SymbolFactory;

	public:
		const char* GetValue();

	private:
		const char* value;
		std::string valueS;

		StringSymbol( long newUID, const char* newValue );
};

#endif
