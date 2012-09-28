#ifndef EM_H
#define EM_H

#include <set>
#include <vector>
#include "linear.h"
#include "common.h"
#include "timer.h"
#include "foil.h"
#include "serializable.h"
#include "relation.h"
#include "mat.h"
#include "scene.h"

class scene;

class EM : public serializable {
public:
	EM();
	~EM();
	
	void learn(const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y);
	bool run(int maxiters);
	bool predict(const scene_sig &sig, const relation_table &rels, const rvec &x, int &mode, rvec &y);
	// Return the mode with the model that best fits (x, y)
	int best_mode(const scene_sig &sig, const rvec &x, double y, double &besterror) const;
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;
	void get_map_modes(std::vector<int> &modes) const;
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
private:
	class em_data : public serializable {
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
	
		em_data() : target(-1), time(-1), sig_index(-1), map_mode(-1), model_row(-1) {}
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);
	};
	
	class mode_info : public serializable {
	public:
		mode_info() : pos(2), neg(2), clauses_dirty(true), model(NULL) {}
		
		~mode_info() {
			delete model;
		}
		
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);
		
		bool cli_inspect(int first, const std::vector<std::string> &args, std::ostream &os);

		bool stale;
		int target; // index in sig for the target object
		
		std::set<int> stale_points;
		std::set<int> members;
		scene_sig sig;
		
		LinearModel *model;
		
		// classifier stuff
		relation pos, neg;
		clause_vec mode_clauses;
		
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
	void update_mode_prob(int i, std::set<int> &check);
	void update_MAP(const std::set<int> &pts);
	bool remove_modes();
	void mark_mode_stale(int i);
	bool find_new_mode_inds(const std::set<int> &noise_inds, int sig_ind, std::vector<int> &mode_inds) const;
	double calc_prob(int m, const scene_sig &sig, const rvec &x, double y, int target, std::vector<int> &best_assign, double &best_error) const;
	void mode_add_example(int m, int i, bool update);
	void mode_del_example(int m, int i);
	void init_mode(int mode, int sig, LinearModel *m, const std::vector<int> &members, const std::vector<int> &obj_map);
	void learn_obj_clause(int m, int i);
	void update_clauses(int m);
	void print_foil6_data(std::ostream &os, int mode) const;
	bool map_objs(int mode, int target, const scene_sig &sig, const relation_table &rels, std::vector<int> &mapping) const;
	void extend_relations(const relation_table &add, int time);
	void fill_xy(const std::vector<int> &rows, mat &X, mat &Y) const;

	relation_table rel_tbl;
	std::vector<em_data*> data;
	std::vector<scene_sig> sigs;
	std::vector<mode_info*> modes;
	std::map<int, std::set<int> > noise;  // sig index -> data index
	int ndata, nmodes;
	obj_map_table obj_maps;
	
	enum Timers { E_STEP_T, M_STEP_T, NEW_T };
	timer_set timers;
};


#endif
