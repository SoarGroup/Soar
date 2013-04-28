#ifndef MODE_H
#define MODE_H

#include <set>
#include <vector>
#include <utility>
#include "mat.h"
#include "relation.h"
#include "scene_sig.h"
#include "foil.h"
#include "model.h"
#include "cliproxy.h"

class em_train_data;
class sig_info;

class em_mode : public serializable, public cliproxy {
public:
	em_mode(bool noise, bool manual, const model_train_data &data);
	
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
	void add_example(int i, const std::vector<int> &obj_map);
	void del_example(int i);
	void predict(const scene_sig &s, const rvec &x, const std::vector<int> &obj_map, double &y) const;
	void largest_const_subset(std::vector<int> &subset);
	const std::set<int> &get_noise(int sigindex) const;
	void get_noise_sigs(std::vector<int> &sigs);
	double calc_prob(int target, const scene_sig &sig, const rvec &x, double y, std::vector<int> &best_assign, double &best_error) const;
	bool update_fits();
	
	void get_params(scene_sig &sig, rvec &coefs, double &intercepts) const;
	void set_params(const scene_sig &dsig, int target, const mat &coefs, const rvec &inter);
	bool uniform_sig(int sig, int target) const;
	void get_members(interval_set &m) const;

	int size() const { return members.size(); }
	bool is_new_fit() const { return new_fit; }
	bool is_manual() const { return manual; }
	void reset_new_fit() { new_fit = false; }
	
	const scene_sig &get_sig() const { return sig; }

	bool map_objs(int target, const scene_sig &dsig, const relation_table &rels, std::vector<int> &mapping) const;
	
	int get_num_nonzero_coefs() const;
	
private:
	interval_set members;

	/*
	 For each member instance, there is a mapping from objects tested by the
	 linear function to object indexes in the member's signature, call it the
	 omap. omap is stored as a vector m such that m[i] = j, where i is an index
	 into the mode's signature, and j is an index into the instance's signature.
	 The omap_table associates unique omaps with sets of instances that share that
	 omap.

	 I'm assuming that the number of unique omaps will be small.
	*/
	typedef std::vector<int> omap;
	typedef std::vector<std::pair<omap, interval_set> > omap_table;
	omap_table omaps;
	
	bool stale, noise, new_fit, manual;
	const model_train_data &data;
	
	mat lin_coefs;
	rvec lin_inter;
	int n_nonzero;
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
	mutable std::vector<std::vector<clause> > obj_clauses;
	mutable bool obj_clauses_stale;

	void learn_obj_clauses(const relation_table &rels) const;
	
	void proxy_get_children(std::map<std::string, cliproxy*> &c);
	void proxy_use_sub(const std::vector<std::string> &args, std::ostream &os);
	void cli_clauses(std::ostream &os) const;
	void cli_members(std::ostream &os) const;
};

#endif
