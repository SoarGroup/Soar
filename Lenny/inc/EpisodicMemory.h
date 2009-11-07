
#ifndef EPISODIC_MEMORY_H
#define EPISODIC_MEMORY_H

#include <list>

#include "SymbolFactory.h"
#include "WME.h"

typedef std::list< WME* > WMEList;
typedef std::list< long > DelList;
typedef long EpisodeId;

class EpisodicMemory
{
	public:
		enum ResultType { success, failure };
		static const EpisodeId firstEpisode = 1;

		virtual EpisodeId AddEpisode( WMEList& addList, DelList& removeList ) = 0;
		virtual ResultType RetrieveEpisode( EpisodeId episode, WMEList &result ) = 0;

		virtual SymbolFactory* GetSymbolFactory() = 0;

	private:

};

#endif
