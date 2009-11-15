
#ifndef _SOAR_EPMEM_H_
#define _SOAR_EPMEM_H_

#include <vector>

#include "Epmem.h"
#include "SoarQuery.h"
#include "SimpleSymbolFactory.h"

typedef struct episode_struct {
	EpmemNS::WMEList *addlist;
	EpmemNS::DelList *dellist;
} Episode;

class SoarEpmem : public EpmemNS::Epmem {
public:
	SoarEpmem();

	EpmemNS::EpisodeId AddEpisode(const EpmemNS::WMEList &addlist, const EpmemNS::DelList &dellist);
	EpmemNS::EpisodeId Retrieve(EpmemNS::EpisodeId episode, EpmemNS::WMEList &result);
	EpmemNS::QueryResult Query(const EpmemNS::WMEList &cue);

	EpmemNS::SymbolFactory *GetSymbolFactory();
	
	int GetNumEpisodes();
	
private:
	std::vector<Episode*> eps;
	SimpleSymbolFactory *symfac;
};

#endif