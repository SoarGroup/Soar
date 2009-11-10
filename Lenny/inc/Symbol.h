
#ifndef SYMBOL_H
#define SYMBOL_H

namespace EpmemNS {

class IntegerSymbol;
class FloatSymbol;
class StringSymbol;
class IdentifierSymbol;

typedef long SymbolUID;

class Symbol
{
	friend class IntegerSymbol;
	friend class FloatSymbol;
	friend class StringSymbol;
	friend class IdentifierSymbol;

	public:
		SymbolUID GetUID();
		const char* GetString();

		enum SymbolType { IdSym, StrSym, FloatSym, IntSym };

		virtual SymbolType GetType() = 0;
		virtual bool IsConst() = 0;

	private:
		void InitSymbol( SymbolUID newUID, const char* newStr );

		SymbolUID UID;
		char* str;
};
}

#endif
