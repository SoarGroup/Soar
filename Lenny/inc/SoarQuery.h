#ifndef _SOARQUERY_H_
#define _SOARQUERY_H_

#include <map>
#include <list>
#include "sml_Client.h"
#include "Epmem.h"

class SoarQuery {
public:
	SoarQuery(const EpmemNS::WMEList &cue);
	~SoarQuery();

	void UpdateNextEpisode(EpmemNS::WMEList &addlist, EpmemNS::DelList &dellist);
	char const *PrintState();
	char const *PrintCueProductions();

	void GetMatchedLeafIds(std::list<EpmemNS::SymbolUID> &result);
	int GetMaxMatchScore();
	bool GetGraphMatch();
	
private:
	int SetCue(const EpmemNS::WMEList &cue);
	void AddWME(sml::Identifier* parent, EpmemNS::WME *wme);
	void DeleteWME(EpmemNS::WMEUID uid);

	sml::Kernel *kernel;
	sml::Agent *agent;
	sml::Identifier *istate;
	std::map<EpmemNS::WME*, sml::WMElement*> wmemap;
	std::map<EpmemNS::SymbolUID, std::list<sml::Identifier*>*> idmap;
	std::map<EpmemNS::WMEUID, EpmemNS::WME*> uidwmemap;
	
	std::list<EpmemNS::SymbolUID> matchedids;
	bool graphmatch;
	
	int maxscore;
	
	friend void SoarUpdateHandler(sml::smlUpdateEventId, void*, sml::Kernel*, sml::smlRunFlags);
};

#endif