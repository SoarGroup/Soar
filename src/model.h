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
#include "soar_interface.h"

class model {
public:
	model(const std::string &name, const std::string &type);
	virtual ~model() {}
	
	void init();
	void finish();
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os);
	
	std::string get_name() const {
		return name;
	}
	
	std::string get_type() const {
		return type;
	}
	
	virtual bool predict(const rvec &x, rvec &y) = 0;
	virtual int get_input_size() const = 0;
	virtual int get_output_size() const = 0;
	
	virtual float test(const rvec &x, const rvec &y);
	virtual void learn(const rvec &x, const rvec &y) {}
	virtual void save(std::ostream &os) const {}
	virtual void load(std::istream &is) {}
	virtual void set_wm_root(Symbol *r) {}
	
protected:
	virtual bool cli_inspect_sub(int first_arg, const std::vector<std::string> &args, std::ostream &os) {
		return false;
	};

private:
	rvec last_pred, last_ref;
	std::string name, type, path;
	std::ofstream predlog;
};

/*
 This class keeps track of how to combine several distinct models to make
 a single prediction.  Its main responsibility is to map the values from
 a single scene vector to the vectors that the individual models expect
 as input, and then map the values of the output vectors from individual
 models back into a single output vector for the entire scene. The mapping
 is specified by the Soar agent at runtime using the assign-model command.
 
 SVS uses a single instance of this class to make all its predictions. I
 may turn this into a singleton in the future.
*/
class multi_model {
public:
	multi_model(std::map<std::string, model*> *model_db);
	~multi_model();
	
	bool predict(const rvec &x, rvec &y);
	void learn(const rvec &x, const rvec &y);
	float test(const rvec &x, const rvec &y);
	
	std::string assign_model (
		const std::string &name, 
		const std::vector<std::string> &inputs, bool all_inputs,
		const std::vector<std::string> &outputs, bool all_outputs );

	void unassign_model(const std::string &name);
	
	void set_property_vector(const std::vector<std::string> &props) {
		prop_vec = props;
	}
	
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;

private:
	struct model_config {
		std::string name;
		std::vector<std::string> xprops;
		std::vector<std::string> yprops;
		std::vector<int> xinds;
		std::vector<int> yinds;
		bool allx;
		bool ally;
		model *mdl;
	};
	
	bool find_indexes(const std::vector<std::string> &props, std::vector<int> &indexes);
	void error_stats_by_dim(int dim, int start, int end, double &mean, double &std, double &min, double &max) const;
	void report_model_config(model_config* c, std::ostream &os) const;
	bool report_error(int i, const std::vector<std::string> &args, std::ostream &os) const;
	
	std::list<model_config*>       active_models;
	std::vector<std::string>       prop_vec;
	std::map<std::string, model*> *model_db;
	
	// measuring prediction errors
	std::vector<rvec> reference_vals;
	std::vector<rvec> predicted_vals;
};

#endif
