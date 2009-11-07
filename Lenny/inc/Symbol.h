
#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>

using std::string;

class IntegerSymbol;
class FloatSymbol;
class StringSymbol;
class IdentifierSymbol;

enum SymbolType { IdSym, StrSym, FloatSym, IntSym };

class Symbol
{
	friend class IntegerSymbol;
	friend class FloatSymbol;
	friend class StringSymbol;
	friend class IdentifierSymbol;

	public:
		long GetUID();
		bool IsConst();
		const char *GetString();
		SymbolType GetType();

	private:
		void InitSymbol( long newUID, SymbolType newType, const char *newStr, bool newCnst );

		long UID;
		char *str;
		bool cnst;
		SymbolType type;
};

#endif
