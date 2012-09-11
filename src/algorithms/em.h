#ifndef EM_H
#define EM_H

#include <set>
#include <vector>
#include "linear.h"
#include "common.h"
#include "timer.h"
#include "foil.h"

class scene;

class EM {
public:
	EM(const relation_table &rel_tbl);
	~EM();
	
	void learn(const state_sig &sig, const rvec &x, const rvec &y, int time);
	bool run(int maxiters);
	bool predict(const state_sig &sig, const rvec &x, const relation_table &rels, int &mode, rvec &y);
	
	void save(std::ostream &os) const;
	void load(std::istream &is);
	
	// Return the mode with the model that best fits (x, y)
	int best_mode(const state_sig &sig, const rvec &x, double y, double &besterror) const;
	
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;
	
	void get_map_modes(std::vector<int> &modes) const;
	
private:
	class em_data {
	public:
		rvec x, y;
		int target;
		int time;
		int sig_index;
		
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
		mode_info() : pos(1), neg(1), clauses_dirty(true) {}
		
		~mode_info() {
			delete model;
		}
		
		void save(std::ostream &os) {
			assert(false);
		}
		
		void load(std::istream &is) {
			assert(false);
		}
		
		bool cli_inspect(int first, const std::vector<std::string> &args, std::ostream &os);

		LinearModel *model;
		bool stale;
		std::set<int> stale_points;
		std::set<int> members;
		state_sig sig;
		int target; // index in sig for the target object
		
		// classifier stuff
		relation pos, neg;
		clause_vec ident_clauses;
		
		/*
		 Each object the model is conditioned on needs to be
		 identifiable with a set of first-order Horn clauses
		 learned with FOIL.
		*/
		std::vector<clause_vec> obj_clauses;
		
		bool clauses_dirty;
	};
	
	/*
	 Table to store mappings from placeholders to objects for a
	 particular linear model and data point, as calculated in
	 calc_prob.
	*/
	typedef std::map<std::pair<int, int>, std::vector<int> > obj_map_table;
	
	void estep();
	bool mstep();
	bool unify_or_add_model();
	bool step();
	void update_eligibility();
	void update_mode_prob(int i, std::set<int> &check, obj_map_table &obj_maps);
	void update_MAP(const std::set<int> &pts, const obj_map_table &obj_maps);
	bool remove_modes();
	void mark_mode_stale(int i);
	bool find_new_mode_inds(const std::set<int> &noise_inds, int sig_ind, std::vector<int> &mode_inds) const;
	double calc_prob(int m, const state_sig &sig, const rvec &x, double y, int target, std::vector<int> &best_assign, double &best_error) const;
	void mode_add_example(int m, int i, bool update);
	void mode_del_example(int m, int i);
	void add_mode(int sig, LinearModel *m, const std::vector<int> &seed_inds, const std::vector<int> &obj_map);
	void learn_obj_clause(int m, int i);
	void update_clauses(int m);
	void print_foil6_data(std::ostream &os, int mode) const;
	
	const relation_table &rel_tbl;
	std::vector<em_data*> data;
	std::vector<state_sig> sigs;
	std::vector<mode_info*> modes;
	std::map<int, std::set<int> > noise;  // sig index -> data index
	int ndata, nmodes;
	
	enum Timers { E_STEP_T, M_STEP_T, NEW_T };
	timer_set timers;
};


#endif
