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
	
	if (out) {
		for (i = out->GetChildrenBegin(); i != out->GetChildrenEnd(); ++i) {
			if (strcmp((*i)->GetAttribute(), "graphmatch") == 0) {
				q->graphmatch = true;
			} else {
				q->matchedids.push_back((*i)->ConvertToIntElement()->GetValue());
			}
		}
	}
}

SoarQuery::SoarQuery(const WMEList &cue) 
: wmemap(), idmap(), uidwmemap(), matchedids(), graphmatch(false)
{
	kernel = Kernel::CreateKernelInCurrentThread(Kernel::kDefaultLibraryName, true, 0);
	agent = kernel->CreateAgent("epmem");
	agent->ExecuteCommandLine("set-stop-phase --after --output");
	agent->ExecuteCommandLine("wait -e");
	
	istate = agent->CreateIdWME(agent->GetInputLink(), "state");
	list<Identifier*> *idlist = new list<Identifier*>();
	idlist->push_back(istate);
	idmap[0] = idlist; // top state always has UID 0
	
	maxscore = SetCue(cue);
	
	kernel->RegisterForUpdateEvent(smlEVENT_AFTER_ALL_OUTPUT_PHASES, SoarUpdateHandler, this);
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

/*
 * Remove all references to disconnected wmes in local data structures.
 * We have to do this because SML doesn't report back the recursive
 * deletions that result from deleting a single wme.
 */
void SoarQuery::CleanUpDisconnected() {
	WMEList::iterator i;
	WMEList::iterator j;
	map<EpmemNS::SymbolUID, std::list<sml::Identifier*>*>::iterator k;
	map<EpmemNS::WME*, sml::WMElement*>::iterator l;
	
	WMEList wmes;
	list<sml::Identifier*>* idlist;
	
	WME *w;
	WMElement *smlwme;
	WMEUID uid;
	
	for (l = wmemap.begin(); l != wmemap.end(); ++l) {
		wmes.push_back(l->first);
	}
	
	i = wmes.begin();
	bool change = true;
	while (change) {
		change = false;
		while (i != wmes.end()) {
			w = (*i);
			smlwme = wmemap[w];
			
			uid = w->GetId()->GetUID();
			if (uid == 0) {
				++i;
				continue;
			}
			bool hasparent = false;
			for (j = wmes.begin(); j != wmes.end(); ++j) {
				if ((*j)->GetVal()->GetUID() == uid) {
					hasparent = true;
					break;
				}
			}
			if (!hasparent) {
				if (w->GetVal()->GetType() == Symbol::IdSym) {
					k = idmap.find(w->GetVal()->GetUID());
					idlist = k->second;
					idlist->remove(static_cast<Identifier*>(smlwme));
					if (idlist->size() == 0) {
						delete k->second;
						idmap.erase(k);
					}
				}
				uidwmemap.erase(w->GetUID());
				wmemap.erase(wmemap.find(w));
				
				i = wmes.erase(i);
				change = true;
			} else {
				++i;
			}
		}
	}
}

void SoarQuery::DeleteWME(WMEUID uid) {
	// wme already deleted recursively
	if (uidwmemap.find(uid) == uidwmemap.end()) {
		return;
	}
	
	WME *wme = uidwmemap[uid];
	WMElement *smlwme = wmemap[wme];
	Symbol *val = wme->GetVal();
	list<Identifier*> *idlist;
	map<long, list<Identifier*>*>::iterator i;
	
	if (val->GetType() == Symbol::IdSym) {
		idlist = idmap[val->GetUID()];
		idlist->remove(static_cast<Identifier*>(smlwme));  // dynamic_cast will fail if pointer already freed
		if (idlist->size() == 0) {
			idmap.erase(idmap.find(val->GetUID()));
			delete idlist;
		}
	}
	
	agent->DestroyWME(smlwme);
	uidwmemap.erase(uidwmemap.find(uid));
	wmemap.erase(wmemap.find(wme));
	CleanUpDisconnected();
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

char const *SoarQuery::GetState() {
	return agent->ExecuteCommandLine("print -d 100 -t i2");
}

/*
 * otherids contains a list of identifier names that we explicitly
 * declare not equals relationships with
 */
string WMEtoCond(WME *w, set<const char*> *otherids = NULL) {
	set<const char*>::iterator i;
	stringstream ss;
	Symbol *id = w->GetId();
	Symbol *attr = w->GetAttr();
	Symbol *val = w->GetVal();
	
	if (id->GetUID() == 0) {
		ss << "(<is>";
	} else {
		ss << "(<" << id->GetString() << ">";
	}
	if (attr->GetType() == Symbol::IdSym) {
		ss << " ^<" << attr->GetString() << ">";
	}
	else {
		ss << " ^" << attr->GetString();
	}
	
	if (val->GetType() == Symbol::IdSym) {
		if (otherids) {
			ss << " { <" << val->GetString() << ">";
			for (i = otherids->begin(); i != otherids->end(); ++i) {
				if (strcmp(*i, val->GetString()) != 0) {
					ss << " <> <" << *i << ">";
				}
			}
			ss << " }";
		} else {
			ss << " <" << val->GetString() << ">";
		}
	} else {
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
	set<const char*> idnames;
	list<SymbolUID>::iterator i;
	WMEList *parents;
	WMEList::iterator j;
	EpmemNS::IdentifierSymbol *id;
	Symbol *attr, *val;
	WMEList includes;
	stringstream ss;
	
	ids.push_back(leaf);
	
	for (i = ids.begin(); i != ids.end(); ++i) {
		parents = idparmap[*i];
		for (j = parents->begin(); j != parents->end(); ++j) {
			includes.push_back(*j);
			id = (*j)->GetId();
			attr = (*j)->GetAttr();
			val = (*j)->GetVal();
			if (id->GetUID() != 0) {  // it's not state id
				ids.push_back(id->GetUID());
				idnames.insert(id->GetString());
			}
			if (attr->GetType() == Symbol::IdSym) {
				idnames.insert(attr->GetString());
			}
			if (val->GetType() == Symbol::IdSym) {
				idnames.insert(val->GetString());
			}
		}
	}
	
	for (j = includes.begin(); j != includes.end(); ++j) {
		ss << WMEtoCond(*j, &idnames) << endl;
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
	
	for(i = constwmes.begin(); i != constwmes.end(); ++i, ++c) {
		EpmemNS::IdentifierSymbol *id = (*i)->GetId();
		Symbol *attr = (*i)->GetAttr();
		Symbol *val = (*i)->GetVal();
		
		ss.str("");
		ss << "sp {const" << c;
		ss << " (state <s> ^io <io>)(<io> ^input-link.state <is> ^output-link <out>)";
		if (id->GetUID() != 0) {  // UID = 0 means S1, which doesn't have parents
			ss << CreateProd(id->GetUID(), idparmap);
		}
		ss << WMEtoCond(*i);
		ss << "--> (<out> ^const " << c << ")}";
		
		result.push_back(ss.str());
	}
	
	for(j = leafids.begin(); j != leafids.end(); ++j, ++c) {
		ss.str("");

		ss << "sp {ident" << *j;
		ss << " (state <s> ^io <io>)(<io> ^input-link.state <is> ^output-link <out>)";
		ss << CreateProd(*j, idparmap);
		ss << "--> (<out> ^ident " << c << ")}";
		
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
		cout << *i << endl;
		cout << agent->ExecuteCommandLine(i->c_str()) << endl;
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

char const *SoarQuery::GetCueProductions() {
	return agent->ExecuteCommandLine("print --full");
}
