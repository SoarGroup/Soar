#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <cassert>
#include <string>

#include "sqlite3.h"

#include "Symbol.h"
#include "IntegerSymbol.h"
#include "FloatSymbol.h"
#include "StringSymbol.h"
#include "IdentifierSymbol.h"

#include "SymbolFactory.h"
#include "SQLiteSymbolFactory.h"

#include "WME.h"

#include "EpisodicMemory.h"
#include "SQLiteEpisodicMemory.h"

#include "misc.h"

using namespace std;
using namespace EpmemNS;

void SymFactoryTest( sqlite3* db )
{
	SQLiteSymbolFactory myFactory( db );

	IntegerSymbol* fiveA = myFactory.GetIntegerSymbol( 5 );
	IntegerSymbol* six = myFactory.GetIntegerSymbol( 6 );
	IntegerSymbol* fiveB = myFactory.GetIntegerSymbol( 5 );

	{
		assert( fiveA->GetUID() == fiveB->GetUID() );
		assert( fiveA->GetUID() != six->GetUID() );

		assert( !strcmp( fiveA->GetString(), "5" ) );
		assert( !strcmp( six->GetString(), "6" ) );
	}
	
	FloatSymbol* fiveFA = myFactory.GetFloatSymbol( 5.0 );
	FloatSymbol* sixF = myFactory.GetFloatSymbol( 6.0 );
	FloatSymbol* fiveFB = myFactory.GetFloatSymbol( 5.0 );
	
	{
		assert( fiveFA->GetUID() == fiveFB->GetUID() );
		assert( fiveFA->GetUID() != sixF->GetUID() );

		assert( fiveA->GetUID() != fiveFA->GetUID() );
		assert( fiveA->GetUID() != sixF->GetUID() );
		assert( six->GetUID() != sixF->GetUID() );

		assert( !strcmp( fiveFA->GetString(), "5" ) );
		assert( !strcmp( sixF->GetString(), "6" ) );
	}

	StringSymbol* fiveSA = myFactory.GetStringSymbol( "5" );
	StringSymbol* sixS = myFactory.GetStringSymbol( "6" );
	StringSymbol* fiveSB = myFactory.GetStringSymbol( "5" );
	
	{
		assert( fiveSA->GetUID() == fiveSB->GetUID() );
		assert( fiveSA->GetUID() != sixS->GetUID() );

		assert( fiveA->GetUID() != fiveSA->GetUID() );
		assert( fiveA->GetUID() != sixS->GetUID() );
		assert( six->GetUID() != sixS->GetUID() );

		assert( fiveFA->GetUID() != fiveSA->GetUID() );
		assert( fiveFA->GetUID() != sixS->GetUID() );
		assert( sixF->GetUID() != sixS->GetUID() );

		assert( !strcmp( fiveSA->GetString(), "5" ) );
		assert( !strcmp( sixS->GetString(), "6" ) );
	}

	IdentifierSymbol* fiveIA = myFactory.GetIdentifierSymbol( 'X', 5 );
	IdentifierSymbol* sixI = myFactory.GetIdentifierSymbol( 'X', 6 );
	IdentifierSymbol* fiveIB = myFactory.GetIdentifierSymbol( 'X', 5 );
	
	{
		assert( fiveIA->GetUID() == fiveIB->GetUID() );
		assert( fiveIA->GetUID() != sixI->GetUID() );

		assert( fiveA->GetUID() != fiveIA->GetUID() );
		assert( fiveA->GetUID() != sixI->GetUID() );
		assert( six->GetUID() != sixI->GetUID() );

		assert( fiveFA->GetUID() != fiveIA->GetUID() );
		assert( fiveFA->GetUID() != sixI->GetUID() );
		assert( sixF->GetUID() != sixI->GetUID() );

		assert( fiveSA->GetUID() != fiveIA->GetUID() );
		assert( fiveSA->GetUID() != sixI->GetUID() );
		assert( sixS->GetUID() != sixI->GetUID() );

		assert( !strcmp( fiveIA->GetString(), "X5" ) );
		assert( !strcmp( sixI->GetString(), "X6" ) );
	}

	Symbol* genericFiveInt = myFactory.GetSymbol( static_cast<long>( 5 ) );
	Symbol* genericFiveFloat = myFactory.GetSymbol( static_cast<double>( 5.0 ) );
	Symbol* genericFiveString = myFactory.GetSymbol( static_cast<const char*>( "5" ) );
	Symbol* genericFiveId = myFactory.GetSymbol( 'X', 5 );

	{
		assert( genericFiveInt->GetUID() == fiveA->GetUID() );
		assert( genericFiveFloat->GetUID() == fiveFA->GetUID() );
		assert( genericFiveString->GetUID() == fiveSA->GetUID() );
		assert( genericFiveId->GetUID() == fiveIA->GetUID() );
	}
}

