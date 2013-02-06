#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <vector>
#include "lda.h"
#include "relation.h"
#include "foil.h"
#include "serializable.h"
#include "scene_sig.h"

class em_train_data;
class sig_info;

class clause_info : public serializable {
public:
	clause_info() : nc(NULL) {}
	~clause_info() { if (nc) { delete nc; } }
	
	clause cl;
	relation false_pos;
	relation true_pos;
	num_classifier *nc;
	
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
};

class classifier : public serializable {
public:
	classifier();
	classifier(bool use_foil, bool use_pruning, int nc_type);
	~classifier();

	int vote(int target, const scene_sig &sig, const relation_table &rels, const rvec &x) const;
	void update(const relation &pos, const relation &neg, const relation_table &rels, const std::vector<em_train_data*> &cont_data, const std::vector<sig_info*> &sigs);
	
	void inspect(std::ostream &os) const;
	void inspect_detailed(std::ostream &os) const;
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);

private:
	int const_vote;
	int nc_type;
	bool use_const, use_foil, use_pruning;
	std::vector<clause_info> clauses;
	
	relation false_negatives, true_negatives;
	num_classifier *neg_nc;

	mutable timer_set timers;
};

#endif
