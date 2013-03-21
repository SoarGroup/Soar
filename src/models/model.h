#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include "common.h"
#include "mat.h"
#include "relation.h"
#include "scene_sig.h"
#include "cliproxy.h"

struct model_train_inst {
	rvec x, y;
	int target;
	int sig_index;
	const scene_sig *sig;
	
	model_train_inst() : target(-1), sig(NULL) {}
};

class model_train_data : public serializable, public cliproxy {
public:
	model_train_data();
	~model_train_data();
	
	void add(int target, const scene_sig &sig, const relation_table &r, const rvec &x, const rvec &y);
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);

	const model_train_inst &get_inst(int t)  const { return *insts[t]; }
	const model_train_inst &get_last_inst()  const { return *insts.back(); }
	const relation_table &get_all_rels()     const { return all_rels; }
	const relation_table &get_context_rels() const { return context_rels; }
	
	std::vector<const scene_sig*> get_sigs() const {
		return std::vector<const scene_sig*>(sigs.begin(), sigs.end());
	}
	
	int size() const { return insts.size(); }

private:
	void proxy_get_children(std::map<std::string, cliproxy*> &c);
	
	void cli_relations(const std::vector<std::string> &args, std::ostream &os) const;
	void cli_contdata(const std::vector<std::string> &args, std::ostream &os) const;
	void cli_save(const std::vector<std::string> &args, std::ostream &os) const;

	std::vector<scene_sig*> sigs;
	std::vector<model_train_inst*> insts;
	relation_table all_rels, context_rels;
};

class model : public serializable, public cliproxy {
public:
	model(const std::string &name, const std::string &type, bool learning);
	virtual ~model() {}
	
	std::string get_name() const { return name; }
	std::string get_type() const { return type; }
	const model_train_data &get_data() const { return train_data; }
	
	void learn(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y);
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
	virtual bool predict(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y) = 0;
	virtual int get_input_size() const = 0;
	virtual int get_output_size() const = 0;
	virtual void test(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y);

	void proxy_get_children(std::map<std::string, cliproxy*> &c);
	
private:
	std::string name, type;
	bool learning;
	model_train_data train_data;
	
	virtual void update() {}
	virtual void serialize_sub(std::ostream &os) const {}
	virtual void unserialize_sub(std::istream &is) {}

	void cli_save(const std::vector<std::string> &args, std::ostream &os);
	void cli_load(const std::vector<std::string> &args, std::ostream &os);
};

/*
 This class keeps track of how to combine several distinct models to make
 a single prediction.  Its main responsibility is to map the values from
 a single scene vector to the vectors that the individual models expect
 as input, and then map the values of the output vectors from individual
 models back into a single output vector for the entire scene. The mapping
 is specified by the Soar agent at runtime using the assign-model command.
*/
class multi_model : public cliproxy {
public:
	typedef std::vector<std::pair<std::string, std::string> > prop_vec;
	
	multi_model(std::map<std::string, model*> *model_db);
	~multi_model();
	
	bool predict(const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y);
	void learn(const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y);
	void test(const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y);
	void unassign_model(const std::string &name);
	
	std::string assign_model (
		const std::string &name, 
		const prop_vec &inputs, bool all_inputs,
		const prop_vec &outputs);
	
private:
	struct model_config {
		std::string name;
		prop_vec xprops;
		prop_vec yprops;
		bool allx;
		model *mdl;
	};
	
	struct test_info {
		scene_sig sig;
		rvec x;
		rvec y;
		rvec pred;
		rvec error;
	};
	
	bool predict_or_test(bool test, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y);
	
	void proxy_get_children(std::map<std::string, cliproxy*> &c);
	void cli_error(const std::vector<std::string> &args, std::ostream &os) const;
	void cli_assign(std::ostream &os) const;

	std::list<model_config*>       active_models;
	std::map<std::string, model*> *model_db;
	
	// record prediction errors
	std::vector<test_info> tests;
};

#endif