void WMETest( sqlite3* db )
{
	const char* idLetter = "S";
	long idNum = 1;
	const char* attr = "foo";
	const char* val = "bar";

	SQLiteSymbolFactory myFactory( db );

	WME myWME( myFactory.GetIdentifierSymbol( idLetter[0], idNum ), myFactory.GetStringSymbol( attr ), myFactory.GetStringSymbol( val ), idNum );

	{
		{
			string idNumStr;
			toString( idNum, idNumStr );

			string expected( "(" + idNumStr + ": " + idLetter + idNumStr + " ^" + attr + " " + val + ")" );

			assert( !myWME.GetString().compare( expected ) );
		}

		assert( myWME.GetId()->GetUID() == myFactory.GetSymbol( idLetter[0], idNum )->GetUID() );
		assert( myWME.GetAttr()->GetUID() == myFactory.GetSymbol( static_cast<const char*>( attr ) )->GetUID() );
		assert( myWME.GetVal()->GetUID() == myFactory.GetSymbol( static_cast<const char*>( val ) )->GetUID() );
		assert( myWME.GetUID() == idNum );
	}
}

void _EpMemTest_Combine( WMEList& start, WMEList& adds, DelList& dels )
{
	// start + adds
	for ( WMEList::iterator p=adds.begin(); p!=adds.end(); p++ )
	{
		start.push_back( (*p) );
	}

	// start + adds - dels
	for ( DelList::iterator p1=dels.begin(); p1!=dels.end(); p1++ )
	{
		WMEList::iterator p3 = start.end();

		for ( WMEList::iterator p2=start.begin(); p2!=start.end(); p2++ )
		{
			if ( (*p2)->GetUID() == (*p1) )
			{
				p3 = p2;
			}
		}

		if ( p3 != start.end() )
		{
			start.erase( p3 );
		}
	}
}

void _EpMemTest_Compare( WMEList& known, WMEList& test )
{
	WMEList::iterator s, r;

	for ( r=test.begin(), s=known.begin(); r!=test.end(); r++, s++ )
	{
		assert( (*r)->GetUID() == (*s)->GetUID() );
	}
	assert( s == known.end() );
}

