#include <iostream>
#include <iomanip>
#include <vector>
#include "classify.h"
#include "common.h"
#include "dtree.h"
#include "scene.h"
#include "filter_table.h"
#include "lda.h"
#include "mat.h"

using namespace std;

ostream &operator<<(ostream &os, const classifier_inst &inst) {
	for (int i = 0; i < inst.attrs.size(); ++i) {
		os << (inst.attrs[i] ? 1 : 0) << ' ';
	}
	os << " : " << inst.cat << endl;
	return os;
}

istream &operator>>(istream &is, classifier_inst &inst) {
	string x;
	inst.attrs.clear();
	while (true) {
		is >> x;
		if (x == "0") {
			inst.attrs.push_back(false);
		} else if (x == "1") {
			inst.attrs.push_back(true);
		} else if (x == ":") {
			is >> inst.cat;
			break;
		} else {
			assert(false);
		}
	}
	return is;
}

classifier::classifier(const dyn_mat &X, const dyn_mat &Y) 
: X(X), Y(Y), ndata(0), tree(NULL)
{
	timers.add("classify");
	timers.add("LDA");
	timers.add("update");
}

void classifier::new_data(const boolvec &atoms) {
	function_timer t(timers.get(UPDATE_T));

	++ndata;
	assert(ndata == insts.size() + 1);
	classifier_inst i;
	i.attrs = atoms;
	i.cat = -1;
	insts.push_back(i);
	if (!tree) {
		tree = new ID5Tree(insts, atoms.size());
	}
	tree->update_tree(ndata - 1);
	tree->prune();
}

int classifier::classify(const rvec &x, const boolvec &atoms) {
	function_timer t1(timers.get(CLASSIFY_T));

	vector<int> matched_insts;
	vector<category> matched_cats;
	const ID5Tree *matched_node = tree->get_matched_node(atoms);

	matched_node->get_categories(matched_cats);
	if (matched_cats.size() == 0) {
		return -1;
	}
	if (matched_cats.size() == 1) {
		return matched_cats[0];
	}

	matched_node->get_instances(matched_insts);
	if (matched_insts.size() == 0) {
		return -1;
	}
	vector<category> c(matched_insts.size());
	vector<int> signature(matched_insts.size() * 2);
	for (int i = 0; i < matched_insts.size(); ++i) {
		c[i] = insts[matched_insts[i]].cat;
		signature[i * 2] = matched_insts[i];
		signature[i * 2 + 1] = c[i];
	}
	
	LDA_NN_Classifier *lda;
	lda_cache_type::iterator i = lda_cache.find(matched_node);
	if (i != lda_cache.end()) {
		if (i->second.first == signature) {
			lda = i->second.second;
		} else {
			i->second.first = signature;
			delete i->second.second;
			function_timer t2(timers.get(LDA_T));
			lda = new LDA_NN_Classifier(X.get(), matched_insts, c);
			i->second.second = lda;
		}
	} else {
		function_timer t2(timers.get(LDA_T));
		lda = new LDA_NN_Classifier(X.get(), matched_insts, c);
		lda_cache[matched_node] = make_pair(signature, lda);
	}
	
	return lda->classify(x);
}

classifier::~classifier() {
	lda_cache_type::iterator i;
	for (i = lda_cache.begin(); i != lda_cache.end(); ++i) {
		delete i->second.second;
	}
}

void classifier::print_tree(ostream &os) const {
	if (tree) {
		os << "digraph g {" << endl;
		tree->print_graphviz(os);
		os << "}" << endl;
	} else {
		os << "empty" << std::endl;
	}
}

bool classifier::cli_inspect(int first_arg, const vector<string> &args, ostream &os) const {
	if (first_arg >= args.size()) {
		os << "subqueries are: tree train timing" << endl;
		return false;
	}
	
	if (args[first_arg] == "tree") {
		if (tree == NULL) {
			os << "NULL" << endl;
			return false;
		}
		if (first_arg + 1 >= args.size()) {
			tree->print("", os);
			return true;
		}
		int id;
		if (!parse_int(args[first_arg + 1], id)) {
			os << "specify a node number" << endl;
			return false;
		}
		return tree->cli_inspect(id, os);
	} else if (args[first_arg] == "train") {
		for (int i = 0; i < insts.size(); ++i) {
			os << setw(4) << i << " " << insts[i];
		}
		return true;
	} else if (args[first_arg] == "timing") {
		timers.report(os);
		return true;
	}
	return false;
}

void classifier::update(const vector<category> &cats) {
	assert(cats.size() == insts.size());
	
	for (int i = 0; i < cats.size(); ++i) {
		if (insts[i].cat != cats[i]) {
			category old = insts[i].cat;
			insts[i].cat = cats[i];
			tree->update_counts_change(i, old);
		}
	}
	tree->update_tree(-1);
	tree->prune();
}

void classifier::get_tested_atoms(vector<int> &atoms) const {
	if (tree) {
		tree->get_all_splits(atoms);
	}
}

void classifier::save(std::ostream &os) const {
	save_vector(insts, os);
}

void classifier::load(std::istream &is) {
	insts.clear();
	load_vector(insts, is);
	
	if (tree) {
		delete tree;
	}
	ndata = insts.size();
	if (ndata > 0) {
		tree = new ID5Tree(insts, insts[0].attrs.size());
		tree->batch_update();
		tree->prune();
	}
}
