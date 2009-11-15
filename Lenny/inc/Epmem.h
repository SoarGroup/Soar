#ifndef _EPMEM_H_
#define _EPMEM_H_

#include <list>
#include "WME.h"
#include "SymbolFactory.h"

namespace EpmemNS {

typedef std::list< WME* > WMEList;
typedef std::list< long > DelList;
typedef long EpisodeId;

typedef struct query_result_struct {
	EpisodeId episode;
	float matchScore;
	bool graphMatch;
} QueryResult;

class Epmem {
public:
	virtual EpisodeId AddEpisode(const WMEList &addlist, const DelList &dellist) = 0;
	virtual EpisodeId Retrieve(EpisodeId episode, WMEList &result) = 0;
	virtual QueryResult Query(const WMEList &cue) = 0;

	virtual SymbolFactory *GetSymbolFactory() = 0;
	
	virtual int GetNumEpisodes() = 0;
};

}

#endif