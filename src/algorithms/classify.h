#ifndef CLASSIFY_H
#define CLASSIFY_H

#include <iostream>
#include <vector>
#include "common.h"
#include "lda.h"
#include "timer.h"

class ID5Tree;
class scene;

typedef int category;

class classifier_inst {
public:
	boolvec attrs;
	int cat;
};

std::ostream &operator<<(std::ostream &os, const classifier_inst &inst);
std::istream &operator>>(std::istream &is, classifier_inst &inst);

class classifier {
public:
	classifier(const dyn_mat &X, const dyn_mat &Y);
	~classifier();
	
	void new_data(const boolvec &atoms);
	void update(const std::vector<category> &cats);
	int classify(const rvec &x, const boolvec &atoms);
	
	void print_tree(std::ostream &os) const;
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;
	void get_tested_atoms(std::vector<int> &atoms) const;
	
	void save(std::ostream &os) const;
	void load(std::istream &is);
	
private:
	typedef std::map<const ID5Tree*, std::pair<std::vector<int>, LDA_NN_Classifier*> > lda_cache_type;

	int ndata;
	const dyn_mat &X;
	const dyn_mat &Y;
	std::vector<classifier_inst> insts;
	ID5Tree *tree;
	lda_cache_type lda_cache;
	
	enum Timers {CLASSIFY_T, LDA_T, UPDATE_T};
	timer_set timers;
};

#endif
