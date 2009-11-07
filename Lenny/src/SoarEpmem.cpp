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

void SoarEpmem::AddWME(Identifier* parent, Symbol *attr, Symbol *val, int uid) {
	WMElement *newwme;
	map<int, Identifier*>::iterator i;

	switch (val->GetType()) {
		case IdSym:
			i = idmap.find(val->GetUID());
			if (i != idmap.end()) {
				newwme = agent->CreateSharedIdWME(parent, attr->GetString(), i->second);
			} else {
				newwme = agent->CreateIdWME(parent, attr->GetString());
				idmap[val->GetUID()] = dynamic_cast<Identifier*>(newwme);
			}
			break;
		case StrSym:
			newwme = agent->CreateStringWME(parent, attr->GetString(), val->GetString());
			break;
		case FloatSym:
			newwme = agent->CreateFloatWME(parent, attr->GetString(), static_cast<FloatSymbol*>(val)->GetValue());
			break;
		case IntSym:
			newwme = agent->CreateIntWME(parent, attr->GetString(), static_cast<IntegerSymbol*>(val)->GetValue());
			break;
	}
	wmemap[uid] = newwme;
}

void SoarEpmem::UpdateNextEpisode(list<WME*> &addlist, list<int> &dellist) {
	list<WME*> addlistcopy(addlist);
	list<WME*>::iterator i;
	list<int>::iterator j;
	
	// addlist not ordered by parents first, need multiple passes
	while (addlistcopy.size() > 0) {
		i = addlistcopy.begin();
		while (i != addlistcopy.end()) {
			Symbol *id = (*i)->GetId();
			Symbol *attr = (*i)->GetAttr();
			Symbol *val = (*i)->GetVal();
			int uid = (*i)->GetUID();

			map<int, Identifier*>::iterator k;
			k = idmap.find(id->GetUID());
			if (k != idmap.end()) {
				Identifier *parent = k->second;
				AddWME(k->second, attr, val, uid);
				i = addlistcopy.erase(i);
			} else {
				++i;
			}
		}
	}
	
	for (j = dellist.begin(); j != dellist.end(); ++j) {
		agent->DestroyWME(wmemap[*j]);
	}
}

SoarEpmem::SoarEpmem() 
: wmemap(), idmap()
{
	kernel = Kernel::CreateKernelInCurrentThread(Kernel::kDefaultLibraryName, true, 0);
	agent = kernel->CreateAgent("soar1");
	istate = agent->CreateIdWME(agent->GetInputLink(), "state");
	idmap[0] = istate; // top state always has UID 0
}

QueryResult SoarEpmem::Query(list<WME*> cue) {

}