#ifndef _SOARQUERY_H_
#define _SOARQUERY_H_

#include <map>
#include <list>
#include "sml_Client.h"
#include "Epmem.h"

using namespace EpmemNS;
using std::map;
using std::list;

class SoarQuery {
public:
	SoarQuery();
	QueryResult Query(const WMEList &cue);

	void UpdateNextEpisode(WMEList &addlist, DelList &dellist);
	char const *PrintState();

private:
	void AddWME(sml::Identifier* parent, WME *wme);
	void DeleteWME(WMEUID uid);

	sml::Kernel *kernel;
	sml::Agent *agent;
	sml::Identifier *istate;
	map<WME*, sml::WMElement*> wmemap;
	map<SymbolUID, list<sml::Identifier*>*> idmap; // maps identifier uids to sml id wmes
	map<WMEUID, WME*> uidwmemap;
};

#endif