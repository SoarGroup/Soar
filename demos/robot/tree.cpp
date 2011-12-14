#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <iterator>
#include <map>

using namespace std;

typedef vector<bool> attribvec;

double entropy(const vector<int> &categories) {
	double ent = 0.;
	map<int, int> counts;
	vector<int>::const_iterator i;
	map<int, int>::iterator j;
	for (i = categories.begin(); i != categories.end(); ++i) {
		if (counts.find(*i) == counts.end()) {
			counts[*i] = 1;
		} else {
			++counts[*i];
		}
	}
	for (j = counts.begin(); j != counts.end(); ++j) {
		double p = ((double) j->second) / categories.size();
		ent += -p * log2(p);
	}
	return ent;
}

int choose_attrib(const vector<attribvec> &attribs, const vector<int> &categories, const list<int> &useable) {
	int highattrib = -1, nattribs = useable.size(), nitems = attribs.size();
	double highgain, curr_ent = entropy(categories);
	
	list<int>::const_iterator i;
	for (i = useable.begin(); i != useable.end(); ++i) {
		vector<int> true_cats, false_cats;
		for (int j = 0; j < nitems; ++j) {
			if (attribs[j][*i]) {
				true_cats.push_back(categories[j]);
			} else {
				false_cats.push_back(categories[j]);
			}
		}
		
		double true_ent = entropy(true_cats) * ((double) true_cats.size()) / nitems;
		double false_ent = entropy(false_cats) * ((double) false_cats.size()) / nitems;
		double gain = curr_ent - (true_ent + false_ent);
		if (highattrib == -1 || gain > highgain) {
			highattrib = *i;
			highgain = gain;
		}
	}
	return highattrib;
}

class DTree {
public:
	DTree(const vector<attribvec> &attribs, const vector<int> &categories)
	: left(NULL), right(NULL), attrib(-1)
	{
		list<int> useable;
		for (int i = 0; i < attribs[0].size(); ++i) {
			useable.push_back(i);
		}
		learn_rec(attribs, categories, useable);
	}
	
	~DTree() {
		delete left;
		delete right;
	}
	
	int classify(const attribvec &attribs) {
		if (attrib < 0) {
			return category;
		}
		if (attribs[attrib]) {
			return left->classify(attribs);
		}
		return right->classify(attribs);
	}
	
	void output(const string &prefix) const {
		if (attrib < 0) {
			cout << prefix << " : " << category << endl;
		} else {
			stringstream lss, rss;
			lss << prefix << " " << attrib << ":t";
			rss << prefix << " " << attrib << ":f";
			if (left) {
				left->output(lss.str());
			}
			if (right) {
				right->output(rss.str());
			}
		}
	}
	
private:
	DTree()
	: left(NULL), right(NULL), attrib(-1), category(90909) // debug val
	{ }
	
	void learn_rec(const vector<attribvec> &attribs, const vector<int> &categories, const list<int> &useable) {
		assert(attribs.size() == categories.size() && attribs.size() > 0);
		
		category = categories[0];
		bool same_cats = true;
		for (int i = 0; i < categories.size(); ++i) {
			if (category != categories[i]) {
				same_cats = false;
				break;
			}
		}
		if (same_cats) {
			return;
		}
		
		bool same_attribs = true;
		for (int i = 0; i < attribs.size(); ++i) {
			if (attribs[i] != attribs[0]) {
				same_attribs = false;
				break;
			}
		}
		
		if (useable.size() == 0 || same_attribs) {
			map<int, int> counts;
			for (int i = 0; i < categories.size(); ++i) {
				int c = categories[i];
				if (counts.find(c) == counts.end()) {
					counts[c] = 1;
				} else {
					++counts[c];
				}
				if ((counts.size() == 1) || (counts[category] < counts[c])) {
					category = c;
				}
			}
			return;
		}
		
		attrib = choose_attrib(attribs, categories, useable);
		vector<attribvec> lattribs, rattribs;
		vector<int> lcats, rcats;
		for (int i = 0; i < categories.size(); ++i) {
			if (attribs[i][attrib]) {
				lattribs.push_back(attribs[i]);
				lcats.push_back(categories[i]);
			} else {
				rattribs.push_back(attribs[i]);
				rcats.push_back(categories[i]);
			}
		}
		
		list<int> child_useable(useable);
		child_useable.remove(attrib);
		if (lattribs.size() > 0) {
			left = new DTree();
			left->learn_rec(lattribs, lcats, child_useable);
		} else {
			left = NULL;
		}
		
		if (rattribs.size() > 0) {
			right = new DTree();
			right->learn_rec(rattribs, rcats, child_useable);
		} else {
			right = NULL;
		}
	}
	
	
	int attrib, category;
	DTree *left;
	DTree *right;
};

int main(int argc, char *argv[]) {
	ifstream datafile("preddata.arff");
	string line;
	vector<attribvec> attribs;
	vector<int> categories;
	
	while (getline(datafile, line)) {
		stringstream ss(line);
		int x;
		
		if (line.size() == 0 || line[0] == '@') {
			continue;
		}
		
		attribvec a;
		for (int i = 0; i < 39; ++i) {
			ss >> x;
			ss.ignore(); // comma
			a.push_back(x != 0);
		}
		attribs.push_back(a);
		ss >> x;
		categories.push_back(x);
	}
	copy(categories.begin(), categories.end(), ostream_iterator<int>(cout, " "));
	cout << endl;
	DTree t(attribs, categories);
	t.output("");
	return 0;
}
