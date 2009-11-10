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

using std::string;
using std::vector;

class SimpleEpmem : public Epmem {
public:
	SimpleEpmem();
	~SimpleEpmem();
	
	EpisodeId AddEpisode(const WMEList &addlist, const DelList &dellist);
	EpisodeId Retrieve(EpisodeId episode, WMEList &result);
	QueryResult Query(const WMEList &cue);
	
	SymbolFactory *GetSymbolFactory() { return symfactory; }

	string GetString();

private:
	vector<WMEList*> eps;
	SimpleSymbolFactory *symfactory;
};
}

#endif