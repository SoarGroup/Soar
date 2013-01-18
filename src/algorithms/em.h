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
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os);
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
private:

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
	
	class train_data : public serializable {
	public:
		rvec x, y;
		int target;
		int time;
		int sig_index;
		
		int mode;
		std::vector<data_mode_info> minfo;
		
		train_data() : target(-1), time(-1), sig_index(-1), mode(0) {}
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);
	};
	
	class classifier : public serializable {
	public:
		classifier() : const_vote(0), use_const(true), neg_lda(NULL) {}
		
		int const_vote;
		bool use_const;
		clause_vec clauses;
		std::vector<relation> false_positives, true_positives;
		std::vector<LDA*> pos_ldas;
		
		relation false_negatives, true_negatives;
		LDA *neg_lda;
		
		void inspect(std::ostream &os) const;
		void inspect_detailed(std::ostream &os) const;
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
	
	class mode_info : public serializable {
	public:
		mode_info(bool noise, const std::vector<train_data*> &data, const std::vector<sig_info*> &sigs);
		~mode_info();
		
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);
		
		bool cli_inspect(int first, const std::vector<std::string> &args, std::ostream &os);
		void add_example(int i);
		void del_example(int i);
		void predict(const scene_sig &s, const rvec &x, const std::vector<int> &obj_map, rvec &y) const;
		void largest_const_subset(std::vector<int> &subset);
		const std::set<int> &get_noise(int sigindex) const;
		void get_noise_sigs(std::vector<int> &sigs);
		double calc_prob(int target, const scene_sig &sig, const rvec &x, double y, std::vector<int> &best_assign, double &best_error) const;
		bool update_fits();
		void set_linear_params(const std::vector<int> &data_inds, const mat &coefs, const rvec &inter);
		bool uniform_sig(int sig, int target) const;
		void learn_obj_clauses(const relation_table &rels);

		int size() const { return members.size(); }
		bool is_new_fit() const { return new_fit; }
		void reset_new_fit() { new_fit = false; }
		
		const std::set<int> &get_members() const { return members; }
		const scene_sig &get_sig() const { return sig; }
		const relation &get_member_rel() const { return member_rel; }

		bool map_objs(int target, const scene_sig &dsig, const relation_table &rels, std::vector<int> &mapping) const;
		
		/*
		 Each pair of modes has one classifier associated with it. For
		 mode i, the classifier for it and mode j is stored in the
		 j_th element of this vector. Elements 0 - i of this vector
		 are NULL since those classifiers are already present in a
		 previous mode's classifier vector.
		*/
		std::vector<classifier*> classifiers;
		
		bool classifier_stale;
		
	private:
		bool stale, noise, new_fit;
		const std::vector<train_data*> &data;
		const std::vector<sig_info*> &sigs;
		
		mat lin_coefs;
		rvec lin_inter;
		std::set<int> members;
		relation member_rel;
		scene_sig sig;
		
		/*
		 Noise data sorted by their Y values. First element in pair is the Y value,
		 second is the index.
		*/
		std::set<std::pair<double, int> > sorted_ys;
		
		/*
		 Each object the model is conditioned on needs to be
		 identifiable with a set of first-order Horn clauses
		 learned with FOIL.
		*/
		std::vector<clause_vec> obj_clauses;
	};
	
	void estep();
	bool mstep();
	void fill_xy(const std::vector<int> &rows, mat &X, mat &Y) const;

	bool unify_or_add_mode();
	int find_linear_subset(mat &X, mat &Y, std::vector<int> &subset, mat &coefs, rvec &inter) const;
	void find_linear_subset_em(const_mat_view X, const_mat_view Y, std::vector<int> &subset) const;
	void find_linear_subset_block(const_mat_view X, const_mat_view Y, std::vector<int> &subset) const;
	bool remove_modes();
	

	void update_classifier();
	void update_pair(int i, int j);
	int classify(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, std::vector<int> &obj_map);
	int vote_pair(int i, int j, int target, const scene_sig &sig, const relation_table &rels, const rvec &x) const;
	LDA *learn_numeric_classifier(const relation &p, const relation &n) const;
	
	bool cli_inspect_train(int first, const std::vector<std::string> &args, std::ostream &os) const;
	bool cli_inspect_relations(int first, const std::vector<std::string> &args, std::ostream &os) const;
	bool cli_inspect_classifiers(int first, const std::vector<std::string> &args, std::ostream &os) const;

	relation_table rel_tbl, context_rel_tbl;
	std::vector<train_data*> data;
	std::vector<sig_info*> sigs;
	std::vector<mode_info*> modes;
	int ndata, nmodes;
	bool use_em, use_foil, use_foil_close, use_lda, use_pruning;

	/*
	 Keeps track of the minimum number of new noise examples needed before we have
	 to check for a possible new mode
	*/
	int check_after;
	
	// noise binned by signature
	std::map<int, std::set<int> > noise_by_sig;
		
	mutable timer_set timers;
};


#endif
