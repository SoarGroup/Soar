#ifndef EM_H
#define EM_H

#include <set>
#include <vector>
#include "linear.h"
#include "common.h"
#include "classify.h"
#include "timer.h"

class scene;

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
	
	void save(std::ostream &os) const;
	void load(std::istream &is);
	
	void test_classify(const rvec &x, double y, int &best, int &predicted, double &besterror);
	
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;
	
	const classifier &get_classifier() const { return clsfr; }
	
private:
	std::vector<LRModel*> models;
	std::set<int> stale_models;
	std::map<int, std::set<int> > stale_points;
	std::vector<category> map_class;
	
	mat  xdata;       // ndata x xdim
	mat  ydata;       // ndata x 1
	mat  Py_z;        // nmodels x ndata
	imat eligible;    // nmodels x ndata
	
	int ndata, nmodels, xdim;
	
	classifier clsfr;
	
	enum Timers { E_STEP_T, M_STEP_T };
	timer_set timers;
};


#endif
