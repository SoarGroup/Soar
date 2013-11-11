#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iterator>
#include <utility>
#include <algorithm>
#include <fstream>

#include "svs.h"
#include "command.h"
#include "sgnode.h"
#include "soar_interface.h"
#include "scene.h"
#include "common.h"
#include "model.h"
#include "filter_table.h"
#include "drawer.h"
#include "logger.h"
#include "model.h"

using namespace std;

typedef map<wme*,command*>::iterator cmd_iter;

svs_interface *make_svs(agent *a) {
	return new svs(a);
}


sgwme::sgwme(soar_interface *si, Symbol *ident, sgwme *parent, sgnode *node)
: soarint(si), id(ident), parent(parent), node(node)
{
	node->listen(this);
	name_wme = soarint->make_wme(id, si->get_common_syms().id, node->get_name());

	if (node->is_group()) {
		group_node *g = node->as_group();
		for (int i = 0; i < g->num_children(); ++i) {
			add_child(g->get_child(i));
		}
	}

	// Create wmes for all string properties
	const string_properties_map& str_props = node->get_string_properties();
	for(string_properties_map::const_iterator i = str_props.begin(); i != str_props.end(); i++){
		set_property(i->first, i->second);
	}

	// Create wmes for all numeric properties
	const numeric_properties_map& num_props = node->get_numeric_properties();
	for(numeric_properties_map::const_iterator i = num_props.begin(); i != num_props.end(); i++){
		set_property(i->first, i->second);
	}
}

sgwme::~sgwme() {
	map<sgwme*,wme*>::iterator i;

	if (node) {
		node->unlisten(this);
	}
	soarint->remove_wme(name_wme);

	for(std::map<std::string, wme*>::iterator i = properties.begin(); i != properties.end(); i++){
		soarint->remove_wme(i->second);
	}

	for (i = childs.begin(); i != childs.end(); ++i) {
		i->first->parent = NULL;
		delete i->first;
		soarint->remove_wme(i->second);
	}
	if (parent) {
		map<sgwme*, wme*>::iterator ci = parent->childs.find(this);
		assert(ci != parent->childs.end());
		soarint->remove_wme(ci->second);
		parent->childs.erase(ci);
	}
}

void sgwme::node_update(sgnode *n, sgnode::change_type t, const std::string& update_info) {
	int added_child = 0;
	group_node *g;
	switch (t) {
		case sgnode::CHILD_ADDED:
			if(parse_int(update_info, added_child)){
				g = node->as_group();
				add_child(g->get_child(added_child));
			}
			break;
		case sgnode::DELETED:
			node = NULL;
			delete this;
			break;
		case sgnode::PROPERTY_CHANGED:
			update_property(update_info);
			break;
		case sgnode::PROPERTY_DELETED:
			delete_property(update_info);
			break;
		default:
			break;
	};
}

void sgwme::add_child(sgnode *c) {
	char letter;
	string cname = c->get_name();
	sgwme *child;

	if (cname.size() == 0 || !isalpha(cname[0])) {
		letter = 'n';
	} else {
		letter = cname[0];
	}
	wme *cid_wme = soarint->make_id_wme(id, "child");

	child = new sgwme(soarint, soarint->get_wme_val(cid_wme), this, c);
	childs[child] = cid_wme;
}

template <class WmeType>
void sgwme::set_property(const std::string& propertyName, const WmeType& value){
  Symbol* rootID = id;
	std::string att = propertyName;
	std::string parentAtt = "";
	size_t periodPos = propertyName.find_first_of('.');
	if(periodPos != std::string::npos){
		// This is a two level property
		parentAtt = propertyName.substr(0, periodPos);
		att = propertyName.substr(periodPos+1);
		wme* parentWME;

		// First, we get the parent WME
		std::map<std::string, wme*>::iterator i = properties.find(parentAtt);

		if(i == properties.end()){
			// First time seeing this parent WME, make a new one
			parentWME = soarint->make_id_wme(id, parentAtt);
			properties[parentAtt] = parentWME;
		} else {
		  // The parent WME already exists
			parentWME = i->second;
			if(!soarint->is_identifier(soarint->get_wme_val(parentWME))){
				// Something weird, the parent WME exists but not as an identifier
				soarint->remove_wme(parentWME);
				parentWME = soarint->make_id_wme(id, parentAtt);
				properties[parentAtt] = parentWME;
			}
		}

		rootID = soarint->get_wme_val(parentWME);
	}

  // Remove the old wme and add the new one
	std::map<std::string, wme*>::iterator i = properties.find(propertyName);
	if(i != properties.end()){
		soarint->remove_wme(i->second);
  }
	properties[propertyName] = soarint->make_wme(rootID, att, value);
}

