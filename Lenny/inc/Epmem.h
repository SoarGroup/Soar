#ifndef _EPMEM_H_
#define _EPMEM_H_

#include <list>
#include "WME.h"
#include "SymbolFactory.h"

using namespace std;

typedef struct query_result_struct {
	int episode;
	float matchScore;
	bool graphMatch;
} QueryResult;

class Epmem {
public:
	virtual int AddEpisode(const list<WME*> &addlist, const list<long> &dellist) = 0;
	virtual int Retrieve(int episode, list<WME*> &result) = 0;
	virtual QueryResult Query(const list<WME*> &cue) = 0;

	virtual SymbolFactory *GetSymbolFactory() = 0;
};

#endif