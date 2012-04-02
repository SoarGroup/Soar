#include <iostream>
#include <vector>
#include "classify.h"
#include "common.h"
#include "dtree.h"
#include "scene.h"
#include "filter_table.h"

using namespace std;

void classifier_inst::save(ostream &os) const {
	os << cat << endl;
	save_vector(attrs, os);
}

void classifier_inst::load(istream &is) {
	is >> cat;
	load_vector(attrs, is);
}

classifier::classifier(const mat &X, const mat &Y, scene *scn) 
: X(X), Y(Y), ndata(0), scn(scn->copy())
{
	vector<vector<string> > atoms;
	get_filter_table().get_all_atoms(scn, atoms);
	tree = new ID5Tree(insts, atoms.size());
}

classifier::~classifier() {
	delete scn;
}

void classifier::add(int cat) {
	++ndata;
	assert(ndata == insts.size() + 1);
	scn->set_properties(X.row(ndata - 1));
	classifier_inst i;
	i.attrs = scn->get_atom_vals();
	i.cat = cat;
	insts.push_back(i);
	tree->update_tree(ndata - 1);
	tree->prune();
}

void classifier::change_cat(int i, int new_cat) {
	int old = insts[i].cat;
	insts[i].cat = new_cat;
	tree->update_counts_change(i, old);
}

int classifier::classify(const rvec &x) {
	scn->set_properties(x);
	attr_vec a = scn->get_atom_vals();
	return tree->classify(a);
}

void classifier::batch_update(const vector<category> &classes) {
	if (tree) {
		delete tree;
	}
	insts.clear();
	insts.reserve(classes.size());
	int nattrs;
	for (int i = 0; i < classes.size(); ++i) {
		scn->set_properties(X.row(i));
		classifier_inst inst;
		inst.attrs = scn->get_atom_vals();
		inst.cat = classes[i];
		insts.push_back(inst);
		nattrs = inst.attrs.size();
	}
	ndata = insts.size();
	tree = new ID5Tree(insts, nattrs);
	tree->batch_update();
	tree->prune();
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
		os << "specify an argument" << endl;
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
		char *end;
		int id = strtol(args[first_arg + 1].c_str(), &end, 10);
		if (*end != '\0') {
			os << "specify a node number" << endl;
			return false;
		}
		return tree->cli_inspect(id, os);
	}
	return false;
}

void classifier::update() {
	tree->update_tree(-1);
	tree->prune();
}

void classifier::get_tested_atoms(vector<int> &atoms) const {
	tree->get_all_splits(atoms);
}
