
#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>

class IntegerSymbol;
class FloatSymbol;
class StringSymbol;
class IdentifierSymbol;

class Symbol
{
	friend class IntegerSymbol;
	friend class FloatSymbol;
	friend class StringSymbol;
	friend class IdentifierSymbol;

	public:
		long GetUID();
		const char* GetString();
		bool IsConst();

	private:
		void InitSymbol( long newUID, std::string& newStr, bool newCnst );

		long UID;
		const char* str;
		std::string stdstr;
		bool cnst;
};

#endif
