#ifndef CLASSIFY_H
#define CLASSIFY_H

#include <iostream>
#include <vector>
#include "common.h"

class ID5Tree;
class scene;

typedef int category;
typedef std::vector<bool> attr_vec;

class classifier_inst {
public:
	attr_vec attrs;
	int cat;
	
	void save(std::ostream &os) const;
	void load(std::istream &is);
};

std::ostream &operator<<(std::ostream &os, const classifier_inst &inst);

class classifier {
public:
	classifier(const dyn_mat &X, const dyn_mat &Y, scene *scn);
	~classifier();
	
	void new_data();
	void update(const std::vector<category> &cats);
	void batch_update(const std::vector<category> &classes);
	int classify(const rvec &x);
	
	void print_tree(std::ostream &os) const;
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;
	void get_tested_atoms(std::vector<int> &atoms) const;
	
private:
	int ndata;
	const dyn_mat &X;
	const dyn_mat &Y;
	std::vector<classifier_inst> insts;
	
	scene *scn;
	
	ID5Tree *tree;
};

#endif
