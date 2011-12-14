#include <sstream>
#include <iterator>
#include <utility>
#include "scene.h"
#include "filter.h"

using namespace std;

void all_nodes(scene *scn, vector<vector<string> > &argset) {
	vector<sgnode*> nodes;
	vector<sgnode*>::const_iterator i;
	scn->get_all_nodes(nodes);
	for ( i = nodes.begin(); i != nodes.end(); ++i) {
		vector<string> p;
		p.push_back((**i).get_name());
		argset.push_back(p);
	}
}

void all_node_pairs_unordered_no_repeat(scene *scn, vector<vector<string> > &argset) {
	vector<sgnode*> nodes;
	vector<sgnode*>::const_iterator i;
	vector<string> names;
	vector<string>::const_iterator j, k;
	
	scn->get_all_nodes(nodes);
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		names.push_back((**i).get_name());
	}
	
	for (j = names.begin(); j != names.end(); ++j) {
		for (k = j + 1; k != names.end(); ++k) {
			vector<string> p;
			p.push_back(*j);
			p.push_back(*k);
			argset.push_back(p);
		}
	}
}

void all_node_triples_unordered_no_repeat(scene *scn, vector<vector<string> > &argset) {
	vector<sgnode*> nodes;
	vector<sgnode*>::const_iterator i;
	vector<string> names;
	vector<string>::const_iterator j, k, l;
	
	scn->get_all_nodes(nodes);
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		names.push_back((**i).get_name());
	}
	
	for (j = names.begin(); j != names.end(); ++j) {
		for (k = j + 1; k != names.end(); ++k) {
			for (l = k + 1; l != names.end(); ++l) {
				vector<string> p;
				p.push_back(*j);
				p.push_back(*k);
				p.push_back(*l);
				argset.push_back(p);
			}
		}
	}
}
