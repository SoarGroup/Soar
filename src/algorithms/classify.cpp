#include <iostream>
#include <iomanip>
#include <vector>
#include "classify.h"
#include "common.h"
#include "dtree.h"
#include "scene.h"
#include "filter_table.h"
#include "lda.h"

using namespace std;

void get_nonuniform_cols(const_mat_view data, vector<int> &cols) {
	for (int j = 0; j < data.cols(); ++j) {
		for (int i = 1; i < data.rows(); ++i) {
			if (data(i, j) != data(0, j)) {
				cols.push_back(j);
				break;
			}
		}
	}
}

void clean_data(const_mat_view data, mat &cleaned, vector<int> &nonuniform_cols) {
	get_nonuniform_cols(data, nonuniform_cols);
	cleaned.resize(data.rows(), nonuniform_cols.size());
	for (int i = 0; i < nonuniform_cols.size(); ++i) {
		cleaned.col(i) = data.col(nonuniform_cols[i]);
	}
	mat rand_offsets = mat::Random(cleaned.rows(), cleaned.cols()) / 10000;
	cleaned += rand_offsets;
}

int largest_class(const vector<int> &membership) {
	map<int, int> counts;
	for (int i = 0; i < membership.size(); ++i) {
		++counts[membership[i]];
	}
	map<int, int>::iterator i;
	int largest_count = -1;
	int largest = -1;
	for (i = counts.begin(); i != counts.end(); ++i) {
		if (largest_count < i->second) {
			largest = i->first;
		}
	}
	return largest;
}

ostream &operator<<(ostream &os, const classifier_inst &inst) {
	for (int i = 0; i < inst.attrs.size(); ++i) {
		os << (inst.attrs[i] ? 1 : 0) << ' ';
	}
	os << inst.cat;
	return os;
}

void classifier_inst::save(ostream &os) const {
	os << cat << endl;
	save_vector(attrs, os);
}

void classifier_inst::load(istream &is) {
	is >> cat;
	load_vector(attrs, is);
}

classifier::classifier(const mat &X, const mat &Y, scene *scn) 
: X(X), Y(Y), ndata(0), scn(scn->clone())
{
	vector<string> atoms;
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
	
	vector<int> matched_insts;
	tree->get_matched_node(a)->get_instances(matched_insts);
	if (matched_insts.size() == 0) {
		return -1;
	}
	mat Xm(matched_insts.size(), X.cols());
	vector<category> c(matched_insts.size());
	bool uniform = true;
	for (int i = 0; i < matched_insts.size(); ++i) {
		Xm.row(i) = X.row(matched_insts[i]);
		c[i] = insts[matched_insts[i]].cat;
		if (c[i] != c[0]) {
			uniform = false;
		}
	}
	
	if (uniform) {
		return c[0];
	}
	
	mat cleaned;
	vector<int> nonuniform_cols;
	clean_data(Xm, cleaned, nonuniform_cols);
	
	if (c.size() > 0 && cleaned.cols() == 0) {
		LOG(WARN) << "Degenerate case, no useful classification data." << endl;
		return largest_class(c);
	}
	
	LDA_NN_Classifier lda(cleaned, c);
	
	int result;
	if (nonuniform_cols.size() != x.size()) {
		rvec x1(nonuniform_cols.size());
		for (int i = 0; i < nonuniform_cols.size(); ++i) {
			x1(i) = x(nonuniform_cols[i]);
		}
		result = lda.classify(x1);
	} else {
		result = lda.classify(x);
	}
	
	return result;
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
		os << "subqueries are: tree train" << endl;
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
			os << setw(4) << i << " " << insts[i] << endl;
		}
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
