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
	void add_data(const evec &x, double y);
	void estep();
	bool mstep();
	bool unify_or_add_model();
	bool remove_models();
	bool step();
	bool run(int maxiters);
	bool predict(const evec &x, double &y);
	double error();
	int get_nmodels() const { return nmodels; }
	
	void mark_model_stale(int i);
	void get_tested_atoms(std::vector<int> &atoms) const;
	
	void save(std::ostream &os) const;
	void load(std::istream &is);
	
	void print_tree(std::ostream &os) const;
	void test_classify(const evec &x, double y, int &best, int &predicted, double &besterror);
	
private:
	int classify(const evec &x);
	
	std::vector<LRModel*> models;
	std::set<int> stale_models;
	std::map<int, std::set<int> > stale_points;
	
	mat  xdata;       // ndata x xdim
	evec ydata;       // ndata
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
