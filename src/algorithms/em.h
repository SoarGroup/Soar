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
	
	void save(std::ostream &os) const;
	void load(std::istream &is);
	
	// Return the mode with the model that best fits (x, y)
	int best_mode(const propvec_sig &sig, const rvec &x, double y, double &besterror) const;
	
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;
	
	void get_map_modes(std::vector<int> &modes) const;
	
private:
	class em_data_info {
	public:
		std::vector<double> mode_prob; // mode_prob[i] = probability that data point belongs to mode i
		int map_mode;                  // MAP (Maximum A Posteriori) mode, should always be argmax(mode_prob[i])
		
		// the following are always associated with the MAP mode
		std::vector<int> obj_map;      // object variable in model -> object index in instance
		int model_row;                 // the row in the model's training matrix where this point appears
	
		void save(std::ostream &os) const;
		void load(std::istream &is);
	};
	
	class mode_info {
	public:
		void save(std::ostream &os) {
			assert(false);
		}
		
		void load(std::istream &is) {
			assert(false);
		}
		
		~mode_info() {
			delete model;
		}
		
		LinearModel *model;
		bool stale;
		std::set<int> stale_points;
		std::set<int> members;
		propvec_sig sig;
	};
	
	/*
	 Table to store mappings from placeholders to objects for a
	 particular linear model and data point, as calculated in
	 calc_prob.
	*/
	typedef std::map<std::pair<int, int>, std::vector<int> > obj_map_table;
	
	void update_eligibility();
	void update_mode_prob(int i, std::set<int> &check, obj_map_table &obj_maps);
	void update_MAP(const std::set<int> &pts, const obj_map_table &obj_maps);
	bool remove_modes();
	void mark_mode_stale(int i);
	bool find_new_mode_inds(const std::set<int> &noise_inds, int sig_ind, std::vector<int> &mode_inds) const;
	double calc_prob(int m, const propvec_sig &sig, const rvec &x, double y, std::vector<int> &best_assign, double &best_error) const;
	void model_add_example(int m, int i, bool update);
	void model_del_example(int m, int i);

	const std::vector<train_inst> &data;
	const std::vector<propvec_sig> &sigs;
	
	std::vector<em_data_info> em_info;
	
	std::vector<mode_info*> modes;
	
	std::map<int, std::set<int> > noise;  // sig index -> data index
	int ndata, nmodes;
	
	enum Timers { E_STEP_T, M_STEP_T, NEW_T };
	timer_set timers;
};


#endif
