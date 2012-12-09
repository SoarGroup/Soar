#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "foil.h"
#include "common.h"

using namespace std;

enum {
	MAX_CLAUSES = 10,
};

struct clause_test_spec {
	const char *clause;
	double success_rate;
};

struct test_spec {
	const char *path;
	clause_test_spec clauses[MAX_CLAUSES];
};

test_spec tests[] = {
	{ "foil_tests/1.test", {
		{ "intersect(0,1,2) & ramp(0,2)", 1.0 },
		{ NULL, 0.0} }  // sentinel
	},
	{ "foil_tests/neg.test", {
		{ "~indicator(0,1)", 1.0 },
		{ NULL, 0.0} }
	},
	{ NULL, { {NULL, 0.0} } }  // sentinel
};

double time();
bool run_foil(const char *path, clause_vec &clauses, relation &pos, relation &neg, relation_table &all_rels, double &time);
void standalone(const char *path);
void test();

int main(int argc, char *argv[]) {
	if (argc > 1) {
		standalone(argv[1]);
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


bool run_foil(const char *path, clause_vec &clauses, relation &pos, relation &neg, relation_table &all_rels, double &t) {
	FOIL foil;
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

void standalone(const char *path) {
	clause_vec clauses;
	relation pos, neg;
	relation_table all;
	double t;
	
	if (!run_foil(path, clauses, pos, neg, all, t))
		assert(false);
	
	cout << t << " msecs" << endl;
	for (int i = 0, iend = clauses.size(); i < iend; ++i) {
		double success = clause_success_rate(clauses[i], pos, neg, all);
		cout << clauses[i] << " -> " << success << endl;
	}
}

void test() {
	stringstream ss;
	clause_vec clauses;
	relation pos, neg;
	relation_table all;
	double t;
	int i, j;

	for (int i = 0; tests[i].path != NULL; ++i) {
		clauses.clear();
		if (!run_foil(tests[i].path, clauses, pos, neg, all, t))
			assert(false);
		
		for (j = 0; tests[i].clauses[j].clause != NULL; ++j) {
			const clause_test_spec &spec = tests[i].clauses[j];
			assert(j < clauses.size());
			ss.str("");
			ss << clauses[j];
			assert(ss.str() == spec.clause);
			double success = clause_success_rate(clauses[j], pos, neg, all);
			assert(success == spec.success_rate);
		}
		assert(j == clauses.size());
	}
}
