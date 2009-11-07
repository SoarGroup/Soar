
#ifndef SQLITE_EPISODIC_MEMORY_H
#define SQLITE_EPISODIC_MEMORY_H

#include "EpisodicMemory.h"
#include "SymbolFactory.h"
#include "WME.h"

#include "sqlite3.h"

class SQLiteEpisodicMemory: public EpisodicMemory
{
	public:
		EpisodeId AddEpisode( WMEList& addList, DelList& removeList );
		ResultType RetrieveEpisode( EpisodeId episode, WMEList &result );

		SymbolFactory* GetSymbolFactory();

		SQLiteEpisodicMemory( sqlite3* newDB );

	private:
		sqlite3* db;
		sqlite3_stmt* addAdd;
		sqlite3_stmt* addDel;
		sqlite3_stmt* getEpAdds;
		sqlite3_stmt* getEpDels;

		SymbolFactory* myFactory;
		EpisodeId currentEpisode;
};

#endif
