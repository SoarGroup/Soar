#ifndef _SOAREPMEM_H_
#define _SOAREPMEM_H_

#include <map>
#include "sml_Client.h"
#include "Epmem.h"

using namespace sml;
using std::map;

class SoarEpmem {
public:
	SoarEpmem();
	QueryResult Query(list<WME*> cue);

private:
	void UpdateNextEpisode(list<WME*> &addlist, list<int> &dellist);
	void AddWME(Identifier* parent, Symbol *attr, Symbol *val, int uid);

	Kernel *kernel;
	Agent *agent;
	Identifier *istate;
	map<int, WMElement*> wmemap; // maps wme uids from database to sml wmes
	map<int, Identifier*> idmap; // maps identifier uids to sml id wmes
};

#endif