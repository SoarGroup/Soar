#ifndef EM_H
#define EM_H

#include <set>
#include <vector>
#include "linear.h"
#include "common.h"
#include "timer.h"

class scene;

class EM {
public:
	EM(const dyn_mat &xdata, const dyn_mat &ydata);
	~EM();
	
	void new_data();
	void estep();
	bool mstep();
	bool unify_or_add_model();
	bool step();
	bool run(int maxiters);
	bool predict(int mode, const rvec &x, double &y);
	int get_nmodels() const { return nmodels; }
	
	void save(std::ostream &os) const;
	void load(std::istream &is);
	
	// Return the mode with the model that best fits (x, y)
	int best_mode(const rvec &x, double y, double &besterror) const;
	
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;
	
	const std::vector<int> get_map_modes() const {
		return map_mode;
	}
	
private:
	void update_eligibility();
	void update_Py_z(int i, std::set<int> &check);
	void update_MAP(const std::set<int> &pts);
	bool remove_models();
	void mark_model_stale(int i);
	bool find_new_mode_inds(std::vector<int> &inds) const;
	
	const dyn_mat &xdata;
	const dyn_mat &ydata;
	dyn_mat Py_z;          // nmodels x ndata
	dyn_mat eligible;      // nmodels x ndata
	
	std::vector<LRModel*> models;
	std::set<int> stale_models;
	std::map<int, std::set<int> > stale_points;
	std::vector<int> map_mode;
	
	std::set<int> old_noise_inds;
	std::set<int> noise_inds;
	
	int ndata, nmodels;
	
	enum Timers { E_STEP_T, M_STEP_T, NEW_T };
	timer_set timers;
};


#endif
