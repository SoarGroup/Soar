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
#include "scene_sig.h"

class LDA;

class EM : public serializable {
public:
	EM();
	~EM();
	
	void learn(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y);
	bool run(int maxiters);
	bool predict(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, int &mode, rvec &y);
	// Return the mode with the model that best fits (x, y)
	int best_mode(int target, const scene_sig &sig, const rvec &x, double y, double &besterror) const;
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;
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
	
	class classifier : public serializable {
	public:
		classifier() : const_class(-1) {}
		
		int const_class;
		clause_vec clauses;
		std::vector<relation*> residuals;
		std::vector<LDA*> ldas;
		
		void inspect(std::ostream &os) const;
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);
	};

	class mode_info : public serializable {
	public:
		mode_info() : member_rel(2), stale(true), classifier_stale(true), model(NULL) {}
		
		~mode_info();
		
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);
		
		bool cli_inspect(int first, const std::vector<std::string> &args, std::ostream &os);

		bool stale, classifier_stale;
		
		std::set<int> stale_points;
		std::set<int> members;
		relation member_rel;
		scene_sig sig;
		LinearModel *model;
		
		/*
		 Each object the model is conditioned on needs to be
		 identifiable with a set of first-order Horn clauses
		 learned with FOIL.
		*/
		std::vector<clause_vec> obj_clauses;
		
		/*
		 Each pair of modes has one classifier associated with it. For
		 mode i, the classifier for it and mode j is stored in the
		 j_th element of this vector. Elements 0 - i of this vector
		 are NULL since those classifiers are already present in a
		 previous mode's classifier vector.
		*/
		std::vector<classifier*> classifiers;
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
	double calc_prob(int m, int target, const scene_sig &sig, const rvec &x, double y, std::vector<int> &best_assign, double &best_error) const;
	void mode_add_example(int m, int i, bool update);
	void mode_del_example(int m, int i);
	void init_mode(int mode, int sig_id, LinearModel *m, const std::vector<int> &members, const std::vector<int> &obj_map);
	void learn_obj_clause(int m, int i);
	void update_clauses(int m);
	bool map_objs(int mode, int target, const scene_sig &sig, const relation_table &rels, std::vector<int> &mapping) const;
	void extend_relations(const relation_table &add, int time);
	void fill_xy(const std::vector<int> &rows, mat &X, mat &Y) const;
	void make_classifier_matrix(const relation &p, const relation &n, mat &m, std::vector<int> &classes) const;
	bool cli_inspect_train(int first, const std::vector<std::string> &args, std::ostream &os) const;
	bool cli_inspect_relations(int i, const std::vector<std::string> &args, std::ostream &os) const;
	bool cli_inspect_classifiers(std::ostream &os) const;
	bool LWR(const scene_sig &sig, const rvec &x, rvec &y) const;

	void update_classifier();
	void update_pair(int i, int j);
	int classify(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, std::vector<int> &obj_map);
	int classify_pair(int i, int j, int target, const scene_sig &sig, const relation_table &rels, const rvec &x) const;

	relation_table rel_tbl;
	std::vector<em_data*> data;
	std::vector<scene_sig> sigs;
	std::vector<mode_info*> modes;
	std::map<int, std::set<int> > noise;  // sig index -> data index
	int ndata, nmodes;
	obj_map_table obj_maps;
	
	std::vector<std::vector<classifier*> > classifiers;
	
	mutable timer_set timers;
};


#endif
