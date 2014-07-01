#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <iterator>
#include <limits>
#include "serialize.h"
#include "model.h"
#include "svs.h"

using namespace std;

model *_make_null_model_(svs *owner, const string &name);
model *_make_lwr_model_ (svs *owner, const string &name);
model *_make_em_model_  (svs *owner, const string &name);

struct model_constructor_table_entry {
	const char *type;
	model* (*func)(svs*, const string&);
};

static model_constructor_table_entry constructor_table[] = {
	{ "null",        _make_null_model_},
	{ "lwr",         _make_lwr_model_},
	{ "em",          _make_em_model_},
};

model *make_model(svs *owner, const string &name, const string &type) {
	int table_size = sizeof(constructor_table) / sizeof(model_constructor_table_entry);

	for (int i = 0; i < table_size; ++i) {
		if (type == constructor_table[i].type) {
			return constructor_table[i].func(owner, name);
		}
	}
	return NULL;
}

void slice(const rvec &src, rvec &tgt, const vector<int> &srcinds, const vector<int> &tgtinds) {
	if (srcinds.empty() && tgtinds.empty()) {
		int n = max(src.size(), tgt.size());
		tgt.head(n) = src.head(n);
	} else if (srcinds.empty()) {
		for (int i = 0; i < tgtinds.size(); ++i) {
			tgt(tgtinds[i]) = src(i);
		}
	} else {
		for (int i = 0; i < srcinds.size(); ++i) {
			tgt(i) = src(srcinds[i]);
		}
	}
}

bool find_prop_inds(const scene_sig &sig, const vector<multi_model::obj_prop> &pv, vector<int> &obj_inds, vector<int> &prop_inds) {
	int oind, pind;
	for (int i = 0; i < pv.size(); ++i) {
		if (!sig.get_dim(pv[i].object, pv[i].property, oind, pind)) {
			return false;
		}
		if (obj_inds.empty() || obj_inds.back() != oind) {
			obj_inds.push_back(oind);
		}
		prop_inds.push_back(pind);
	}
	return true;
}

bool split_props(const string &unsplit, multi_model::obj_prop &out) {
	vector<string> fields;
	split(unsplit, ":", fields);
	if (fields.size() != 2) {
		return false;
	}
	out.object = fields[0];
	out.property = fields[1];
	return true;
}

model::model(const std::string &name, const std::string &type, bool learning) 
: name(name), type(type), learning(learning)
{}

void model::learn(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y) {
	if (learning) {
		train_data.add(target, sig, rels, x, y);
		update();
	}
}

void model::proxy_get_children(map<string, cliproxy*> &c) {
	c["save"] = new memfunc_proxy<model>(this, &model::cli_save);
	c["load"] = new memfunc_proxy<model>(this, &model::cli_load);
	c["data"] = &train_data;
}

void model::cli_save(const vector<string> &args, ostream &os) {
	string path;
	if (args.empty()) {
		path = name + ".model";
	} else {
		path = args[0];
	}
	ofstream f(path.c_str());
	if (!f.is_open()) {
		os << "cannot open file " << path << " for writing" << endl;
		return;
	}
	serialize(f);
	f.close();
	os << "saved to " << path << endl;
}

void model::cli_load(const vector<string> &args, ostream &os) {
	string path;
	if (args.empty()) {
		path = name + ".model";
	} else {
		path = args[0];
	}
	ifstream f(path.c_str());
	if (!f.is_open()) {
		os << "cannot open file " << path << " for reading" << endl;
		return;
	}
	unserialize(f);
	f.close();
	os << "loaded from " << path << endl;
}

void model::serialize(ostream &os) const {
	serializer sr(os);
	sr << name << type << learning << '\n';
	sr << train_data << '\n';
	serialize_sub(os);
}

void model::unserialize(istream &is) {
	unserializer(is) >> name >> type >> learning >> train_data;
	unserialize_sub(is);
}

