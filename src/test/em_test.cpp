#include <stdlib.h>
#include <fstream>
#include <vector>
#include "em.h"
#include "scene.h"
#include "dtree.h"

using namespace std;

void load_data(const char *path, EM &em) {
	ifstream file(path);
	string line;
	
	while (getline(file, line)) {
		int i = line.find_first_not_of(" \t");
		if (i == string::npos || line[i] == '#') {
			continue;
		}
		i = line.find(';');
		if (i == string::npos) {
			cerr << "data file error" << endl;
			exit(1);
		}
		stringstream xpart(line.substr(0, i));
		stringstream ypart(line.substr(i + 1));
		
		vector<double> vals;
		double v;
		while (xpart >> v) {
			vals.push_back(v);
		}
		floatvec x(vals.size());
		for (int j = 0; j < vals.size(); ++j) {
			x[j] = vals[j];
		}
		
		vals.clear();
		while (ypart >> v) {
			vals.push_back(v);
		}
		
		em.add_data(x, vals[0]);
		em.run(5);
	}
}

int main() {
	srand(1);
	vector<attr_vec> predvals;
	vector<int> categories;
	vector<string> prednames;
	ifstream sgel("scene.sgel");
	string line;
	scene scn("test", "world", false);
	
	while (getline(sgel, line)) {
		scn.parse_sgel(line);
	}
	scn.get_predicate_names(prednames);
	
	EM em(0.001, &scn);
	load_data("emdata", em);
	cout << "Final MAP:";
	for (int i = 0; i < em.ndata; ++i) {
		cout << em.dtree_insts[i].cat << " ";
		categories.push_back(em.dtree_insts[i].cat);
	}
	cout << endl;
	
	cout << "Final Tree:" << endl;
	em.dtree->output(prednames);
	
	/*
	DecisionTree *dtree = new DecisionTree(predvals, categories);
	dtree->output(prednames);
	*/
	return 0;
}