void sgwme::update_property(const std::string& propertyName){
	wme* propWme;
	const string_properties_map& str_props = node->get_string_properties();
	const numeric_properties_map& num_props = node->get_numeric_properties();

	string_properties_map::const_iterator str_it = str_props.find(propertyName);
	numeric_properties_map::const_iterator num_it = num_props.find(propertyName);

	if(str_it != str_props.end()){
		// Make a wme with a string value
		set_property(propertyName, str_it->second);
	} else if(num_it != num_props.end()){
		// Make a wme with a numeric value
		set_property(propertyName, num_it->second);
	} else {
		// Something went wrong, the property is not on the node
		return;
	}
}

void sgwme::delete_property(const std::string& propertyName){
	for(std::map<std::string, wme*>::iterator i = properties.begin(); i != properties.end(); i++){
		if (i->first.find(propertyName) == 0){
			soarint->remove_wme(i->second);
			properties.erase(i);
		}
	}
}


svs_state::svs_state(svs *svsp, Symbol *state, soar_interface *si, scene *scn)
: svsp(svsp), parent(NULL), state(state), si(si), level(0),
  scene_num(-1), scene_num_wme(NULL), scn(scn), scene_link(NULL)
{
	assert (si->is_top_state(state));
	si->get_name(state, name);
	outspec = svsp->get_output_spec();
	loggers = svsp->get_loggers();
	init();
}

svs_state::svs_state(Symbol *state, svs_state *parent)
: parent(parent), state(state), svsp(parent->svsp), si(parent->si),
  outspec(parent->outspec), level(parent->level+1), scene_num(-1),
  scene_num_wme(NULL), scn(NULL), scene_link(NULL)
{
	assert (si->get_parent_state(state) == parent->state);
	loggers = svsp->get_loggers();
	init();
}

svs_state::~svs_state() {
	map<wme*, command*>::iterator i, iend;

	for (i = curr_cmds.begin(), iend = curr_cmds.end(); i != iend; ++i) {
		delete i->second;
	}

	delete mmdl;

	if (scn) {
		svsp->get_drawer()->delete_scene(scn->get_name());
		delete scn; // results in root being deleted also
	}
}

void svs_state::init() {
	string name;
	common_syms &cs = si->get_common_syms();

	si->get_name(state, name);
	svs_link = si->get_wme_val(si->make_id_wme(state, cs.svs));
	cmd_link = si->get_wme_val(si->make_id_wme(svs_link, cs.cmd));
	scene_link = si->get_wme_val(si->make_id_wme(svs_link, cs.scene));
	if (!scn) {
		if (parent) {
			scn = parent->scn->clone(name);
		} else {
			// top state
			scn = new scene(name, svsp);
			scn->set_draw(true);
		}
	}
	scn->refresh_draw();
	root = new sgwme(si, scene_link, (sgwme*) NULL, scn->get_root());
	mmdl = new multi_model(svsp->get_models());
	learn_models = false;
	test_models = false;
}

void svs_state::update_scene_num() {
	long curr;
	if (scene_num_wme) {
		if (!si->get_val(si->get_wme_val(scene_num_wme), curr)) {
			exit(1);
		}
		if (curr == scene_num) {
			return;
		}
		si->remove_wme(scene_num_wme);
	}
	if (scene_num >= 0) {
		scene_num_wme = si->make_wme(svs_link, "scene-num", scene_num);
	}
}

