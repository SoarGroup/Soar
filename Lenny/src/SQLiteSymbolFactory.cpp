#include <stdlib.h>
#include <list>
#include <map>
#include <iostream>

#include "SQLiteSymbolFactory.h"
#include "sqlite3.h"
#include "misc.h"

using namespace std;

IntegerSymbol* SQLiteSymbolFactory::GetIntegerSymbol( long val )
{
	IntegerSymbol* returnVal = NULL;
	
	sqlite3_bind_int64( findInt, 2, val );
	
	if ( sqlite3_step( findInt ) == SQLITE_ROW )
	{
		returnVal = NewIntegerSymbol( sqlite3_column_int64( findInt, 0 ), val );
	}
	else
	{
		sqlite3_bind_int64( addSymbol, 1, integer_t );
		sqlite3_bind_int64( addSymbol, 2, val );
		sqlite3_bind_null( addSymbol, 3 );
		sqlite3_bind_null( addSymbol, 4 );
		sqlite3_step( addSymbol );
		
		returnVal = NewIntegerSymbol( sqlite3_last_insert_rowid( db ), val );
		
		sqlite3_reset( addSymbol );
	}
	
	sqlite3_reset( findInt );
	
	return returnVal;
}

FloatSymbol* SQLiteSymbolFactory::GetFloatSymbol( double val )
{
	FloatSymbol* returnVal = NULL;
	
	sqlite3_bind_double( findFloat, 2, val );
	
	if ( sqlite3_step( findFloat ) == SQLITE_ROW )
	{
		returnVal = NewFloatSymbol( sqlite3_column_int64( findFloat, 0 ), val );
	}
	else
	{
		sqlite3_bind_int64( addSymbol, 1, float_t );
		sqlite3_bind_null( addSymbol, 2 );
		sqlite3_bind_double( addSymbol, 3, val );		
		sqlite3_bind_null( addSymbol, 4 );
		sqlite3_step( addSymbol );
		
		returnVal = NewFloatSymbol( sqlite3_last_insert_rowid( db ), val );
		
		sqlite3_reset( addSymbol );
	}
	
	sqlite3_reset( findFloat );
	
	return returnVal;
}

StringSymbol* SQLiteSymbolFactory::GetStringSymbol( const char* val )
{
	StringSymbol* returnVal = NULL;
	
	sqlite3_bind_text( findString, 2, val, -1, SQLITE_STATIC );
	
	if ( sqlite3_step( findString ) == SQLITE_ROW )
	{
		returnVal = NewStringSymbol( sqlite3_column_int64( findString, 0 ), val );
	}
	else
	{
		sqlite3_bind_int64( addSymbol, 1, string_t );
		sqlite3_bind_null( addSymbol, 2 );
		sqlite3_bind_null( addSymbol, 3 );
		sqlite3_bind_text( addSymbol, 4, val, -1, SQLITE_STATIC );
		sqlite3_step( addSymbol );
		
		returnVal = NewStringSymbol( sqlite3_last_insert_rowid( db ), val );
		
		sqlite3_reset( addSymbol );
	}
	
	sqlite3_reset( findString );
	
	return returnVal;
}

IdentifierSymbol* SQLiteSymbolFactory::GetIdentifierSymbol( char letter, long number )
{
	IdentifierSymbol* returnVal = NULL;
	
	char letterS[2];
	letterS[0] = letter;
	letterS[1] = '\0';
	
	sqlite3_bind_text( findId, 2, letterS, -1, SQLITE_STATIC );
	sqlite3_bind_int64( findId, 3, number );
	
	if ( sqlite3_step( findId ) == SQLITE_ROW )
	{
		returnVal = NewIdentifierSymbol( sqlite3_column_int64( findId, 0 ), letter, number );
	}
	else
	{
		sqlite3_bind_int64( addSymbol, 1, id_t );
		sqlite3_bind_int64( addSymbol, 2, number );
		sqlite3_bind_null( addSymbol, 3 );
		sqlite3_bind_text( addSymbol, 4, letterS, -1, SQLITE_STATIC );
		sqlite3_step( addSymbol );
		
		returnVal = NewIdentifierSymbol( sqlite3_last_insert_rowid( db ), letter, number );
		
		sqlite3_reset( addSymbol );
	}
	
	sqlite3_reset( findId );
	
	return returnVal;
}
		
SQLiteSymbolFactory::SQLiteSymbolFactory( sqlite3* newDB )
{
	db = newDB;	
	
	// create tables
	{
		sqlite3_stmt *stmt;
		const char *tail;
		int rc;
		
		list< const char* > structures;		
		structures.push_back( "CREATE TABLE IF NOT EXISTS symbols ( uid INTEGER PRIMARY KEY AUTOINCREMENT, sym_type INTEGER, sym_int INTEGER, sym_float REAL, sym_string TEXT )" );
		structures.push_back( "CREATE UNIQUE INDEX IF NOT EXISTS symbols_type_int ON symbols (sym_type,sym_int)" );
		structures.push_back( "CREATE UNIQUE INDEX IF NOT EXISTS symbols_type_float ON symbols (sym_type,sym_float)" );
		structures.push_back( "CREATE UNIQUE INDEX IF NOT EXISTS symbols_type_string_int ON symbols (sym_type,sym_string,sym_int)" );
		
		for ( list< const char* >::iterator p=structures.begin(); p!=structures.end(); p++ )
		{
			rc = sqlite3_prepare_v2( db, (*p), -1, &stmt, &tail );
			if ( rc != SQLITE_OK )
			{
				cout << "BAD SQL (" << (*p) << "): " << sqlite3_errmsg( db ) << endl;
				exit(1);
			}
			
			rc = sqlite3_step( stmt );
			if ( rc != SQLITE_DONE )
			{
				cout << "BAD EXEC (" << (*p) << "): " << sqlite3_errmsg( db ) << endl;
				exit(1);
			}
			sqlite3_finalize( stmt );
		}
	}
	
	// prepare reusable queries
	{
		const char *tail;
		int rc;
		
		map< sqlite3_stmt**, const char* > queries;
		queries[ &findInt ] = "SELECT uid FROM symbols WHERE sym_type=? AND sym_int=?";
		queries[ &findFloat ] = "SELECT uid FROM symbols WHERE sym_type=? AND sym_float=?";
		queries[ &findString ] = "SELECT uid FROM symbols WHERE sym_type=? AND sym_string=? AND sym_int IS NULL";
		queries[ &findId ] = "SELECT uid FROM symbols WHERE sym_type=? AND sym_string=? AND sym_int=?";
		queries[ &addSymbol ] = "INSERT INTO symbols (sym_type, sym_int, sym_float, sym_string) VALUES (?, ?, ?, ?)";
		
		for ( map< sqlite3_stmt**, const char* >::iterator p=queries.begin(); p!=queries.end(); p++ )
		{
			rc = sqlite3_prepare_v2( db, p->second, -1, p->first, &tail );
			if ( rc != SQLITE_OK )
			{
				cout << "BAD SQL (" << p->second << "): " << sqlite3_errmsg( db ) << endl;
				exit(1);
			}
		}
		
		sqlite3_bind_int64( findInt, 1, integer_t );
		sqlite3_bind_int64( findFloat, 1, float_t );
		sqlite3_bind_int64( findString, 1, string_t );
		sqlite3_bind_int64( findId, 1, id_t );
	}
}
