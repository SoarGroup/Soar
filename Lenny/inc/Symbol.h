
#ifndef SYMBOL_H
#define SYMBOL_H

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

		enum SymbolType { IdSym, StrSym, FloatSym, IntSym };

		virtual SymbolType GetType() = 0;
		virtual bool IsConst() = 0;

	private:
		void InitSymbol( long newUID, const char* newStr );

		long UID;
		char* str;
};

#endif