void svs_state::update_cmd_results(bool early) {
	cmd_iter i;
	if (early) {
		set_default_output();
	}
	for (i = curr_cmds.begin(); i != curr_cmds.end(); ++i) {
		if (i->second->early() == early) {
			i->second->update();
		}
	}
}

void svs_state::process_cmds() {
	wme_list all;
	wme_list::iterator i;
	cmd_iter j;

	si->get_child_wmes(cmd_link, all);

	for (j = curr_cmds.begin(); j != curr_cmds.end(); ) {
		if ((i = find(all.begin(), all.end(), j->first)) == all.end()) {
			delete j->second;
			curr_cmds.erase(j++);
		} else {
			all.erase(i);
			++j;
		}
	}

	// all now contains only new commands
	for (i = all.begin(); i != all.end(); ++i) {
		command *c = make_command(this, *i);
		if (c) {
			curr_cmds[*i] = c;
		} else {
			string attr;
			si->get_val(si->get_wme_attr(*i), attr);
			loggers->get(LOG_ERR) << "could not create command " << attr << endl;
		}
	}
}

void svs_state::clear_scene() {
	scn->clear();
}

void svs_state::update_models() {
	function_timer t(timers.get_or_add("model"));
	scene_sig curr_sig, out_names;
	output_spec::const_iterator i;
	rvec curr_pvals, out;
	relation_table curr_rels;

	if (level > 0) {
		/* No legitimate information to learn from imagined states */
		return;
	}

	scn->get_properties(curr_pvals);
	get_output(out);
	curr_sig = scn->get_signature();

	timer &t1 = timers.get_or_add("up_rels");
	t1.start();
	scn->get_relations(curr_rels);
	t1.stop();

	// add an entry to the signature for the output
	scene_sig::entry out_entry;
	out_entry.id = -2;
	out_entry.name = "output";
	out_entry.type = "output";
	for (int i = 0; i < outspec->size(); ++i) {
		out_entry.props.push_back(outspec->at(i).name);
	}
	curr_sig.add(out_entry);

	if (prev_sig == curr_sig) {
		rvec x(prev_pvals.size() + out.size());
		if (out.size() > 0) {      // work-around for eigen bug when out.size() == 0
			x << prev_pvals, out;
		} else {
			x = prev_pvals;
		}
		if (test_models) {
			mmdl->test(curr_sig, prev_rels, x, curr_pvals);
		}
		if (learn_models) {
			mmdl->learn(curr_sig, prev_rels, x, curr_pvals);
		}
	}
	prev_sig = curr_sig;
	prev_rels = curr_rels;
	prev_pvals = curr_pvals;
}

void svs_state::set_output(const rvec &out) {
	assert(out.size() == outspec->size());
	next_out = out;
}

void svs_state::set_default_output() {
	next_out.resize(outspec->size());
	for (int i = 0; i < outspec->size(); ++i) {
		next_out[i] = (*outspec)[i].def;
	}
}

bool svs_state::get_output(rvec &out) const {
	if (next_out.size() != outspec->size()) {
		out.resize(outspec->size());
		for (int i = 0; i < outspec->size(); ++i) {
			out[i] = (*outspec)[i].def;
		}
		return false;
	} else {
		out = next_out;
		return true;
	}
}

void svs_state::proxy_get_children(map<string, cliproxy*> &c) {
	c["learn_models"] = new bool_proxy(&learn_models, "Learn models in this state.");
	c["test_models"]  = new bool_proxy(&test_models, "Test models in this state.");
	c["timers"]       = &timers;
	c["mconfig"]      = mmdl;
	c["scene"]        = scn;
	c["output"]       = new memfunc_proxy<svs_state>(this, &svs_state::cli_out);
	c["output"]->set_help("Print current output.");

	proxy_group *cmds = new proxy_group;
	map<wme*, command*>::const_iterator i;
	for (i = curr_cmds.begin(); i != curr_cmds.end(); ++i) {
		string id;
		if (!si->get_name(si->get_wme_val(i->first), id)) {
			assert(false);
		}
		cmds->add(id, i->second);
	}

	c["command"] = cmds;
}

