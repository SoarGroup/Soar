#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <cassert>
#include "foil.h"
#include "common.h"
#include "serialize.h"

using namespace std;

typedef vector<clause> clause_vec;

struct multi_classifier {
	vector<int> clses;
	map<pair<int, int>, clause_vec> pair_clauses;
};

struct inst {
	int time;
	int cls;
};

struct data {
	relation_table rels;
	int target;
	vector<inst> insts;
};

data train;
int train_start, train_end, test_start, test_end;
bool print_clauses, print_votes, use_context;
vector<string> test_files;

double gettime() {
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1e9 + ts.tv_nsec) / 1.0e6;
}

void get_rels_at_time(const relation_table &rels, int time, relation_table &out) {
	tuple tt(1);
	tt[0] = time;
	relation_table::const_iterator i, iend;
	relation::const_iterator j, jend;
	for (i = rels.begin(), iend = rels.end(); i != iend; ++i) {
		relation &r = out[i->first];
		r.reset(i->second.arity());
		for (j = i->second.begin(), jend = i->second.end(); j != jend; ++j) {
			if ((*j)[0] == time) {
				tuple t = *j;
				t[0] = 0;
				r.add(t);
			}
		}
	}
}

void learn(const relation &pos, const relation &neg, const relation_table &rels, clause_vec &clauses) {
	FOIL foil;
	
	foil.set_problem(pos, neg, rels);
	foil.learn(true, false);
	for (int i = 0, iend = foil.num_clauses(); i < iend; ++i) {
		clauses.push_back(foil.get_clause(i));
	}
}

void multi_learn(const data &train, multi_classifier &mc) {
	map<int, relation> insts_by_class;
	for (int i = 0, iend = train.insts.size(); i < iend; ++i) {
		int c = train.insts[i].cls;
		relation &r = insts_by_class[c];
		if (r.arity() == 0) {
			r.reset(2);
		}
		r.add(i, train.target);
	}
	
	map<int, relation>::iterator i, j, end;
	for (i = insts_by_class.begin(), end = insts_by_class.end(); i != end; ++i) {
		for ((j = i)++; j != end; ++j) {
			pair<int, int> p(i->first, j->first);
			learn(i->second, j->second, train.rels, mc.pair_clauses[p]);
		}
	}
}

bool classify(const vector<clause> &clauses, const relation_table &rels, int target) {
	var_domains d;
	d[0].insert(0);
	d[1].insert(target);
	for (int i = 0, iend = clauses.size(); i < iend; ++i) {
		var_domains d1 = d;
		if (test_clause(clauses[i], rels, d1)) {
			return true;
		}
	}
	return false;
}

int multi_classify(const multi_classifier &c, const relation_table &rels, int target) {
	map<int, int> votes;
	
	map<pair<int, int>, clause_vec>::const_iterator k, kend;
	for (k = c.pair_clauses.begin(), kend = c.pair_clauses.end(); k != kend; ++k) {
		int i = k->first.first, j = k->first.second;
		if (classify(k->second, rels, target)) {
			votes[i]++;
			if (print_votes) {
				cout << "[" << i << "]: " << j << endl;
			}
		} else {
			votes[j]++;
			if (print_votes) {
				cout << " " << i << " :[" << j << "]" << endl;
			}
		}
	}
	
	int best = -1;
	map<int, int>::iterator i, iend;
	for (i = votes.begin(), iend = votes.end(); i != iend; ++i) {
		if (best < 0 || votes[best] < i->second) {
			best = i->first;
		}
	}
	return best;
}

void print_multi(const multi_classifier &mc) {
	map<pair<int, int>, clause_vec>::const_iterator i, iend;
	for (i = mc.pair_clauses.begin(), iend = mc.pair_clauses.end(); i != iend; ++i) {
		cout << "For classes " << i->first.first << ", " << i->first.second << endl;
		for (int j = 0, jend = i->second.size(); j < jend; ++j) {
			cout << "\t" << i->second[j] << endl;
		}
	}
}

