#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <map>
#include <vector>
#include "scene.h"
#include "soar_interface.h"
#include "common.h"

class model {
public:
	virtual bool predict(const floatvec &x, floatvec &y) = 0;
	virtual std::string get_type() const = 0;
	virtual int get_input_size() const = 0;
	virtual int get_output_size() const = 0;
	
	virtual void learn(scene *scn, const floatvec &x, const floatvec &y, float dt) {}
	
	virtual float test(const floatvec &x, const floatvec &y) {
		floatvec py(y.size());
		if (!predict(x, py)) {
			return -1.0;
		} else {
			return py.dist(y);
		}
	}
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
	typedef std::map<std::string, std::string> slot_prop_map;

	multi_model() {}
	
	~multi_model() {
		std::list<model_config*>::iterator i;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			delete *i;
		}
	}
	
	bool predict(const floatvec &x, floatvec &y) {
		std::list<model_config*>::const_iterator i;
		int j;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			model_config *cfg = *i;
			DATAVIS() << "BEGIN '" << cfg->name << "'" << std::endl;
			floatvec xp = cfg->allx ? x : x.slice(cfg->xinds);
			floatvec yp(cfg->ally ? y.size() : cfg->yinds.size());
			if (!cfg->mdl->predict(xp, yp)) {
				return false;
			}
			if (cfg->ally) {
				y = yp;
			} else {
				y.set_indices(cfg->yinds, yp);
			}
			DATAVIS() << "END" << std::endl;
		}
		return true;
	}
	
	void learn(scene *scn, const floatvec &x, const floatvec &y, float dt) {
		std::list<model_config*>::iterator i;
		int j;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			model_config *cfg = *i;
			floatvec xp = cfg->allx ? x : x.slice(cfg->xinds);
			floatvec yp = cfg->ally ? y : y.slice(cfg->yinds);
			/*
			float error = cfg->mdl->test(xp, yp);
			if (error >= 0. && error < 1.0e-8) {
				continue;
			}
			*/
			DATAVIS() << "BEGIN '" << cfg->name << "'" << std::endl;
			cfg->mdl->learn(scn, xp, yp, dt);
			DATAVIS() << "END" << std::endl;
		}
	}
	
	float test(const floatvec &x, const floatvec &y) {
		float s = 0.0;
		std::list<model_config*>::iterator i;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			model_config *cfg = *i;
			DATAVIS() << "BEGIN '" << cfg->name << "'" << std::endl;
			floatvec xp = cfg->allx ? x : x.slice(cfg->xinds);
			floatvec yp = cfg->ally ? y : y.slice(cfg->yinds);
			float d = cfg->mdl->test(xp, yp);
			if (d < 0.) {
				DATAVIS() << "'pred error' 'no prediction'" << std::endl;
				s = -1.;
			} else if (s >= 0.) {
				DATAVIS() << "'pred error' " << d << std::endl;
				s += d;
			}
			DATAVIS() << "END" << std::endl;
		}
		if (s >= 0.) {
			return s / active_models.size();
		}
		return -1.;
	}
	
	std::string assign_model(const std::string &name, 
	                         const std::vector<std::string> &inputs, bool all_inputs,
	                         const std::vector<std::string> &outputs, bool all_outputs) 
	{
		model *m;
		model_config *cfg;
		if (!map_get(model_db, name, m)) {
			return "no model";
		}
		
		cfg = new model_config();
		cfg->name = name;
		cfg->mdl = m;
		cfg->allx = all_inputs;
		cfg->ally = all_outputs;
		
		if (all_inputs) {
			if (m->get_input_size() >= 0 && m->get_input_size() != prop_vec.size()) {
				return "size mismatch";
			}
		} else {
			if (m->get_input_size() >= 0 && m->get_input_size() != inputs.size()) {
				return "size mismatch";
			}
			cfg->xprops = inputs;
			if (!find_indexes(inputs, cfg->xinds)) {
				delete cfg;
				return "property not found";
			}
		}
		if (all_outputs) {
			if (m->get_output_size() >= 0 && m->get_output_size() != prop_vec.size()) {
				return "size mismatch";
			}
		} else {
			if (m->get_output_size() >= 0 && m->get_output_size() != outputs.size()) {
				return "size mismatch";
			}
			cfg->yprops = outputs;
			if (!find_indexes(outputs, cfg->yinds)) {
				delete cfg;
				return "property not found";
			}
		}
		active_models.push_back(cfg);
		return "";
	}

	void unassign_model(const std::string &name) {
		std::list<model_config*>::iterator i;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			if ((**i).name == name) {
				active_models.erase(i);
				return;
			}
		}
	}
	
	void add_model(const std::string &name, model *m) {
		model_db[name] = m;
	}
	
	void set_property_vector(const std::vector<std::string> &props) {
		prop_vec = props;
	}
	
private:
	bool find_indexes(const std::vector<std::string> &props, std::vector<int> &indexes) {
		std::vector<std::string>::const_iterator i;

		for (i = props.begin(); i != props.end(); ++i) {
			int index = find(prop_vec.begin(), prop_vec.end(), *i) - prop_vec.begin();
			if (index == prop_vec.size()) {
				std::cerr << "PROPERTY NOT FOUND " << *i << std::endl;
				return false;
			}
			indexes.push_back(index);
		}
		return true;
	}

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
	
	std::list<model_config*>      active_models;
	std::map<std::string, model*> model_db;
	std::vector<std::string>      prop_vec;
};

#endif
