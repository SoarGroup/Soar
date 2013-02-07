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
#include "lwr.h"
#include "lda.h"
#include "classifier.h"
#include "mode.h"

class em_train_data : public serializable {
public:
	/*
	 Holds information about how a training data point relates to each mode
	*/
	class data_mode_info : public serializable {
	public:
		double prob;                // probability that data point belongs to mode
		bool prob_stale;            // does prob need to be update?
		std::vector<int> obj_map;   // mapping from variable in mode sig -> object index in instance
	
		data_mode_info() : prob(0), prob_stale(true) {}
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);
	};

	rvec x, y;
	int target;
	int time;
	int sig_index;
	
	int mode;
	std::vector<data_mode_info> minfo;
	
	em_train_data() : target(-1), time(-1), sig_index(-1), mode(0) {}
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
};

class sig_info : public serializable {
public:
	sig_info();
	scene_sig sig;
	std::vector<int> members;  // indexes of data points with this sig
	LWR lwr;                   // lwr model trained on all points of this sig

	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
};

class EM : public serializable {
public:
	EM();
	~EM();
	
	void learn(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y);
	bool run(int maxiters);
	bool predict(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, int &mode, rvec &y);
	// Return the mode with the model that best fits (x, y)
	int best_mode(int target, const scene_sig &sig, const rvec &x, double y, double &besterror) const;
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os);
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
private:
	void estep();
	bool mstep();
	void fill_xy(const std::vector<int> &rows, mat &X, mat &Y) const;

	bool unify_or_add_mode();
	int find_linear_subset(mat &X, mat &Y, std::vector<int> &subset, mat &coefs, rvec &inter) const;
	void find_linear_subset_em(const_mat_view X, const_mat_view Y, std::vector<int> &subset) const;
	void find_linear_subset_block(const_mat_view X, const_mat_view Y, std::vector<int> &subset) const;
	mode_info *add_mode(bool manual);
	bool remove_modes();

	void update_classifier();
	void update_pair(int i, int j);
	int classify(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, std::vector<int> &obj_map);
	int vote_pair(int i, int j, int target, const scene_sig &sig, const relation_table &rels, const rvec &x) const;
	
	bool cli_inspect_train(int first, const std::vector<std::string> &args, std::ostream &os) const;
	bool cli_dump_train(int first, const std::vector<std::string> &args, std::ostream &os) const;
	bool cli_inspect_relations(int first, const std::vector<std::string> &args, std::ostream &os) const;
	bool cli_inspect_classifiers(int first, const std::vector<std::string> &args, std::ostream &os) const;
	bool cli_add_mode(int first, const std::vector<std::string> &args, std::ostream &os);

	relation_table rel_tbl, context_rel_tbl;
	std::vector<em_train_data*> data;
	std::vector<sig_info*> sigs;
	std::vector<mode_info*> modes;
	int ndata, nmodes;
	bool use_em, use_foil, use_foil_close, use_nc, use_pruning, use_unify, learn_new_modes;

	/*
	 Keeps track of the minimum number of new noise examples needed before we have
	 to check for a possible new mode
	*/
	int check_after;
	
	// noise binned by signature
	std::map<int, std::set<int> > noise_by_sig;
	
	int nc_type;
	
	mutable timer_set timers;
};


#endif
