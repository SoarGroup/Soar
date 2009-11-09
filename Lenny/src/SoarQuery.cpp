#include <map>
#include "sml_Client.h"

#include "SoarEpmem.h"
#include "Symbol.h"
#include "FloatSymbol.h"
#include "IntegerSymbol.h"
#include "IdentifierSymbol.h"
#include "StringSymbol.h"

using namespace sml;
using std::map;

void SoarEpmem::AddWME(Identifier* parent, WME *wme) {
	WMElement *smlwme;
	Identifier *idwme;
	map<long, list<Identifier*>*>::iterator i;
	list<Identifier*> *idwmelist;
	
	Symbol *attr = wme->GetAttr();
	Symbol *val = wme->GetVal();
	
	switch (val->GetType()) {
		case IdSym:
			i = idmap.find(val->GetUID());
			if (i != idmap.end()) {
				idwmelist = i->second;
				idwme = idwmelist->front();
				smlwme = agent->CreateSharedIdWME(parent, attr->GetString(), idwme);
			} else {
				idwmelist = new list<Identifier*>();
				idmap[val->GetUID()] = idwmelist;
				smlwme = agent->CreateIdWME(parent, attr->GetString());
			}
			idwmelist->push_back(dynamic_cast<Identifier*>(smlwme));

			break;
		case StrSym:
			smlwme = agent->CreateStringWME(parent, attr->GetString(), val->GetString());
			break;
		case FloatSym:
			smlwme = agent->CreateFloatWME(parent, attr->GetString(), static_cast<FloatSymbol*>(val)->GetValue());
			break;
		case IntSym:
			smlwme = agent->CreateIntWME(parent, attr->GetString(), static_cast<IntegerSymbol*>(val)->GetValue());
			break;
	}
	wmemap[wme] = smlwme;
	smlwmemap[smlwme] = wme;
}

void SoarEpmem::DeleteWME(WME *wme) {
	WMElement *smlwme = wmemap[wme];
	Symbol *val = wme->GetVal();
	list<Identifier*> *idlist;
	map<long, list<Identifier*>*>::iterator i;
	
	if (val->GetType() == IdSym) {
		idlist = idmap[val->GetUID()];
		idlist->remove(dynamic_cast<Identifier*>(smlwme));
		if (idlist->size() == 0) {
			idmap.erase(idmap.find(val->GetUID()));
			delete idlist;
		}
	}
	
	agent->DestroyWME(smlwme);
	wmemap.erase(wmemap.find(wme));
	smlwmemap.erase(smlwmemap.find(smlwme));
}

void SoarEpmem::UpdateNextEpisode(list<WME*> &addlist, list<WME*> &dellist) {
	list<WME*> addlistcopy(addlist);
	list<WME*>::iterator i;
	list<WME*>::iterator j;
	
	// addlist not ordered by parents first, need multiple passes
	while (addlistcopy.size() > 0) {
		i = addlistcopy.begin();
		while (i != addlistcopy.end()) {
			Symbol *id = (*i)->GetId();

			map<long, list<Identifier*>*>::iterator k;
			k = idmap.find(id->GetUID());
			if (k != idmap.end()) {
				Identifier *parent = k->second->front();
				AddWME(parent, *i);
				i = addlistcopy.erase(i);
			} else {
				++i;
			}
		}
	}
	
	for (j = dellist.begin(); j != dellist.end(); ++j) {
		DeleteWME(*j);
	}
}

SoarEpmem::SoarEpmem() 
: wmemap(), idmap()
{
	kernel = Kernel::CreateKernelInCurrentThread(Kernel::kDefaultLibraryName, true, 0);
	agent = kernel->CreateAgent("soar1");
	istate = agent->CreateIdWME(agent->GetInputLink(), "state");
	list<Identifier*> *idlist = new list<Identifier*>();
	idlist->push_back(istate);
	idmap[0] = idlist; // top state always has UID 0
}

QueryResult SoarEpmem::Query(list<WME*> cue) {

}