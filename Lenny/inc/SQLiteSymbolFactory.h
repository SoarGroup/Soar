
#ifndef SQLITE_SYMBOLFACTORY_H
#define SQLITE_SYMBOLFACTORY_H

#include "SymbolFactory.h"
#include "sqlite3.h"

namespace EpmemNS
{
	class SQLiteSymbolFactory: public SymbolFactory
	{
		public:
			IntegerSymbol* GetIntegerSymbol( long val );
			FloatSymbol* GetFloatSymbol( double val );
			StringSymbol* GetStringSymbol( const char* val );
			IdentifierSymbol* GetIdentifierSymbol( char letter, long number );

			Symbol* GetSymbolByUID( long UID );

			SQLiteSymbolFactory( sqlite3* newDB );

		private:
			sqlite3* db;

			sqlite3_stmt* findInt;
			sqlite3_stmt* findFloat;
			sqlite3_stmt* findString;
			sqlite3_stmt* findId;
			sqlite3_stmt* addSymbol;
			sqlite3_stmt* findSymbol;
	};
}

#endif
