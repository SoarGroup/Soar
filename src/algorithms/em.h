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
	EM(const std::vector<train_inst> &data, const std::vector<propvec_sig> &sigs);
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
	int best_mode(const propvec_sig &sig, const rvec &x, double y, double &besterror) const;
	
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;
	
	const std::vector<int> get_map_modes() const {
		return map_mode;
	}
	
private:
	struct model_data_info {
		std::vector<int> obj_map;  // object variable in model -> object index in instance
		int row;                   // the row in the model's training matrix where this instance appears
	};
	
	class model_info {
	public:
		void save(std::ostream &os) {
			assert(false);
		}
		
		void load(std::istream &is) {
			assert(false);
		}
		
		~model_info() {
			delete model;
		}
		
		LinearModel *model;
		bool stale;
		std::set<int> stale_points;
		std::map<int, model_data_info> data_map; // index into 'data' -> info about that training example
		propvec_sig sig;
	};
	
	void update_eligibility();
	void update_Py_z(int i, std::set<int> &check);
	void update_MAP(const std::set<int> &pts);
	bool remove_models();
	void mark_model_stale(int i);
	bool find_new_mode_inds(const std::set<int> &noise_inds, int sig_ind, std::vector<int> &mode_inds) const;
	double calc_prob(int m, const propvec_sig &sig, const rvec &x, double y, std::vector<int> &best_assign, double &best_error) const;
	void model_add_example(int m, int i, bool update);
	void model_del_example(int m, int i);

	const std::vector<train_inst> &data;
	const std::vector<propvec_sig> &sigs;
	
	dyn_mat Py_z;          // nmodels x ndata
	dyn_mat eligible;      // nmodels x ndata
	
	std::vector<model_info*> models;
	std::vector<int> map_mode;
	
	std::map<int, std::set<int> > noise;  // sig index -> data index
	
	int ndata, nmodels;
	
	enum Timers { E_STEP_T, M_STEP_T, NEW_T };
	timer_set timers;
};


#endif
