#ifndef _SOAREPMEM_H_
#define _SOAREPMEM_H_

#include <map>
#include <list>
#include "sml_Client.h"
#include "Epmem.h"

using namespace sml;
using std::map;
using std::list;

class SoarEpmem {
public:
	SoarEpmem();
	QueryResult Query(list<WME*> cue);

private:
	void UpdateNextEpisode(list<WME*> &addlist, list<WME*> &dellist);
	void AddWME(Identifier* parent, WME *wme);
	void DeleteWME(WME *wme);

	Kernel *kernel;
	Agent *agent;
	Identifier *istate;
	map<WME*, WMElement*> wmemap;
	map<WMElement*, WME*> smlwmemap;
	map<long, list<Identifier*>*> idmap; // maps identifier uids to sml id wmes
};

#endif