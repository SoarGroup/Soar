
#ifndef SQLITE_SYMBOLFACTORY_H
#define SQLITE_SYMBOLFACTORY_H

#include "SymbolFactory.h"
#include "sqlite3.h"

class SQLiteSymbolFactory: public SymbolFactory
{
	public:
		Symbol* GetIntegerSymbol( long val );
		Symbol* GetFloatSymbol( double val );
		Symbol* GetStringSymbol( const char* val );
		Symbol* GetIdentifierSymbol( char letter, long number );
		
		SQLiteSymbolFactory( sqlite3* newDB );
		
	private:
		sqlite3* db;
		
		sqlite3_stmt* findInt;
		sqlite3_stmt* findFloat;
		sqlite3_stmt* findString;
		sqlite3_stmt* findId;
		sqlite3_stmt* addSymbol;
		
		enum SymbolType { integer_t, float_t, string_t, id_t };
};

#endif
