
#include <iostream>

#include "sqlite3.h"

#include "SQLiteSymbolFactory.h"
#include "Symbol.h"
#include "IntegerSymbol.h"
#include "FloatSymbol.h"
#include "StringSymbol.h"
#include "IdentifierSymbol.h"

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
	
	IntegerSymbol* fiveA = myFactory.GetIntegerSymbol( 5 );
	IntegerSymbol* six = myFactory.GetIntegerSymbol( 6 );
	IntegerSymbol* fiveB = myFactory.GetIntegerSymbol( 5 );
	
	FloatSymbol* fiveFA = myFactory.GetFloatSymbol( 5.0 );
	FloatSymbol* sixF = myFactory.GetFloatSymbol( 6.0 );
	FloatSymbol* fiveFB = myFactory.GetFloatSymbol( 5.0 );
	
	StringSymbol* fiveSA = myFactory.GetStringSymbol( "5" );
	StringSymbol* sixS = myFactory.GetStringSymbol( "6" );
	StringSymbol* fiveSB = myFactory.GetStringSymbol( "5" );
	
	IdentifierSymbol* fiveIA = myFactory.GetIdentifierSymbol( 'X', 5 );
	IdentifierSymbol* sixI = myFactory.GetIdentifierSymbol( 'X', 6 );
	IdentifierSymbol* fiveIB = myFactory.GetIdentifierSymbol( 'X', 5 );
	
	cout << fiveA->GetString() << ": " << fiveA->GetUID() << " = " << fiveA->GetValue() << endl;
	cout << fiveB->GetString() << ": " << fiveB->GetUID() << " = " << fiveB->GetValue() << endl;
	cout << six->GetString() << ": " << six->GetUID() << " = " << six->GetValue() << endl;
	
	cout << fiveFA->GetString() << ": " << fiveFA->GetUID() << " = " << fiveFA->GetValue() << endl;
	cout << fiveFB->GetString() << ": " << fiveFB->GetUID() << " = " << fiveFB->GetValue() << endl;
	cout << sixF->GetString() << ": " << sixF->GetUID() << " = " << sixF->GetValue() << endl;
	
	cout << fiveSA->GetString() << ": " << fiveSA->GetUID() << " = " << fiveSA->GetValue() << endl;
	cout << fiveSB->GetString() << ": " << fiveSB->GetUID() << " = " << fiveSB->GetValue() << endl;
	cout << sixS->GetString() << ": " << sixS->GetUID() << " = " << sixS->GetValue() << endl;
	
	cout << fiveIA->GetString() << ": " << fiveIA->GetUID() << " = " << fiveIA->GetLetter() << fiveIA->GetNumber() << endl;
	cout << fiveIB->GetString() << ": " << fiveIB->GetUID() << " = " << fiveIB->GetLetter() << fiveIB->GetNumber() << endl;
	cout << sixI->GetString() << ": " << sixI->GetUID() << " = " << sixI->GetLetter() << sixI->GetNumber() << endl;
	
	sqlite3_close( db );

	cout << "hello world" << endl;
	
	return 0;
}
