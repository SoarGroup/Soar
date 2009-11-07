
#include <iostream>
#include <list>
#include <map>
#include <cassert>

#include "SQLiteEpisodicMemory.h"

#include "SymbolFactory.h"
#include "SQLiteSymbolFactory.h"
#include "WME.h"

#include "sqlite3.h"

using namespace std;

EpisodeId SQLiteEpisodicMemory::AddEpisode( WMEList& addList, DelList& removeList )
{
	// add adds
	{
		int rc;

		sqlite3_bind_int64( addAdd, 1, currentEpisode );

		for ( WMEList::iterator a=addList.begin(); a!=addList.end(); a++ )
		{
			sqlite3_bind_int64( addAdd, 2, (*a)->GetId()->GetUID() );
			sqlite3_bind_int64( addAdd, 3, (*a)->GetAttr()->GetUID() );
			sqlite3_bind_int64( addAdd, 4, (*a)->GetVal()->GetUID() );
			sqlite3_bind_int64( addAdd, 5, (*a)->GetUID() );
			rc = sqlite3_step( addAdd );
			assert( rc == SQLITE_DONE );

			sqlite3_reset( addAdd );
		}
	}

	// add dels
	{
		int rc;

		sqlite3_bind_int64( addDel, 1, currentEpisode );

		for ( DelList::iterator d=removeList.begin(); d!=removeList.end(); d++ )
		{
			sqlite3_bind_int64( addDel, 2, (*d) );
			rc = sqlite3_step( addDel );
			assert( rc == SQLITE_DONE );

			sqlite3_reset( addDel );
		}
	}

	return ( currentEpisode++ );
}

EpisodicMemory::ResultType SQLiteEpisodicMemory::RetrieveEpisode( EpisodeId episode, WMEList &result )
{
	if ( ( episode < firstEpisode ) || ( episode >= currentEpisode  ) )
	{
		return failure;
	}

	// clear result set
	result.clear();

	{
		Symbol* id;
		Symbol* attr;
		Symbol* val;
		long uid;

		bool goodToAdd;
		bool stillDeleting = false;
		long uidDel;

		// get all adds in the order they were entered
		sqlite3_bind_int64( getEpAdds, 1, EpisodicMemory::firstEpisode );
		sqlite3_bind_int64( getEpAdds, 2, episode );

		// // get all deletes in the order of w_uid (these way we can prevent adding)
		sqlite3_bind_int64( getEpDels, 1, EpisodicMemory::firstEpisode );
		sqlite3_bind_int64( getEpDels, 2, episode );
		if ( sqlite3_step( getEpDels ) == SQLITE_ROW )
		{
			stillDeleting = true;
			uidDel = sqlite3_column_int64( getEpDels, 0 );
		}

		// while there's stuff to add
		while ( sqlite3_step( getEpAdds ) == SQLITE_ROW )
		{
			// assume a good addition
			goodToAdd = true;
			uid = sqlite3_column_int64( getEpAdds, 3 );

			// if still stuff to prevent adding
			if ( stillDeleting )
			{
				// if a delete
				if ( uid == uidDel )
				{
					// prevent add
					goodToAdd = false;

					// see if there's more deletes
					if ( sqlite3_step( getEpDels ) == SQLITE_ROW )
					{
						uidDel = sqlite3_column_int64( getEpDels, 0 );
					}
					else
					{
						stillDeleting = false;
					}
				}
			}

			// if a good add
			if ( goodToAdd )
			{
				// cross-reference id with symbol factory, confirm a valid identifier
				id = myFactory->GetSymbolByUID( sqlite3_column_int64( getEpAdds, 0 ) );
				assert( id->GetType() == Symbol::IdSym );

				// cross-reference attr/value with symbol factory
				attr = myFactory->GetSymbolByUID( sqlite3_column_int64( getEpAdds, 1 ) );
				val = myFactory->GetSymbolByUID( sqlite3_column_int64( getEpAdds, 2 ) );

				// add wme
				result.push_back( new WME( dynamic_cast<IdentifierSymbol*>( id ), attr, val, uid ) );
			}
		}

		sqlite3_reset( getEpAdds );
		sqlite3_reset( getEpDels );
	}

	return success;
}

SQLiteEpisodicMemory::SQLiteEpisodicMemory( sqlite3* newDB )
: db( newDB ), myFactory( new SQLiteSymbolFactory( newDB ) ), currentEpisode( firstEpisode )
{
	// create tables
	{
		sqlite3_stmt *stmt;
		const char *tail;
		int rc;

		list< const char* > structures;
		{
			structures.push_back( "CREATE TABLE IF NOT EXISTS ep_adds ( uid INTEGER PRIMARY KEY AUTOINCREMENT, ep_id INTEGER, w_id INTEGER, w_attr INTEGER, w_val INTEGER, w_uid INTEGER )" );
			structures.push_back( "CREATE INDEX IF NOT EXISTS ep_adds_id_uid ON ep_adds (ep_id, uid)" );

			structures.push_back( "CREATE TABLE IF NOT EXISTS ep_dels ( uid INTEGER PRIMARY KEY AUTOINCREMENT, ep_id INTEGER, w_uid INTEGER )" );
			structures.push_back( "CREATE INDEX IF NOT EXISTS ep_dels_id_wuid ON ep_dels (ep_id, w_uid)" );
		}

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
		queries[ &addAdd ] = "INSERT INTO ep_adds ( ep_id, w_id, w_attr, w_val, w_uid ) VALUES (?,?,?,?,?)";
		queries[ &addDel ] = "INSERT INTO ep_dels ( ep_id, w_uid ) VALUES (?,?)";
		queries[ &getEpAdds ] = "SELECT w_id, w_attr, w_val, w_uid FROM ep_adds WHERE ep_id >= ? AND ep_id <= ? ORDER BY uid ASC";
		queries[ &getEpDels ] = "SELECT w_uid FROM ep_dels WHERE ep_id >= ? AND ep_id <= ? ORDER BY w_uid ASC";

		for ( map< sqlite3_stmt**, const char* >::iterator p=queries.begin(); p!=queries.end(); p++ )
		{
			rc = sqlite3_prepare_v2( db, p->second, -1, p->first, &tail );
			if ( rc != SQLITE_OK )
			{
				cout << "BAD SQL (" << p->second << "): " << sqlite3_errmsg( db ) << endl;
				exit(1);
			}
		}
	}
}

SymbolFactory* SQLiteEpisodicMemory::GetSymbolFactory()
{
	return myFactory;
}