// add ability to set it?
void svs_state::cli_out(const vector<string> &args, ostream &os) {
	if (next_out.size() == 0) {
		os << "no output" << endl;
	} else {
		table_printer t;
		for (int i = 0; i < next_out.size(); ++i) {
			t.add_row() << (*outspec)[i].name << next_out(i);
		}
		t.print(os);
	}
}

void svs_state::disown_scene() {
	delete root;
	scn = NULL;
}

svs::svs(agent *a)
: use_models(false), record_movie(false), scn_cache(NULL)
{
	si = new soar_interface(a);
	draw = new drawer();
	loggers = new logger_set(si);
}

svs::~svs() {
	for (int i = 0, iend = state_stack.size(); i < iend; ++i) {
		delete state_stack[i];
	}
	if (scn_cache) {
		delete scn_cache;
	}

	delete si;
	map<string, model*>::iterator j;
	for (j = models.begin(); j != models.end(); ++j) {
		delete j->second;
	}
	delete draw;
}

void svs::state_creation_callback(Symbol *state) {
	string type, msg;
	svs_state *s;

	if (state_stack.empty()) {
		if (scn_cache) {
			scn_cache->verify_listeners();
		}
		s = new svs_state(this, state, si, scn_cache);
		scn_cache = NULL;
	} else {
		s = new svs_state(state, state_stack.back());
	}

	state_stack.push_back(s);
}

void svs::state_deletion_callback(Symbol *state) {
	svs_state *s;
	s = state_stack.back();
	assert(state == s->get_state());
	if (state_stack.size() == 1) {
		// removing top state, save scene for reinit
		scn_cache = s->get_scene();
		s->disown_scene();
	}
	delete s;
	state_stack.pop_back();
}

void svs::proc_input(svs_state *s) {
	for (int i = 0; i < env_inputs.size(); ++i) {
		strip(env_inputs[i], " \t");
		if (env_inputs[i][0] == 'o') {
			int err = parse_output_spec(env_inputs[i]);
			if (err >= 0) {
				cerr << "error in output description at field " << err << endl;
			}
		} else {
			s->get_scene()->parse_sgel(env_inputs[i]);
		}
	}
	env_inputs.clear();
}

void svs::output_callback() {
	function_timer t(timers.get_or_add("output"));

	vector<svs_state*>::iterator i;
	string sgel;
	svs_state *topstate = state_stack.front();

	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).process_cmds();
	}
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).update_cmd_results(true);
	}

	/* environment IO */
	rvec out;
	topstate->get_output(out);

	assert(outspec.size() == out.size());

	stringstream ss;
	for (int i = 0; i < outspec.size(); ++i) {
		ss << outspec[i].name << " " << out[i] << endl;
	}
	env_output = ss.str();
}

void svs::input_callback() {
	function_timer t(timers.get_or_add("input"));

	svs_state *topstate = state_stack.front();
	proc_input(topstate);
	if (use_models) {
		topstate->update_models();
	}

	vector<svs_state*>::iterator i;
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).update_cmd_results(false);
	}

	if (record_movie) {
		static int frame = 0;
		stringstream ss;
		ss << "save screen" << setfill('0') << setw(4) << frame++ << ".ppm";
		draw->send(ss.str());
	}
}

/*
 This is a naive implementation. If this method is called concurrently
 with proc_input, the env_inputs vector will probably become
 inconsistent. This eventually needs to be replaced by a thread-safe FIFO.
*/
void svs::add_input(const string &in) {
	split(in, "\n", env_inputs);
}

string svs::get_output() const {
	return env_output;
}

string svs::svs_query(const string &query){
    return "";
}

