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

using namespace std;

typedef list<WME*> Episode;

class SimpleEpmem : public Epmem {
public:
	SimpleEpmem();
	~SimpleEpmem();
	
	int AddEpisode(const list<WME*> &addlist, const list<long> &dellist);
	int Retrieve(int episode, list<WME*> &result);
	QueryResult Query(const list<WME*> &cue);
	
	SymbolFactory *GetSymbolFactory() { return symfactory; }

	string GetString();

private:
	vector<list<WME*>*> eps;
	SimpleSymbolFactory *symfactory;
};
#endif