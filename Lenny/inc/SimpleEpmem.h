/*
 * Simple Epmem implementation that stores episodes as lists of WMEs.
 * Querying is not implemented.
 */

#ifndef _SIMPLEEPMEM_H_
#define _SIMPLEEPMEM_H_

#include <list>
#include <vector>
#include "Epmem.h"
#include "SimpleSymbolFactory.h"

namespace EpmemNS {

class SimpleEpmem : public Epmem {
public:
	SimpleEpmem();
	~SimpleEpmem();
	
	EpisodeId AddEpisode(const WMEList &addlist, const DelList &dellist);
	EpisodeId Retrieve(EpisodeId episode, WMEList &result);
	QueryResult Query(const WMEList &cue);
	int GetNumEpisodes() { return eps.size(); }
	
	SymbolFactory *GetSymbolFactory() { return symfactory; }

	std::string GetString();

private:
	std::vector<WMEList*> eps;
	SimpleSymbolFactory *symfactory;
};
}

#endif