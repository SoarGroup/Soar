
#include <iostream>

#include "SymbolFactory.h"
#include "SQLiteSymbolFactory.h"
#include "sqlite3.h"

using namespace std;

int main()
{
	sqlite3 *db;
	int rc = sqlite3_open_v2( ":memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL );
	if ( rc )
	{
		cout << "Can't open database: " << sqlite3_errmsg( db ) << endl;
		exit(1);
	}
	
	SQLiteSymbolFactory myFactory( db );
	
	Symbol* fiveA = myFactory.GetIntegerSymbol( 5 );
	Symbol* six = myFactory.GetIntegerSymbol( 6 );
	Symbol* fiveB = myFactory.GetIntegerSymbol( 5 );
	
	Symbol* fiveFA = myFactory.GetFloatSymbol( 5.0 );
	Symbol* sixF = myFactory.GetFloatSymbol( 6.0 );
	Symbol* fiveFB = myFactory.GetFloatSymbol( 5.0 );
	
	Symbol* fiveSA = myFactory.GetStringSymbol( "5" );
	Symbol* sixS = myFactory.GetStringSymbol( "6" );
	Symbol* fiveSB = myFactory.GetStringSymbol( "5" );
	
	Symbol* fiveIA = myFactory.GetIdentifierSymbol( 'X', 5 );
	Symbol* sixI = myFactory.GetIdentifierSymbol( 'X', 6 );
	Symbol* fiveIB = myFactory.GetIdentifierSymbol( 'X', 5 );
	
	cout << fiveA->GetString() << ": " << fiveA->GetUID() << endl;
	cout << fiveB->GetString() << ": " << fiveB->GetUID() << endl;
	cout << six->GetString() << ": " << six->GetUID() << endl;
	
	cout << fiveFA->GetString() << ": " << fiveFA->GetUID() << endl;
	cout << fiveFB->GetString() << ": " << fiveFB->GetUID() << endl;
	cout << sixF->GetString() << ": " << sixF->GetUID() << endl;
	
	cout << fiveSA->GetString() << ": " << fiveSA->GetUID() << endl;
	cout << fiveSB->GetString() << ": " << fiveSB->GetUID() << endl;
	cout << sixS->GetString() << ": " << sixS->GetUID() << endl;
	
	cout << fiveIA->GetString() << ": " << fiveIA->GetUID() << endl;
	cout << fiveIB->GetString() << ": " << fiveIB->GetUID() << endl;
	cout << sixI->GetString() << ": " << sixI->GetUID() << endl;
	
	sqlite3_close( db );
	
	cout << "hello world" << endl;
	
	return 0;
}
