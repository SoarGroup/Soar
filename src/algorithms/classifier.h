#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <vector>
#include "lda.h"
#include "relation.h"
#include "foil.h"
#include "serializable.h"
#include "scene_sig.h"
#include "model.h"
#include "cliproxy.h"

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

class binary_classifier : public serializable, public cliproxy {
public:
	binary_classifier();
	binary_classifier(bool use_foil, bool use_pruning, int nc_type);
	~binary_classifier();

	int vote(int target, const scene_sig &sig, const relation_table &rels, const rvec &x) const;
	void update(const relation &pos, const relation &neg, const relation_table &rels, const model_train_data &train_data);
	
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

/*
 Uses one-to-one voting to extend binary classification to multiple classes.
*/
class classifier : public serializable, public cliproxy {
public:
	classifier(const model_train_data &data);
	~classifier();
	
	void set_options(bool foil, bool prune, bool context, int nc_type);

	void add_class();
	void del_classes(const std::vector<int> &c);
	
	void update_inst(int i, int cls);
	void classify(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, std::vector<int> &votes);

	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
private:
	class pair_info : public serializable {
	public:
		int cls_i, cls_j;
		binary_classifier *clsfr;
		
		pair_info() : cls_i(-1), cls_j(-1), clsfr(NULL) {}
		pair_info(int i, int j, binary_classifier *c) : cls_i(i), cls_j(j), clsfr(c) {}
		~pair_info() { delete clsfr; }
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);
	};
	
	class class_info : public serializable {
	public:
		relation mem_rel;
		bool stale;        // has membership changed?
		
		class_info() : mem_rel(2), stale(false) {}
		
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &os);
	};
	
	const model_train_data &data;
	std::list<pair_info*> pairs;
	std::vector<class_info*> classes;
	std::vector<int> membership;       // class of each instance. membership.size() == data.size()
	
	bool foil, prune, context;
	int nc_type;

	pair_info *find(int i, int j);
	void update();
	
	void proxy_get_children(std::map<std::string, cliproxy*> &c);
	void proxy_use_sub(const std::vector<std::string> &args, std::ostream &os);
	void cli_nc_type(const std::vector<std::string> &args, std::ostream &os);
	void cli_dump_foil6(const std::vector<std::string> &args, std::ostream &os) const;
};

#endif
