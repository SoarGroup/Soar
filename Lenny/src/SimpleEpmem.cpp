#include "SimpleEpmem.h"

#include "Symbol.h"
#include "IdentifierSymbol.h"

#include <assert.h>
#include <sstream>

SimpleEpmem::SimpleEpmem() : eps() {
	symfactory = new SimpleSymbolFactory();
}
	
SimpleEpmem::~SimpleEpmem() {
	vector<list<WME*>*>::iterator i;
	for (i = eps.begin(); i != eps.end(); ++i) {
		delete *i;
	}
	delete symfactory;
}

/* delete wmes that aren't connected to root state */
void deleteDisconnected(list<WME*> &ep) {
	list<WME*>::iterator i;
	list<WME*>::iterator j;
	
	i = ep.begin();
	bool change = true;
	while (change) {
		change = false;
		while (i != ep.end()) {
			long uid = (*i)->GetId()->GetUID();
			if (uid == 0) {
				++i;
				continue;
			}
			bool hasparent = false;
			for (j = ep.begin(); j != ep.end(); ++j) {
				if ((*j)->GetVal()->GetUID() == uid) {
					hasparent = true;
					break;
				}
			}
			if (!hasparent) {
				i = ep.erase(i);
				change = true;
			} else {
				++i;
			}
		}
	}
}

/*
 * Add a new episode by copying the last episode and making the
 * specified changes.
 */
int SimpleEpmem::AddEpisode(const list<WME*> &addlist, const list<long> &dellist) {
	list<WME*> *newep;
	if (eps.size() == 0) {
		newep = new list<WME*>();
	} else {
		newep = new list<WME*>(*eps.back());
	}
	
	newep->insert(newep->end(), addlist.begin(), addlist.end());
	
	list<long>::iterator i;
	list<WME*>::iterator j;
	list<long> dellistcopy(dellist);

	for (i = dellistcopy.begin(); i != dellistcopy.end(); ++i) {
		bool found = false;
		for (j = newep->begin(); j != newep->end(); ++j) {
			if ((*j)->GetUID() == *i) {
				newep->erase(j);
				found = true;
				break;
			}
		}
		assert(found);
	}

	deleteDisconnected(*newep);
	eps.push_back(newep);
}

int SimpleEpmem::Retrieve(int episode, list<WME*> &result) {
	if (episode < 0 || episode >= eps.size()) return -1;
	list<WME*> *e = eps[episode];
	result.insert(result.end(), e->begin(), e->end());
	return e->size();
}

/* Not implemented */
QueryResult SimpleEpmem::Query(const list<WME*> &cue) {
	QueryResult r;
	r.episode = -1;
	r.matchScore = -1;
	r.graphMatch = false;
	return r;
}

string SimpleEpmem::GetString() {
	stringstream ss;
	int c;
	vector<list<WME*>*>::iterator i;
	list<WME*>::iterator j;
	for (i = eps.begin(), c = 0; i != eps.end(); ++i, ++c) {
		ss << "Episode " << c << endl;
		for (j = (*i)->begin(); j != (*i)->end(); ++j) {
			ss << "  " << (*j)->GetString() << endl;
		}
	}
	
	return ss.str();
}