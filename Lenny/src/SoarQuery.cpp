#include <map>
#include <set>
#include <string>
#include <sstream>
#include "sml_Client.h"

#include "SoarQuery.h"
#include "Symbol.h"
#include "FloatSymbol.h"
#include "IntegerSymbol.h"
#include "IdentifierSymbol.h"
#include "StringSymbol.h"

using namespace sml;
using std::map;
using std::set;
using std::string;
using std::stringstream;
using std::endl;

SoarQuery::SoarQuery() 
: wmemap(), idmap(), uidwmemap()
{
	kernel = Kernel::CreateKernelInCurrentThread(Kernel::kDefaultLibraryName, true, 0);
	agent = kernel->CreateAgent("soar1");
	istate = agent->CreateIdWME(agent->GetInputLink(), "state");
	list<Identifier*> *idlist = new list<Identifier*>();
	idlist->push_back(istate);
	idmap[0] = idlist; // top state always has UID 0
}

void SoarQuery::AddWME(Identifier* parent, WME *wme) {
	WMElement *smlwme;
	Identifier *idwme;
	map<long, list<Identifier*>*>::iterator i;
	list<Identifier*> *idwmelist;
	
	Symbol *attr = wme->GetAttr();
	Symbol *val = wme->GetVal();
	
	uidwmemap[wme->GetUID()] = wme;
	
	switch (val->GetType()) {
		case Symbol::IdSym:
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
		case Symbol::StrSym:
			smlwme = agent->CreateStringWME(parent, attr->GetString(), val->GetString());
			break;
		case Symbol::FloatSym:
			smlwme = agent->CreateFloatWME(parent, attr->GetString(), static_cast<FloatSymbol*>(val)->GetValue());
			break;
		case Symbol::IntSym:
			smlwme = agent->CreateIntWME(parent, attr->GetString(), static_cast<IntegerSymbol*>(val)->GetValue());
			break;
	}
	wmemap[wme] = smlwme;
}

void SoarQuery::DeleteWME(WMEUID uid) {
	WME *wme = uidwmemap[uid];
	WMElement *smlwme = wmemap[wme];
	Symbol *val = wme->GetVal();
	list<Identifier*> *idlist;
	map<long, list<Identifier*>*>::iterator i;
	
	if (val->GetType() == Symbol::IdSym) {
		idlist = idmap[val->GetUID()];
		idlist->remove(dynamic_cast<Identifier*>(smlwme));
		if (idlist->size() == 0) {
			idmap.erase(idmap.find(val->GetUID()));
			delete idlist;
		}
	}
	
	agent->DestroyWME(smlwme);
	uidwmemap.erase(uidwmemap.find(uid));
	wmemap.erase(wmemap.find(wme));
}

void SoarQuery::UpdateNextEpisode(WMEList &addlist, DelList &dellist) {
	WMEList addlistcopy(addlist);
	WMEList::iterator i;
	DelList::iterator j;
	
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
	
	agent->Commit();
	agent->RunSelf(1);
}

char const *SoarQuery::PrintState() {
	return agent->ExecuteCommandLine("print -d 100 -t i2");
}

/*
 * Make the body of the production by creating a condition to match every
 * ancestor wme of a leaf identifier.
 */
string CreateProd(SymbolUID leaf, map<SymbolUID, WMEList*> &idparmap) {
	list<SymbolUID> ids;
	list<SymbolUID>::iterator i;
	WMEList *parents;
	WMEList::iterator j;
	stringstream ss;
	
	ids.push_back(leaf);
	
	for (i = ids.begin(); i != ids.end(); ++i) {
		parents = idparmap[*i];
		for (j = parents->begin(); j != parents->end(); ++j) {
			EpmemNS::IdentifierSymbol *id = (*j)->GetId();
			EpmemNS::Symbol *attr = (*j)->GetAttr();
			EpmemNS::Symbol *val = (*j)->GetVal();
			
			if (id->GetUID() != 0) {  // it's not state id
				ss << "(<" << id->GetString() << "> ^" << attr->GetString() << " <" << val->GetString() << ">)" << endl;
				ids.push_back(id->GetUID());
			} else {
				ss << "(<is> ^" << attr->GetString() << " <" << val->GetString() << ">)" << endl;
			}
		}
	}
	
	return ss.str();
}

/*
 * Create productions that implement surface matching. Each leaf
 * identifier or constant value will be represented by a single
 * production.
 */
void CreateSurfaceMatchProds(const WMEList &cue, list<string> &result) {
	map<SymbolUID, WMEList*> idparmap;  // map from identifier UIDs to the wmes that have them as values
	WMEList constwmes; // wmes that have constants for values
	WMEList::const_iterator i;
	set<SymbolUID> leafids;
	set<SymbolUID>::iterator j;
	
	for(i = cue.begin(); i != cue.end(); ++i) {
		Symbol *val = (*i)->GetVal();
		if (val->GetType() == Symbol::IdSym) {
			if (idparmap.find(val->GetUID()) == idparmap.end()) {
				idparmap[val->GetUID()] = new WMEList();
			}
			idparmap[val->GetUID()]->push_back(*i);
			
			leafids.insert(val->GetUID());
		} else {
			constwmes.push_back(*i);
		}
	}
	
	// remove all ids that have children
	for(i = cue.begin(); i != cue.end(); ++i) {
		leafids.erase((*i)->GetId()->GetUID());
	}
	
	stringstream ss;
	int prodcounter = 0;
	
	for(i = constwmes.begin(); i != constwmes.end(); ++i) {
		EpmemNS::IdentifierSymbol *id = (*i)->GetId();
		EpmemNS::Symbol *attr = (*i)->GetAttr();
		EpmemNS::Symbol *val = (*i)->GetVal();
		
		ss.str("");
		ss << "sp {const" << prodcounter++ << endl << "(state <s> ^io.input-link.state <is>)" << endl;
		ss << CreateProd(id->GetUID(), idparmap);
		ss << "(<" << id->GetString() << "> ^" << attr->GetString() << " " << val->GetString() << ")" << endl;
		ss << "}";
		
		result.push_back(ss.str());
	}
	
	for(j = leafids.begin(); j != leafids.end(); ++j) {
		ss.str("");
		ss << "sp {id" << prodcounter++ << endl << "(state <s> ^io.input-link.state <is>)" << endl;
		ss << CreateProd(*j, idparmap);
		ss << "}";
		
		result.push_back(ss.str());
	}
}

QueryResult SoarQuery::Query(const WMEList &cue) {
	list<string> prods;
	CreateSurfaceMatchProds(cue, prods);
	
	for(list<string>::iterator i = prods.begin(); i != prods.end(); ++i) {
		std::cout << *i << endl;
	}
	
	QueryResult r;
	return r;
}