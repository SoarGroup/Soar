#ifndef EM_H
#define EM_H

#include <set>
#include <vector>
#include "linear.h"
#include "common.h"

class scene;
class ID5Tree;

typedef std::vector<bool> attr_vec;
typedef int category;

class ClassifierInst {
public:
	attr_vec attrs;
	category cat;
	
	void save(std::ostream &os) const;
	void load(std::istream &is);
};

class EM {
public:
	EM(scene *scn);
	~EM();
	
	void resize();
	
	void update_eligibility();
	void update_Py_z(int i, std::set<int> &check);
	void update_MAP(const std::set<int> &pts);
	void add_data(const rvec &x, double y);
	void estep();
	bool mstep();
	bool unify_or_add_model();
	bool remove_models();
	bool step();
	bool run(int maxiters);
	bool predict(const rvec &x, double &y);
	double error();
	int get_nmodels() const { return nmodels; }
	
	void mark_model_stale(int i);
	void get_tested_atoms(std::vector<int> &atoms) const;
	
	void save(std::ostream &os) const;
	void load(std::istream &is);
	
	void print_tree(std::ostream &os) const;
	void test_classify(const rvec &x, double y, int &best, int &predicted, double &besterror);
	
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::string &out) const;
	
private:
	int classify(const rvec &x);
	
	std::vector<LRModel*> models;
	std::set<int> stale_models;
	std::map<int, std::set<int> > stale_points;
	
	mat  xdata;       // ndata x xdim
	mat  ydata;       // ndata x 1
	mat  Py_z;        // nmodels x ndata
	imat eligible;    // nmodels x ndata
	
	/*
	 This will be read directly by the decision tree learner as the
	 category labels.
	*/
	std::vector<ClassifierInst> class_insts;
	
	int ndata, nmodels, xdim;
	
	scene *scn, *scncopy;
	ID5Tree *dtree;
};


#endif
