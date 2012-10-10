#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <iterator>
#include <limits>
#include "model.h"

using namespace std;

void slice(const rvec &source, rvec &target, const vector<int> &indexes) {
	target.resize(indexes.size());
	for (int i = 0; i < indexes.size(); ++i) {
		target(i) = source(indexes[i]);
	}
}

void dassign(const rvec &source, rvec &target, const vector<int> &indexes) {
	assert(source.size() == indexes.size());
	for (int i = 0; i < indexes.size(); ++i) {
		assert(0 <= indexes[i] && indexes[i] < target.size());
		target[indexes[i]] = source[i];
	}
}

bool find_prop_inds(const scene_sig &sig, const multi_model::prop_vec &pv, vector<int> &obj_inds, vector<int> &prop_inds) {
	int oind, pind;
	for (int i = 0; i < pv.size(); ++i) {
		const string &obj = pv[i].first;
		const string &prop = pv[i].second;
		if (!sig.get_dim(pv[i].first, pv[i].second, oind, pind)) {
			return false;
		}
		if (obj_inds.empty() || obj_inds.back() != oind) {
			obj_inds.push_back(oind);
		}
		prop_inds.push_back(pind);
	}
	return true;
}

model::model(const std::string &name, const std::string &type) 
: name(name), type(type)
{}

bool model::cli_inspect(int first_arg, const vector<string> &args, ostream &os) {
	if (first_arg < args.size()) {
		if (args[first_arg] == "save") {
			string path;
			if (first_arg + 1 >= args.size()) {
				path = name + ".model";
			} else {
				path = args[first_arg + 1];
			}
			ofstream f(path.c_str());
			if (!f.is_open()) {
				os << "cannot open file " << path << " for writing" << endl;
				return false;
			}
			serialize(f);
			f.close();
			os << "saved to " << path << endl;
			return true;
		} else if (args[first_arg] == "load") {
			string path;
			if (first_arg + 1 >= args.size()) {
				path = name + ".model";
			} else {
				path = args[first_arg + 1];
			}
			ifstream f(path.c_str());
			if (!f.is_open()) {
				os << "cannot open file " << path << " for reading" << endl;
				return false;
			}
			unserialize(f);
			f.close();
			os << "loaded from " << path << endl;
			return true;
		}
	}
	return cli_inspect_sub(first_arg, args, os);
}

multi_model::multi_model(map<string, model*> *model_db) : model_db(model_db) {}

multi_model::~multi_model() {
	std::list<model_config*>::iterator i;
	for (i = active_models.begin(); i != active_models.end(); ++i) {
		delete *i;
	}
}

bool multi_model::predict(const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y) {
	std::list<model_config*>::const_iterator i;
	for (i = active_models.begin(); i != active_models.end(); ++i) {
		model_config *cfg = *i;
		assert(cfg->allx); // don't know what to do with the signature when we have to slice
		
		rvec yp(cfg->yprops.size());
		vector<int> yinds, yobjs;
		
		find_prop_inds(sig, cfg->yprops, yobjs, yinds);
		/*
		 I'm going to start making the assumption that all
		 models only predict the properties of a single
		 object. Clean this part up later.
		*/
		assert(yobjs.size() == 1);
		if (!cfg->mdl->predict(yobjs[0], sig, rels, x, yp)) {
			return false;
		}
		dassign(yp, y, yinds);
	}
	return true;
}

void multi_model::learn(const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y) {
	std::list<model_config*>::iterator i;
	int j;
	for (i = active_models.begin(); i != active_models.end(); ++i) {
		model_config *cfg = *i;
		assert(cfg->allx); // don't know what to do with the signature when we have to slice
		
		rvec yp;
		vector<int> yinds, yobjs;
		
		find_prop_inds(sig, cfg->yprops, yobjs, yinds);
		assert(yobjs.size() == 1);
		slice(y, yp, yinds);
		cfg->mdl->learn(yobjs[0], sig, rels, x, yp);
	}
}

bool multi_model::test(const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y) {
	rvec predicted(y.size());
	predicted.setConstant(0.0);
	test_x.push_back(x);
	test_y.push_back(y);
	test_rels.push_back(rels);
	reference_vals.push_back(y);

	if (!predict(sig, rels, x, predicted)) {
		predicted_vals.push_back(rvec());
		return false;
	}
	predicted_vals.push_back(predicted);
	return true;
}

string multi_model::assign_model
( const string &name, 
  const prop_vec &inputs, bool all_inputs,
  const prop_vec &outputs)
{
	model *m;
	model_config *cfg;
	if (!map_get(*model_db, name, m)) {
		return "no model";
	}
	
	cfg = new model_config();
	cfg->name = name;
	cfg->mdl = m;
	
	cfg->allx = all_inputs;
	if (!all_inputs) {
		if (m->get_input_size() >= 0 && m->get_input_size() != inputs.size()) {
			return "size mismatch";
		}
		cfg->xprops = inputs;
	}
	
	if (m->get_output_size() >= 0 && m->get_output_size() != outputs.size()) {
		return "size mismatch";
	}
	cfg->yprops = outputs;
	
	active_models.push_back(cfg);
	return "";
}

