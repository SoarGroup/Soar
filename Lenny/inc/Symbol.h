
#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>

class SymbolFactory;

class Symbol
{
	friend class SymbolFactory;

	public:
		long GetUID();
		const char* GetString();
		bool IsConst();

	private:
		Symbol( long newUID, const char* newStr, bool newCnst );

		long UID;
		const char* str;
		std::string stdstr;
		bool cnst;
};

#endif