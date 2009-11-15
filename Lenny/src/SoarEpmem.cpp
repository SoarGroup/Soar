#include <iostream>
#include "SoarEpmem.h"

using namespace EpmemNS;
using namespace std;

SoarEpmem::SoarEpmem()
: eps()
{
	symfac = new SimpleSymbolFactory();
}

EpisodeId SoarEpmem::AddEpisode(const WMEList &addlist, const DelList &dellist) {
	Episode *newep = new Episode();
	newep->addlist = new WMEList(addlist);
	newep->dellist = new DelList(dellist);
	eps.push_back(newep);
	return eps.size() - 1;
}

EpisodeId SoarEpmem::Retrieve(EpisodeId episode, WMEList &result) {
	return -1;
}

QueryResult SoarEpmem::Query(const WMEList &cue) {
	QueryResult r;
	vector<Episode*>::iterator i;
	list<SymbolUID> matched;
	list<SymbolUID>::iterator j;
	
	SoarQuery query(cue);
	
	cout << query.PrintCueProductions() << endl;
	
	for (i = eps.begin(); i != eps.end(); ++i) {
		query.UpdateNextEpisode(*((*i)->addlist), *((*i)->dellist));
		matched.clear();
		query.GetMatchedLeafIds(matched);
		
		cout << query.PrintState() << endl;
		/*
		for (j = matched.begin(); j != matched.end(); ++j) {
			cout << *j << " ";
		}
		cout << endl;
		*/
		
		if (query.GetGraphMatch()) {
			cout << "GRAPH MATCH!!!" << endl;
		}
	}
		
	r.episode = -1;
	return r;
}

SymbolFactory *SoarEpmem::GetSymbolFactory() {
	return symfac;
}

int SoarEpmem::GetNumEpisodes() {
	return eps.size();
}