multi_model::multi_model(map<string, model*> *model_db)
: model_db(model_db)
{
}

multi_model::~multi_model() {
	std::list<model_config*>::iterator i;
	for (i = active_models.begin(); i != active_models.end(); ++i) {
		delete *i;
	}
}

bool multi_model::predict(const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y) {
	return predict_or_test(false, sig, rels, x, y);
}

void multi_model::test(const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y) {
	rvec ry = y;
	predict_or_test(true, sig, rels, x, ry);
}

/*
 When testing, the expectation is that y initially contains the
 reference values.
*/
bool multi_model::predict_or_test(bool test, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y) {
	rvec yorig = y;
	std::list<model_config*>::const_iterator i;
	for (i = active_models.begin(); i != active_models.end(); ++i) {
		model_config *cfg = *i;
		assert(cfg->allx); // don't know what to do with the signature when we have to slice
		
		rvec yp(cfg->yprops.size());
		yp.setConstant(NAN);
		vector<int> yinds, yobjs;
		
		find_prop_inds(sig, cfg->yprops, yobjs, yinds);
		/*
		 I'm going to start making the assumption that all
		 models only predict the properties of a single
		 object. Clean this part up later.
		*/
		assert(yobjs.size() == 1);
		if (test) {
			slice(yorig, yp, yinds, vector<int>());
			cfg->mdl->test(yobjs[0], sig, rels, x, yp);
		} else {
			cfg->mdl->predict(yobjs[0], sig, rels, x, yp);
		}
		slice(yp, y, vector<int>(), yinds);
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
		yp.resize(yinds.size());
		slice(y, yp, yinds, vector<int>());
		cfg->mdl->learn(yobjs[0], sig, rels, x, yp);
	}
}

string multi_model::assign_model
( const string &name, 
  const vector<string> &inputs, bool all_inputs,
  const vector<string> &outputs)
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
			return "input size mismatch";
		}
		for (int i = 0, iend = inputs.size(); i < iend; ++i) {
			obj_prop p;
			if (!split_props(inputs[i], p)) {
				return "invalid property name";
			}
			cfg->xprops.push_back(p);
		}
	}
	
	if (m->get_output_size() >= 0 && m->get_output_size() != outputs.size()) {
		return "output size mismatch";
	}
	for (int i = 0, iend = outputs.size(); i < iend; ++i) {
		obj_prop p;
		if (!split_props(outputs[i], p)) {
			return "invalid property name";
		}
		cfg->yprops.push_back(p);
	}
	
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

void multi_model::proxy_use_sub(const vector<string> &args, ostream &os) {
	const char *indent = "  ";
	std::list<model_config*>::const_iterator j;
	for (j = active_models.begin(); j != active_models.end(); ++j) {
		const model_config* c = *j;
		os << c->name << endl;
		if (c->allx) {
			os << indent << "xdims: all" << endl;
		} else {
			os << indent << "xdims: ";
			for (int i = 0; i < c->xprops.size(); ++i) {
				os << c->xprops[i].object << ":" << c->xprops[i].property << " ";
			}
			os << endl;
		}
		os << indent << "ydims: ";
		for (int i = 0; i < c->yprops.size(); ++i) {
			os << c->yprops[i].object << ":" << c->yprops[i].property << " ";
		}
		os << endl;
	}
}

void multi_model::proxy_get_children(std::map<std::string, cliproxy*> &c) {
	c["assign"] = new memfunc_proxy<multi_model>(this, &multi_model::cli_assign);
	c["assign"]->set_help("Assign a model to predict properties.")
	             .add_arg("NAME", "Model name.")
	             .add_arg("[-i OBJECT:PROPERTY ...]", "List of input properties.")
	             .add_arg("[[-o] OBJECT:PROPERTY ...]", "List of output properties.");
}

