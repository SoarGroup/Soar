#include <map>
#include <set>
#include <string>
#include <string.h>
#include <sstream>
#include "sml_Client.h"

#include "SoarQuery.h"
#include "Symbol.h"
#include "FloatSymbol.h"
#include "IntegerSymbol.h"
#include "IdentifierSymbol.h"
#include "StringSymbol.h"

using namespace sml;
using namespace std;
using namespace EpmemNS;

void SoarUpdateHandler(smlUpdateEventId id, void *usrdata, Kernel *kernel, smlRunFlags runFlags) {
	Identifier::ChildrenIter i;
	SoarQuery *q = static_cast<SoarQuery*>(usrdata);
	Identifier *out = q->agent->GetOutputLink();
	
	q->matchedids.clear();
	q->graphmatch = false;
	for (i = out->GetChildrenBegin(); i != out->GetChildrenEnd(); ++i) {
		if (strcmp((*i)->GetAttribute(), "graphmatch") == 0) {
			q->graphmatch = true;
		} else {
			q->matchedids.push_back((*i)->ConvertToIntElement()->GetValue());
		}
	}
}

SoarQuery::SoarQuery(const WMEList &cue) 
: wmemap(), idmap(), uidwmemap(), matchedids(), graphmatch(false)
{
	kernel = Kernel::CreateKernelInCurrentThread(Kernel::kDefaultLibraryName, true, 0);
	agent = kernel->CreateAgent("soar1");
	kernel->RegisterForUpdateEvent(smlEVENT_AFTER_ALL_OUTPUT_PHASES, SoarUpdateHandler, this);
	agent->ExecuteCommandLine("set-stop-phase --after --output");
	agent->ExecuteCommandLine("wait -e");
	
	istate = agent->CreateIdWME(agent->GetInputLink(), "state");
	list<Identifier*> *idlist = new list<Identifier*>();
	idlist->push_back(istate);
	idmap[0] = idlist; // top state always has UID 0
	
	maxscore = SetCue(cue);
}

SoarQuery::~SoarQuery() {
	map<SymbolUID, list<Identifier*>*>::iterator i;
	
	for (i = idmap.begin(); i != idmap.end(); ++i) {
		delete i->second;
	}
	
	kernel->Shutdown();
	delete kernel;
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

string WMEtoCond(WME *w) {
	stringstream ss;
	Symbol *id = w->GetId();
	Symbol *attr = w->GetAttr();
	Symbol *val = w->GetVal();
	
	ss << "(<" << id->GetString() << ">";
	if (attr->GetType() == Symbol::IdSym) {
		ss << " ^<" << attr->GetString() << ">";
	}
	else {
		ss << " ^" << attr->GetString();
	}
	
	if (val->GetType() == Symbol::IdSym) {
		ss << " <" << val->GetString() << ">";
	}
	else {
		ss << " " << val->GetString();
	}
	
	ss << ")";
	
	return ss.str();
}

string CreateGraphMatchProd(const WMEList &cue) {
	stringstream ss;
	WMEList::const_iterator i;
	
	ss << "sp {graphmatch (state <s> ^io <io>)";
	ss << "(<io> ^input-link.state <is> ^output-link <out>)";
	
	for(i = cue.begin(); i != cue.end(); ++i) {
		ss << WMEtoCond(*i);
	}
	ss << "--> (<out> ^graphmatch t)}";
	
	return ss.str();
}

/*
 * Make the condition body of the production by creating a condition to
 * match every ancestor wme of a leaf identifier.
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
			Symbol *attr = (*j)->GetAttr();
			Symbol *val = (*j)->GetVal();
			
			if (id->GetUID() != 0) {  // it's not state id
				ss << WMEtoCond(*j) << endl;
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
 * production. The production will test for the existence of all
 * ancestors of the leaf wme on the input link and add a wme to the
 * output link to indicate the match.
 */
void CreateSurfaceMatchProds(const WMEList &cue, list<string> &result) {
	map<SymbolUID, WMEList*> idparmap;  // map from identifier UIDs to the wmes that have them as values
	map<SymbolUID, WMEList*>::iterator k;
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
	int c = 0;
	
	for(i = constwmes.begin(); i != constwmes.end(); ++i) {
		EpmemNS::IdentifierSymbol *id = (*i)->GetId();
		Symbol *attr = (*i)->GetAttr();
		Symbol *val = (*i)->GetVal();
		
		ss.str("");
		ss << "sp {const" << id->GetUID();
		ss << " (state <s> ^io <io>)(<io> ^input-link.state <is> ^output-link <out>)";
		ss << CreateProd(id->GetUID(), idparmap);
		ss << WMEtoCond(*i);
		ss << "--> (<out> ^const " << id->GetUID() << ")}";
		
		result.push_back(ss.str());
	}
	
	for(j = leafids.begin(); j != leafids.end(); ++j) {
		ss.str("");

		ss << "sp {ident" << *j;
		ss << " (state <s> ^io <io>)(<io> ^input-link.state <is> ^output-link <out>)";
		ss << CreateProd(*j, idparmap);
		ss << "--> (<out> ^ident " << *j << ")}";
		
		result.push_back(ss.str());
	}
	
	// clean up memory
	for(k = idparmap.begin(); k != idparmap.end(); ++k) {
		delete k->second;
	}
}

int SoarQuery::SetCue(const WMEList &cue) {
	list<string> prods;
	list<string>::iterator i;
	CreateSurfaceMatchProds(cue, prods);
	
	for(i = prods.begin(); i != prods.end(); ++i) {
		agent->ExecuteCommandLine(i->c_str());
	}
	
	agent->ExecuteCommandLine(CreateGraphMatchProd(cue).c_str());
	
	return prods.size();
}

void SoarQuery::GetMatchedLeafIds(list<SymbolUID> &result) {
	result.insert(result.end(), matchedids.begin(), matchedids.end());
}

int SoarQuery::GetMaxMatchScore() {
	return maxscore;
}

bool SoarQuery::GetGraphMatch() {
	return graphmatch;
}

char const *SoarQuery::PrintCueProductions() {
	return agent->ExecuteCommandLine("print --full");
}
