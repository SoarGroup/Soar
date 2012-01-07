#ifndef EM_H
#define EM_H

#include <set>
#include <vector>
#include <armadillo>
#include "linear.h"
#include "common.h"
#include "dtree.h"

class scene;

class EM {
public:
	EM(scene *scn);
	~EM();
	
	void resize();
	
	void update_eligibility();
	void update_Py_z(int i, std::set<int> &check);
	void update_MAP(const std::set<int> &pts);
	void add_data(const floatvec &x, double y);
	void estep();
	bool mstep();
	bool unify_or_add_model();
	bool remove_models();
	bool step();
	bool run(int maxiters);
	bool predict(const floatvec &x, float &y);
	double error();
	
	void mark_model_stale(int i);
	void get_tested_atoms(std::vector<int> &atoms) const;
	
	void save(std::ostream &os) const;
	void load(std::istream &is);
	
private:
	std::vector<LRModel*> models;
	std::set<int> stale_models;
	std::map<int, std::set<int> > stale_points;
	
	arma::mat  xdata;       // ndata x xdim
	arma::vec  ydata;       // ndata x 1
	arma::mat  Py_z;        // nmodels x ndata
	arma::imat eligible;    // nmodels x ndata
	
	/*
	 This will be read directly by the decision tree learner as the
	 category labels.
	*/
	std::vector<DTreeInst> dtree_insts;
	
	int ndata, nmodels, xdim;
	
	scene *scn, *scncopy;
	ID5Tree *dtree;
};


#endif