void multi_model::cli_assign(const vector<string> &args, ostream &os) {
	vector<string> inputs, outputs;
	bool all_inputs;
	char type;
	
	if (args.size() < 1) {
		os << "Specify model name." << endl;
		return;
	}
	
	// assume using all inputs unless inputs explicitly provided
	all_inputs = true;
	type = 'o';
	for (int i = 1, iend = args.size(); i < iend; ++i) {
		if (args[i] == "-i") {
			all_inputs = false;
			type = 'i';
			continue;
		}
		if (args[i] == "-o") {
			type = 'o';
			continue;
		}
		if (type == 'i') {
			inputs.push_back(args[i]);
		} else {
			outputs.push_back(args[i]);
		}
	}
	if (outputs.empty()) {
		os << "Specify at least one output." << endl;
		return;
	}
	os << assign_model(args[0], inputs, all_inputs, outputs) << endl;
}

model_train_data::model_train_data() {}

model_train_data::~model_train_data() {
	clear_and_dealloc(sigs);
	clear_and_dealloc(insts);
}

void model_train_data::add(int target, const scene_sig &sig, const relation_table &r, const rvec &x, const rvec &y) {
	model_train_inst *inst = new model_train_inst;
	for (int i = 0, iend = sigs.size(); i < iend; ++i) {
		if (*sigs[i] == sig) {
			inst->sig_index = i;
			inst->sig = sigs[i];
			break;
		}
	}
	
	if (inst->sig == NULL) {
		scene_sig *newsig = new scene_sig;
		*newsig = sig;
		sigs.push_back(newsig);
		inst->sig_index = sigs.size() - 1;
		inst->sig = newsig;
	}
	
	inst->x = x;
	inst->y = y;
	inst->target = target;
	insts.push_back(inst);
	
	relation_table c;
	extend_relations(all_rels, r, insts.size() - 1);
	::get_context_rels(sig[target].id, r, c);
	extend_relations(context_rels, c, insts.size() - 1);
}

void model_train_data::serialize(ostream &os) const {
	serializer sr(os);
	
	sr << "MODEL_TRAIN_DATA" << sigs.size() << insts.size() << '\n';
	for (int i = 0, iend = sigs.size(); i < iend; ++i) {
		sigs[i]->serialize(os);
	}
	sr << '\n' << "CONTINUOUS_DATA_BEGIN" << '\n';
	
	for (int i = 0, iend = insts.size(); i < iend; ++i) {
		const model_train_inst *inst = insts[i];
		sr << inst->sig_index << inst->target << static_cast<int>(inst->x.size()) << static_cast<int>(inst->y.size());
		for (int i = 0, iend = inst->x.size(); i < iend; ++i) {
			sr << inst->x(i);
		}
		for (int i = 0, iend = inst->y.size(); i < iend; ++i) {
			sr << inst->y(i);
		}
		sr << '\n';
	}
	sr << "CONTINUOUS_DATA_END" << '\n';
	sr << "ALL_RELS_BEGIN" << '\n' << all_rels << '\n' << "ALL_RELS_END" << '\n';
	sr << "CONTEXT_RELS_BEGIN" << '\n' << context_rels << '\n' << "CONTEXT_RELS_END" << '\n';
}

void model_train_data::unserialize(istream &is) {
	string line, label;
	int nsigs, ninsts;

	unserializer unsr(is);
	
	unsr >> label >> nsigs >> ninsts;
	assert(label == "MODEL_TRAIN_DATA");
	
	for (int i = 0; i < nsigs; ++i) {
		scene_sig *s = new scene_sig;
		s->unserialize(is);
		sigs.push_back(s);
	}
	
	unsr >> label;
	assert(label == "CONTINUOUS_DATA_BEGIN");
	for (int i = 0; i < ninsts; ++i) {
		model_train_inst *inst = new model_train_inst;
		int xsz, ysz;
		unsr >> inst->sig_index >> inst->target >> xsz >> ysz;
		inst->sig = sigs[inst->sig_index];
		inst->x.resize(xsz);
		inst->y.resize(ysz);
		for (int i = 0; i < xsz; ++i) {
			unsr >> inst->x(i);
		}
		for (int i = 0; i < ysz; ++i) {
			unsr >> inst->y(i);
		}
		insts.push_back(inst);
	}
	unsr >> label;
	assert(label == "CONTINUOUS_DATA_END");
	
	unsr >> label;
	assert(label == "ALL_RELS_BEGIN");
	unsr >> all_rels >> label;
	assert(label == "ALL_RELS_END");
	unsr >> label;
	assert(label == "CONTEXT_RELS_BEGIN");
	unsr >> context_rels >> label;
	assert(label == "CONTEXT_RELS_END");
}

