#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <cassert>
#include "foil.h"
#include "common.h"

using namespace std;

#define MAX_CLAUSES 10
#define EQUAL_MARGIN 1.0e-3

struct clause_test_spec {
	const char *clause;
	double success_rate;
};

struct test_spec {
	const char *path;
	bool prune;
	clause_test_spec clauses[MAX_CLAUSES];
};

test_spec tests[] = {
	{ "foil_tests/1.test", true, {
		{ "intersect(0,1,2) & ramp(0,2)", 1.0 },
		{ NULL, 0.0} }  // sentinel
	},
	{ "foil_tests/neg.test", true, {
		{ "~indicator(0,1)", 1.0 },
		{ NULL, 0.0} }
	},
	{ "foil_tests/ramp.test", true, {
		{ "intersect(0,1,2)", 0.975 },
		{ NULL, 0.0} }
	},
	{ "foil_tests/ramp_prune.test", true, {
		{ "~intersect(0,1,-1)",          0.889 },
		{ "intersect(0,1,2) & box(0,2)", 0.405 },
		{ NULL, 0.0} }
	},
	{ NULL, false, { {NULL, 0.0} } }  // sentinel
};

bool close(double a, double b);
double time();
bool run_foil(const char *path, bool prune, clause_vec &clauses, relation &pos, relation &neg, relation_table &all_rels, double &time);
void standalone(const char *path, bool prune);
void test();
void test_clauses(clause_vec &clauses, relation &pos, relation &neg, relation_table &all_rels);

int main(int argc, char *argv[]) {
	//LOG.turn_on(FOILDBG);
	if (argc > 1) {
		if (strcmp(argv[1], "-p") == 0) {
			standalone(argv[2], false);
		} else {
			standalone(argv[1], true);
		}
		exit(0);
	}
	
	test();
	return 0;
}


double time() {
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1e9 + ts.tv_nsec) / 1.0e6;
}


bool run_foil(const char *path, bool prune, clause_vec &clauses, relation &pos, relation &neg, relation_table &all_rels, double &t) {
	FOIL foil(prune);
	ifstream input(path);
	
	if (!input) {
		cerr << "could not load " << path << endl;
		assert(false);
	}
	
	if (!foil.load_foil6(input))
		return false;
	
	input.close();
	double t1 = time();
	foil.learn(clauses, NULL);
	t = time() - t1;
	pos = foil.get_pos();
	neg = foil.get_neg();
	all_rels = foil.get_relations();
	return true;
}

void standalone(const char *path, bool prune) {
	clause_vec clauses;
	relation pos, neg;
	relation_table all;
	double t;
	
	if (!run_foil(path, prune, clauses, pos, neg, all, t))
		assert(false);
	
	cout << t << " msecs" << endl;
	for (int i = 0, iend = clauses.size(); i < iend; ++i) {
		cout << i << ": " << clauses[i] << endl;
	}
	test_clauses(clauses, pos, neg, all);
}

void test() {
	stringstream ss;
	clause_vec clauses;
	relation pos, neg;
	relation_table all;
	double t, success, fp, fn;
	int i, j;

	for (int i = 0; tests[i].path != NULL; ++i) {
		clauses.clear();
		if (!run_foil(tests[i].path, tests[i].prune, clauses, pos, neg, all, t))
			assert(false);
		
		for (j = 0; tests[i].clauses[j].clause != NULL; ++j) {
			const clause_test_spec &spec = tests[i].clauses[j];
			assert(j < clauses.size());
			ss.str("");
			ss << clauses[j];
			assert(ss.str() == spec.clause);
			clause_success_rate(clauses[j], pos, neg, all, success, fp, fn);
			assert(close(success, spec.success_rate));
		}
		assert(j == clauses.size());
	}
}

bool close(double a, double b) {
	return fabs(a - b) < EQUAL_MARGIN;
}

void test_clauses(clause_vec &clauses, relation &pos, relation &neg, relation_table &all_rels) {
	relation::const_iterator i, iend;
	int matched, false_negs, false_pos;
	var_domains d;
	
	cout << "False negatives" << endl;
	for (false_negs = 0, i = pos.begin(), iend = pos.end(); i != iend; ++i) {
		d.clear();
		for (int j = 0, jend = i->size(); j < jend; ++j) {
			d[j].insert((*i)[j]);
		}
		matched = test_clause_vec(clauses, all_rels, d);
		if (matched < 0) {
			join(cout, *i, ", ") << endl;
			++false_negs;
		}
	}
	
	cout << "False positives" << endl;
	for (false_pos = 0, i = neg.begin(), iend = neg.end(); i != iend; ++i) {
		d.clear();
		for (int j = 0, jend = i->size(); j < jend; ++j) {
			d[j].insert((*i)[j]);
		}
		matched = test_clause_vec(clauses, all_rels, d);
		if (matched >= 0) {
			cout << "(" << matched << ") ";
			join(cout, *i, ", ") << " | ";
			var_domains::const_iterator di, diend;
			for (di = d.begin(), diend = d.end(); di != diend; ++di) {
				assert(di->second.size() == 1);
				int val = *di->second.begin();
				cout << di->first << " <- " << val << " ";
			}
			cout << endl;
			++false_pos;
		}
	}
	
	int total = pos.size() + neg.size();
	int correct = total - false_negs - false_pos;
	double success_rate = correct / static_cast<double>(total);
	cout << endl
	     << success_rate << " success, " << correct << " correct, " << false_negs + false_pos << " incorrect, "
	     << false_negs << " false negs, " << false_pos << " false pos" << endl;
}