void EpMemTest( sqlite3* db )
{
	SQLiteEpisodicMemory epmem( db );

	{
		SymbolFactory* myFactory = epmem.GetSymbolFactory();

		WMEList ep1add;
		DelList ep1del;
		WMEList ep1;

		WMEList ep2add;
		DelList ep2del;
		WMEList ep2;

		WMEList ep3add;
		DelList ep3del;
		WMEList ep3;

		IdentifierSymbol* sS1 = myFactory->GetIdentifierSymbol( 'S', 1 );
		StringSymbol* sBlock = myFactory->GetStringSymbol( "block" );
		StringSymbol* sId = myFactory->GetStringSymbol( "id" );
		StringSymbol* sOnTop = myFactory->GetStringSymbol( "on-top" );
		StringSymbol* sOnTable = myFactory->GetStringSymbol( "on-table" );
		StringSymbol* sTop = myFactory->GetStringSymbol( "top" );
		StringSymbol* sBottom = myFactory->GetStringSymbol( "bottom" );
		StringSymbol* sA = myFactory->GetStringSymbol( "A" );
		StringSymbol* sB = myFactory->GetStringSymbol( "B" );
		StringSymbol* sC = myFactory->GetStringSymbol( "C" );

		IdentifierSymbol* sB1 = myFactory->GetIdentifierSymbol( 'B', 1 );
		IdentifierSymbol* sB2 = myFactory->GetIdentifierSymbol( 'B', 2 );
		IdentifierSymbol* sB3 = myFactory->GetIdentifierSymbol( 'B', 3 );

		IdentifierSymbol* sO1 = myFactory->GetIdentifierSymbol( 'O', 1 );
		IdentifierSymbol* sO2 = myFactory->GetIdentifierSymbol( 'O', 2 );
		IdentifierSymbol* sO3 = myFactory->GetIdentifierSymbol( 'O', 3 );

		//

		WME wBlock1( sS1, sBlock, sB1, 1 );
		WME wBlock2( sS1, sBlock, sB2, 2 );
		WME wBlock3( sS1, sBlock, sB3, 3 );

		WME wOnTop1( sS1, sOnTop, sO1, 4 );
		WME wOnTop2( sS1, sOnTop, sO2, 5 );
		WME wOnTableB1( sS1, sOnTable, sB1, 6 );

		WME wBlock1Id( sB1, sId, sA, 7 );
		WME wBlock2Id( sB2, sId, sB, 8 );
		WME wBlock3Id( sB3, sId, sC, 9 );

		WME wOnTop1Top( sO1, sTop, sB3, 10 );
		WME wOnTop1Bottom( sO1, sBottom, sB2, 11 );

		WME wOnTop2Top( sO2, sTop, sB2, 12 );
		WME wOnTop2Bottom( sO2, sBottom, sB1, 13 );

		//

		WME wOnTableB3( sS1, sOnTable, sB3, 14 );

		//

		WME wOnTop3( sS1, sOnTop, sO3, 15 );
		WME wOnTop3Top( sO3, sTop, sB2, 16 );
		WME wOnTop3Bottom( sO3, sBottom, sB3, 17 );

		//

		ep1add.push_back( &wBlock1 );
		ep1add.push_back( &wBlock2 );
		ep1add.push_back( &wBlock3 );
		ep1add.push_back( &wOnTop1 );
		ep1add.push_back( &wOnTop2 );
		ep1add.push_back( &wOnTableB1 );
		ep1add.push_back( &wBlock1Id );
		ep1add.push_back( &wBlock2Id );
		ep1add.push_back( &wBlock3Id );
		ep1add.push_back( &wOnTop1Top );
		ep1add.push_back( &wOnTop1Bottom );
		ep1add.push_back( &wOnTop2Top );
		ep1add.push_back( &wOnTop2Bottom );

		ep1del.clear();

		//

		ep2add.push_back( &wOnTableB3 );

		ep2del.push_back( wOnTop1.GetUID() );
		ep2del.push_back( wOnTop1Top.GetUID() );
		ep2del.push_back( wOnTop1Bottom.GetUID() );

		//

		ep3add.push_back( &wOnTop3 );
		ep3add.push_back( &wOnTop3Top );
		ep3add.push_back( &wOnTop3Bottom );

		ep3del.push_back( wOnTop2.GetUID() );
		ep3del.push_back( wOnTop2Top.GetUID() );
		ep3del.push_back( wOnTop2Bottom.GetUID() );

		//

		EpisodeId id1 = epmem.AddEpisode( ep1add, ep1del );
		EpisodeId id2 = epmem.AddEpisode( ep2add, ep2del );
		EpisodeId id3 = epmem.AddEpisode( ep3add, ep3del );

		assert( id1 == EpisodicMemory::firstEpisode );
		assert( id2 == ( EpisodicMemory::firstEpisode + 1 ) );
		assert( id3 == ( EpisodicMemory::firstEpisode + 2 ) );

		// confirm the contents of episode 1
		{
			WMEList ep1retrieved;
			EpisodicMemory::ResultType result = epmem.RetrieveEpisode( id1, ep1retrieved );
			assert( result == EpisodicMemory::success );

			ep1.clear();
			_EpMemTest_Combine( ep1, ep1add, ep1del );
			_EpMemTest_Compare( ep1, ep1retrieved );

			cout << "Episode 1:" << endl;
			for ( WMEList::iterator p=ep1.begin(); p!=ep1.end(); p++ )
			{
				cout << (*p)->GetString() << endl;
			}
			cout << endl << endl;
		}

		// confirm the contents of episode 2
		{
			WMEList ep2retrieved;
			EpisodicMemory::ResultType result = epmem.RetrieveEpisode( id2, ep2retrieved );
			assert( result == EpisodicMemory::success );

			ep2 = ep1;
			_EpMemTest_Combine( ep2, ep2add, ep2del );
			_EpMemTest_Compare( ep2, ep2retrieved );

			cout << "Episode 2:" << endl;
			for ( WMEList::iterator p=ep2.begin(); p!=ep2.end(); p++ )
			{
				cout << (*p)->GetString() << endl;
			}
			cout << endl << endl;
		}

		// confirm the contents of episode 3
		{
			WMEList ep3retrieved;
			EpisodicMemory::ResultType result = epmem.RetrieveEpisode( id3, ep3retrieved );
			assert( result == EpisodicMemory::success );

			ep3 = ep2;
			_EpMemTest_Combine( ep3, ep3add, ep3del );
			_EpMemTest_Compare( ep3, ep3retrieved );

			cout << "Episode 3:" << endl;
			for ( WMEList::iterator p=ep3.begin(); p!=ep3.end(); p++ )
			{
				cout << (*p)->GetString() << endl;
			}
			cout << endl << endl;
		}
	}
}

int main()
{
	sqlite3 *db;
	{
		int rc = sqlite3_open_v2( ":memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL );
		if ( rc )
		{
			cout << "Can't open database: " << sqlite3_errmsg( db ) << endl;
			exit(1);
		}
	}

	//

	SymFactoryTest( db );
	cout << "Passed SymbolFactory test!" << endl;
	
	WMETest( db );
	cout << "Passed WME test!" << endl;

	EpMemTest( db );
	cout << "Passed EpMem test!" << endl;

	//

	sqlite3_close( db );

	//

	return 0;
}