void svs::proxy_get_children(map<string, cliproxy*> &c) {
	c["record_movie"]      = new bool_proxy(&record_movie, "Automatically take screenshots in viewer after every decision cycle.");
	c["connect_viewer"]    = new memfunc_proxy<svs>(this, &svs::cli_connect_viewer);
	c["connect_viewer"]->set_help("Connect to a running viewer.")
	                     .add_arg("PORT", "TCP port (or file socket path in Linux) to connect to.")
	                     ;

	c["disconnect_viewer"] = new memfunc_proxy<svs>(this, &svs::cli_disconnect_viewer);
	c["disconnect_viewer"]->set_help("Disconnect from viewer.");

	c["use_models"]        = new memfunc_proxy<svs>(this, &svs::cli_use_models);
	c["use_models"]->set_help("Use model learning system.")
	                 .add_arg("[VALUE]", "New value. Must be (0|1|on|off|true|false).");

	c["add_model"]         = new memfunc_proxy<svs>(this, &svs::cli_add_model);
	c["add_model"]->set_help("Add a model.")
	                 .add_arg("NAME", "Name of the model.")
	                 .add_arg("TYPE", "Type of the model.")
	                 .add_arg("[PATH]", "Path of file to load model from.");

	c["timers"]            = &timers;
	c["loggers"]           = loggers;
	c["filters"]           = &get_filter_table();

	proxy_group *model_group = new proxy_group;
	map<string, model*>::iterator i, iend;
	for (i = models.begin(), iend = models.end(); i != iend; ++i) {
		model_group->add(i->first, i->second);
	}
	c["model"] = model_group;

	for (int j = 0, jend = state_stack.size(); j < jend; ++j) {
		c[state_stack[j]->get_name()] = state_stack[j];
	}
}

bool svs::do_cli_command(const vector<string> &args, string &output) {
	stringstream ss;
	vector<string> rest;

	if (args.size() < 2) {
		output = "specify path\n";
		return false;
	}

	for (int i = 2, iend = args.size(); i < iend; ++i) {
		rest.push_back(args[i]);
	}

	proxy_use(args[1], rest, ss);
	output = ss.str();
	return true;
}

void svs::cli_connect_viewer(const vector<string> &args, ostream &os) {
	if (args.empty()) {
		os << "specify socket path" << endl;
		return;
	}
	if (draw->connect(args[0])) {
		os << "connection successful" << endl;
		for (int i = 0, iend = state_stack.size(); i < iend; ++i) {
			state_stack[i]->get_scene()->refresh_draw();
		}
	} else {
		os << "connection failed" << endl;
	}
}

void svs::cli_disconnect_viewer(const vector<string> &args, ostream &os) {
	draw->disconnect();
}

int svs::parse_output_spec(const string &s) {
	vector<string> fields;
	vector<double> vals(4);
	output_dim_spec sp;
	char *end;

	split(s, "", fields);
	assert(fields[0] == "o");
	if ((fields.size() - 1) % 5 != 0) {
		return fields.size();
	}

	output_spec new_spec;
	for (int i = 1; i < fields.size(); i += 5) {
		sp.name = fields[i];
		for (int j = 0; j < 4; ++j) {
			if (!parse_double(fields[i + j + 1], vals[j])) {
				return i + j + 1;
			}
		}
		sp.min = vals[0];
		sp.max = vals[1];
		sp.def = vals[2];
		sp.incr = vals[3];
		new_spec.push_back(sp);
	}
	outspec = new_spec;
	return -1;
}

bool svs::add_model(const string &name, model *m) {
	if (models.find(name) != models.end()) {
		return false;
	}
	models[name] = m;
	return true;
}

void svs::cli_use_models(const vector<string> &args, ostream &os) {
	bool_proxy p(&use_models, "Use model learning system.");
	p.proxy_use("", args, os);
	state_stack[0]->get_scene()->set_track_distances(use_models);
}

void svs::cli_add_model(const vector<string> &args, ostream &os) {
	if (args.size() < 2) {
		os << "Specify name and type." << endl;
		return;
	}
	model *m = make_model(this, args[0], args[1]);
	if (!m) {
		os << "Cannot create model. Probably no such type." << endl;
		return;
	}
	if (args.size() >= 3) {
		ifstream input(args[2].c_str());
		if (!input) {
			os << "File could not be read. Model not loaded." << endl;
			delete m;
			return;
		}
		m->unserialize(input);
		input.close();
	}
	add_model(args[0], m);
}