void model_train_data::proxy_get_children(map<string, cliproxy*> &c) {
	c["rels"] = new memfunc_proxy<model_train_data>(this, &model_train_data::cli_relations);
	c["cont"] = new memfunc_proxy<model_train_data>(this, &model_train_data::cli_contdata);
	c["signatures"] = new memfunc_proxy<model_train_data>(this, &model_train_data::cli_sigs);
	c["save"] = new memfunc_proxy<model_train_data>(this, &model_train_data::cli_save);
}

void model_train_data::cli_relations(const vector<string> &args, ostream &os) const {
	const relation_table *rels;
	int i = 0;
	if (i < args.size() && args[i] == "close") {
		rels = &context_rels;
		++i;
	} else {
		rels = &all_rels;
	}
	
	if (i >= args.size()) {
		os << *rels << endl;
		return;
	}
	const relation *r = map_getp(*rels, args[i]);
	if (!r) {
		os << "no such relation" << endl;
		return;
	}
	if (i + 1 >= args.size()) {
		os << *r << endl;
		return;
	}

	relation matches(*r);

	int_tuple t(1);
	int j, k;
	for (j = i + 1, k = 0; j < args.size() && k < matches.arity(); ++j, ++k) {
		if (args[j] != "*") {
			if (!parse_int(args[j], t[0])) {
				os << "invalid pattern" << endl;
				return;
			}
			matches.filter(k, t, false);
		}
	}

	os << matches << endl;
	return;
}

/*
 Print out the continuous training data for a particular signature (0 by default)
 Suitable for use with matlab.
*/
void model_train_data::cli_contdata(const vector<string> &args, ostream &os) const {
	int sig = 0;
	if (args.size() > 0) {
		if (!parse_int(args[0], sig)) {
			os << "specify a valid signature index" << endl;
			return;
		}
	}

	table_printer t;
	t.set_scientific(true);
	t.set_precision(10);
	for (int i = 0, iend = insts.size(); i < iend; ++i) {
		const model_train_inst &d = *insts[i];
		if (d.sig_index != sig) {
			continue;
		}
		t.add_row();
		for (int j = 0, jend = d.x.size(); j < jend; ++j) {
			t << d.x(j);
		}
		for (int j = 0, jend = d.y.size(); j < jend; ++j) {
			t << d.y(j);
		}
	}
	t.print(os);
}

void model_train_data::cli_sigs(const vector<string> &args, ostream &os) const {
	if (args.size() > 0) {
		int i;
		if (!parse_int(args[0], i)) {
			os << "specify a valid signature index" << endl;
			return;
		}
		sigs[i]->print(os);
	}

	for (int i = 0, iend = sigs.size(); i < iend; ++i) {
		if (i > 0) {
			os << endl << endl;
		}
		sigs[i]->print(os);
	}
}

void model_train_data::cli_save(const vector<string> &args, ostream &os) const {
	if (args.empty()) {
		os << "specify file name" << endl;
		return;
	}
	ofstream f(args[0].c_str());
	if (!f.is_open()) {
		os << "cannot open file " << args[0] << " for writing" << endl;
		return;
	}
	serialize(f);
	f.close();
}