void multi_model::unassign_model(const string &name) {
	std::list<model_config*>::iterator i;
	for (i = active_models.begin(); i != active_models.end(); ++i) {
		if ((**i).name == name) {
			active_models.erase(i);
			return;
		}
	}
}

bool multi_model::report_error(int i, const vector<string> &args, ostream &os) const {
	if (reference_vals.empty()) {
		os << "no model error data" << endl;
		return false;
	}
	
	int dim = -1, start = 0, end = reference_vals.size() - 1;
	bool list = false, histo = false;
	if (i < args.size() && args[i] == "list") {
		list = true;
		++i;
	} else if (i < args.size() && args[i] == "histogram") {
		histo = true;
		++i;
	}
	if (i >= args.size()) {
		os << "specify a dimension" << endl;
		return false;
	}
	if (!parse_int(args[i], dim)) {
		os << "invalid dimension" << endl;
		return false;
	}
	if (++i < args.size()) {
		if (!parse_int(args[i], start)) {
			os << "require integer start time" << endl;
			return false;
		}
		if (start < 0 || start >= reference_vals.size()) {
			os << "start time must be in [0, " << reference_vals.size() - 1 << "]" << endl;
			return false;
		}
	}
	if (++i < args.size()) {
		if (!parse_int(args[i], end)) {
			os << "require integer end time" << endl;
			return false;
		}
		if (end <= start || end >= reference_vals.size()) {
			os << "end time must be in [start time, " << reference_vals.size() - 1 << "]" << endl;
			return false;
		}
	}
	
	if (list) {
		os << "num real pred error null norm" << endl;
		for (int j = start; j <= end; ++j) {
			os << setw(4) << j << " ";
			if (dim >= reference_vals[j].size() || dim >= predicted_vals[j].size()) {
				os << "NA" << endl;
			} else {
				double error, null_error, norm_error;
				error = fabs(reference_vals[j](dim) - predicted_vals[j](dim));
				if (j > 0 && (null_error = fabs(reference_vals[j-1](dim) - reference_vals[j](dim))) > 0) {
					norm_error = error / null_error;
				} else {
					null_error = -1;
					norm_error = -1;
				}
				os << reference_vals[j](dim) << " " << predicted_vals[j](dim) << " " 
				   << error << " ";
				if (null_error < 0) {
					os << "NA ";
				} else {
					os << null_error << " ";
				}
				if (norm_error < 0) {
					os << "NA";
				} else {
					os << norm_error;
				}
				os << endl;
			}
		}
	} else if (histo) {
		vector<double> errors;
		for (int j = start; j <= end; ++j) {
			errors.push_back(fabs(reference_vals[j](dim) - predicted_vals[j](dim)));
		}
		histogram(errors, 10, os) << endl;
	} else {
		double mean, mode, std, min, max;
		error_stats_by_dim(dim, start, end, mean, mode, std, min, max);
		os << "mean " << mean << endl
		   << "std  " << std << endl
		   << "mode " << mode << endl
		   << "min  " << min << endl
		   << "max  " << max << endl;
	}
	return true;
}

void multi_model::error_stats_by_dim(int dim, int start, int end, double &mean, double &mode, double &std, double &min, double &max) const {
	assert(dim >= 0 && start >= 0 && end <= reference_vals.size());
	double total = 0.0;
	min = INFINITY; 
	max = 0.0;
	std = 0.0;
	vector<double> ds;
	for (int i = start; i <= end; ++i) {
		if (dim >= reference_vals[i].size() || dim >= predicted_vals[i].size()) {
			continue;
		}
		double r = reference_vals[i](dim);
		double p = predicted_vals[i](dim);
		double d = fabs(r - p);
		ds.push_back(d);
		total += d;
		if (d < min) {
			min = d;
		}
		if (d > max) {
			max = d;
		}
	}
	mean = total / ds.size();
	for (int i = 0; i < ds.size(); ++i) {
		std += pow(ds[i] - mean, 2);
	}
	std = sqrt(std / ds.size());
	sort(ds.begin(), ds.end());
	mode = ds[ds.size() / 2];
}

void multi_model::report_model_config(model_config* c, ostream &os) const {
	const char *indent = "  ";
	os << c->name << endl;
	if (c->allx) {
		os << indent << "xdims: all" << endl;
	} else {
		os << indent << "xdims: ";
		for (int i = 0; i < c->xprops.size(); ++i) {
			os << c->xprops[i].first << ":" << c->xprops[i].second << " ";
		}
		os << endl;
	}
	os << indent << "ydims: ";
	for (int i = 0; i < c->yprops.size(); ++i) {
		os << c->yprops[i].first << ":" << c->yprops[i].second << " ";
	}
	os << endl;
}

bool multi_model::cli_inspect(int i, const vector<string> &args, ostream &os) const {
	if (i >= args.size()) {
		os << "available subqueries are: assignment error" << endl;
		return false;
	}
	if (args[i] == "assignment") {
		std::list<model_config*>::const_iterator j;
		for (j = active_models.begin(); j != active_models.end(); ++j) {
			report_model_config(*j, os);
		}
		return true;
	} else if (args[i] == "error") {
		return report_error(++i, args, os);
	}
	os << "no such query" << endl;
	return false;
}