void parse_args(int argc, char *argv[]) {
	int opt_end;
	string line;
	
	train_start = test_start = 0;
	train_end = test_end = -1;
	print_clauses = print_votes = use_context = false;
	for (int i = 1; i < argc;) {
		if (strcmp(argv[i], "-train") == 0) {
			if (i + 2 >= argc || !parse_int(argv[i+1], train_start) || !parse_int(argv[i+2], train_end)) {
				cerr << "invalid training range" << endl;
				exit(1);
			}
			i += 3;
		} else if (strcmp(argv[i], "-test") == 0) {
			if (i + 2 >= argc || !parse_int(argv[i+1], test_start) || !parse_int(argv[i+2], test_end)) {
				cerr << "invalid testing range" << endl;
				exit(1);
			}
			i += 3;
		} else if (strcmp(argv[i], "-p") == 0) {
			print_clauses = true;
			i++;
		} else if (strcmp(argv[i], "-v") == 0) {
			print_votes = true;
			i++;
		} else if (strcmp(argv[i], "-c") == 0) {
			use_context = true;
			i++;
		} else if (argv[i][0] != '-') {
			opt_end = i;
			break;
		} else {
			cerr << "unrecognized option: " << argv[i] << endl;
			exit(1);
		}
	}
	
	if (argc - opt_end < 3) {
		cerr << "usage: " << argv[0] << " <options> <train_target> <train_rels> <train_classes> [<test_rels> ... ]" << endl;
		cerr << "options:" << endl
		     << "\t-train <start> <end> : training range" << endl
		     << "\t-test  <start> <end> : testing range" << endl
		     << "\t-p                   : print clauses" << endl
		     << "\t-v                   : print votes" << endl
		     << "\t-c                   : use context only" << endl;
		exit(1);
	}
	
	if (!parse_int(argv[opt_end], train.target)) {
		cerr << "illegal target" << endl;
		exit(1);
	}
	
	ifstream train_rels_in(argv[opt_end + 1]), train_class_in(argv[opt_end + 2]);
	if (!train_rels_in || !train_class_in) {
		cerr << "error opening files" << endl;
		exit(1);
	}
	unserializer(train_rels_in) >> train.rels;
	
	for (int t = 0; getline(train_class_in, line); ++t) {
		inst i;
		i.time = t;
		if (!parse_int(line.c_str(), i.cls)) {
			cerr << "invalid training class on line " << t + 1 << endl;
			exit(1);
		}
		if (t >= train_start && (t <= train_end || train_end == -1)) {
			train.insts.push_back(i);
		}
	}
	
	for (int i = opt_end + 3; i < argc; ++i) {
		test_files.push_back(argv[i]);
	}
}

int main(int argc, char *argv[]) {
	parse_args(argc, argv);

	multi_classifier mc;
	multi_learn(train, mc);
	
	if (print_clauses) {
		print_multi(mc);
	}
	
	for (int i = 0, iend = test_files.size(); i < iend; ++i) {
		ifstream testin(test_files[i].c_str());
		if (!testin) {
			cerr << "error opening " << test_files[i] << endl;
			exit(1);
		}
		relation_table test_rels;
		unserializer(testin) >> test_rels;
		interval_set test_times;
		
		test_rels["ball"].at_pos(0, test_times);
		interval_set::const_iterator t, tend;
		for (t = test_times.begin(), tend = test_times.end(); t != tend; ++t) {
			if (*t >= test_start && (*t <= test_end || test_end == -1)) {
				int c;
				relation_table rt, crt;
				get_rels_at_time(test_rels, *t, rt);
				if (use_context) {
					get_context_rels(train.target, rt, crt);
					c = multi_classify(mc, crt, train.target);
				} else {
					c = multi_classify(mc, rt, train.target);
				}
				if (print_votes) {
					cout << "class ";
				}
				cout << c << endl;
			}
		}
	}
	
	return 0;
}
