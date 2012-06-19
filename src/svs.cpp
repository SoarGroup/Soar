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

using namespace std;

typedef map<wme*,command*>::iterator cmd_iter;

void print_tree(sgnode *n) {
	if (n->is_group()) {
		for(int i = 0; i < n->num_children(); ++i) {
			print_tree(n->get_child(i));
		}
	} else {
		const ptlist &pts = n->get_world_points();
		copy(pts.begin(), pts.end(), ostream_iterator<vec3>(cout, ", "));
		cout << endl;
	}
}

sgwme::sgwme(soar_interface *si, Symbol *ident, sgwme *parent, sgnode *node) 
: soarint(si), id(ident), parent(parent), node(node)
{
	int i;
	node->listen(this);
	name_wme = soarint->make_wme(id, "id", node->get_name());
	for (i = 0; i < node->num_children(); ++i) {
		add_child(node->get_child(i));
	}
}

sgwme::~sgwme() {
	map<sgwme*,wme*>::iterator i;

	if (node) {
		node->unlisten(this);
	}
	soarint->remove_wme(name_wme);
	for (i = childs.begin(); i != childs.end(); ++i) {
		i->first->parent = NULL;
		soarint->remove_wme(i->second);
	}
	if (parent) {
		map<sgwme*, wme*>::iterator ci = parent->childs.find(this);
		assert(ci != parent->childs.end());
		soarint->remove_wme(ci->second);
		parent->childs.erase(ci);
	}
}

void sgwme::node_update(sgnode *n, sgnode::change_type t, int added_child) {
	switch (t) {
		case sgnode::CHILD_ADDED:
			add_child(node->get_child(added_child));
			break;
		case sgnode::DELETED:
			node = NULL;
			delete this;
			break;
	};
}

void sgwme::add_child(sgnode *c) {
	sym_wme_pair cid_wme;
	char letter;
	string cname = c->get_name();
	sgwme *child;
	
	if (cname.size() == 0 || !isalpha(cname[0])) {
		letter = 'n';
	} else {
		letter = cname[0];
	}
	cid_wme = soarint->make_id_wme(id, "child");
	
	child = new sgwme(soarint, cid_wme.first, this, c);
	childs[child] = cid_wme.second;
}

svs_state::svs_state(svs *svsp, Symbol *state, soar_interface *si, common_syms *syms)
: svsp(svsp), parent(NULL), state(state), si(si), cs(syms), level(0),
  scene_num(-1), scene_num_wme(NULL), scn(NULL), scene_link(NULL),
  ltm_link(NULL)
{
	assert (si->is_top_state(state));
	outspec = svsp->get_output_spec();
	init();
	timers.add("model");
}

svs_state::svs_state(Symbol *state, svs_state *parent)
: parent(parent), state(state), svsp(parent->svsp), si(parent->si),
  cs(parent->cs), outspec(parent->outspec),
  level(parent->level+1), scene_num(-1),
  scene_num_wme(NULL), scn(NULL), scene_link(NULL), ltm_link(NULL)
{
	assert (si->get_parent_state(state) == parent->state);
	init();
	timers.add("model");
}

svs_state::~svs_state() {
	map<wme*, command*>::iterator i;
	for (i = curr_cmds.begin(); i != curr_cmds.end(); ++i) {
		delete i->second;
	}
	
	delete scn; // results in root being deleted also
	delete mmdl;
}

void svs_state::init() {
	string name;
	
	si->get_name(state, name);
	svs_link = si->make_id_wme(state, cs->svs).first;
	cmd_link = si->make_id_wme(svs_link, cs->cmd).first;
	scene_link = si->make_id_wme(svs_link, cs->scene).first;
	scn = new scene(name, svsp->get_drawer());
	root = new sgwme(si, scene_link, (sgwme*) NULL, scn->get_root());
	if (!parent) {
		ltm_link = si->make_id_wme(svs_link, cs->ltm).first;
	}
	mmdl = new multi_model();
}

void svs_state::update(const string &msg) {
	size_t p = msg.find_first_of('\n');
	int n;
	
	if (sscanf(msg.c_str(), "%d", &n) != 1) {
		perror("svs_state::update");
		cerr << msg << endl;
		exit(1);
	}
	scene_num = n;
	update_scene_num();
	scn->parse_sgel(msg.substr(p+1));
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
			cerr << "could not create command " << attr << endl;
		}
	}
}

void svs_state::clear_scene() {
	scn->clear();
}

void svs_state::update_models() {
	function_timer t(timers.get(MODEL_T));
	vector<string> curr_pnames, out_names;
	output_spec::const_iterator i;
	rvec curr_pvals, out;
	
	if (level > 0) {
		/* No legitimate information to learn from imagined states */
		return;
	}
	
	scn->get_property_names(curr_pnames);
	for (i = outspec->begin(); i != outspec->end(); ++i) {
		curr_pnames.push_back(string("output:") + i->name);
	}
	scn->get_properties(curr_pvals);
	get_output(out);
	
	if (prev_pnames == curr_pnames) {
		rvec x(prev_pvals.size() + out.size());
		if (out.size() > 0) {      // work-around for eigen bug when out.size() == 0
			x << prev_pvals, out;
		} else {
			x = prev_pvals;
		}
		mmdl->test(x, curr_pvals);
		mmdl->learn(x, curr_pvals);
	} else {
		mmdl->set_property_vector(curr_pnames);
		DATAVIS("properties '")
		for (int j = 0; j < curr_pnames.size(); ++j) {
			DATAVIS(curr_pnames[j] << " ")
		}
		DATAVIS("' " << endl)
	}
	prev_pnames = curr_pnames;
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

bool svs_state::cli_inspect(int first_arg, const vector<string> &args, ostream &os) const {
	if (first_arg >= args.size() || args[first_arg] == "help") {
		os << "available queries: atoms models out " << endl;
		return false;
	}
	if (args[first_arg] == "models") {
		return mmdl->cli_inspect(first_arg + 1, args, os);
	} else if (args[first_arg] == "props") {
		vector<string> p;
		rvec v;
		scn->get_property_names(p);
		scn->get_properties(v);
		int w = 0;
		for (int i = 0; i < p.size(); ++i) {
			if (w < p[i].size()) {
				w = p[i].size();
			}
		}
		os << left;
		for (int i = 0; i < p.size(); ++i) {
			os.width(w + 1);
			os << setw(w + 1) << p[i] << setw(1) << v(i) << endl;
		}
		return true;
	} else if (args[first_arg] == "out") {
		if (next_out.size() == 0) {
			os << "no output" << endl;
		} else {
			for (int i = 0; i < next_out.size(); ++i) {
				os << (*outspec)[i].name << " "  << next_out(i) << endl;
			}
		}
		return true;
	} else if (args[first_arg] == "atoms") {
		vector<string> atoms;
		get_filter_table().get_all_atoms(scn, atoms);
		for (int i = 0; i < atoms.size(); ++i) {
			os << atoms[i] << endl;
		}
		return true;
	} else if (args[first_arg] == "timing") {
		timers.report(os);
		return true;
	} else if (args[first_arg] == "command") {
		if (first_arg == args.size() - 1) {
			os << "specify a command id" << endl;
			return false;
		}
		map<wme*, command*>::const_iterator i;
		for (i = curr_cmds.begin(); i != curr_cmds.end(); ++i) {
			string id;
			if (!si->get_name(si->get_wme_val(i->first), id)) {
				assert(false);
			}
			if (id == args[first_arg+1]) {
				i->second->cli_inspect(os);
				return true;
			}
		}
		os << "no such command" << endl;
		return false;
	}
	
	os << "no such query" << endl;
	return false;
}

svs::svs(agent *a)
{
	string env_path = get_option("env");
	if (!env_path.empty()) {
		envsock.accept(env_path, true);
	}
	si = new soar_interface(a);
	make_common_syms();
	timers.add("input");
	timers.add("output");
	timers.add("calc_atoms");
}

svs::~svs() {
	vector<svs_state*>::iterator i;
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		delete *i;
	}
	del_common_syms();
	delete si;
}

void svs::state_creation_callback(Symbol *state) {
	string type, msg;
	svs_state *s;
	
	if (state_stack.empty()) {
		s = new svs_state(this, state, si, &cs);
	} else {
		s = new svs_state(state, state_stack.back());
	}
	
	state_stack.push_back(s);
}

void svs::state_deletion_callback(Symbol *state) {
	svs_state *s;
	s = state_stack.back();
	assert(state == s->get_state());
	state_stack.pop_back();
	delete s;
}

void svs::proc_input(svs_state *s) {
	std::string in;
	if (envsock.connected()) {
		if (!envsock.receive(in)) {
			assert(false);
		}
		split(in, "\n", env_inputs);
	}
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
	function_timer t(timers.get(OUTPUT_T));
	
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
	if (envsock.connected()) {
		envsock.send(ss.str());
	} else {
		env_output = ss.str();
	}
}

void svs::input_callback() {
	function_timer t(timers.get(INPUT_T));
	
	svs_state *topstate = state_stack.front();
	proc_input(topstate);
	topstate->update_models();
	
	vector<svs_state*>::iterator i;
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).update_cmd_results(false);
	}
	
	timers.start(CALC_ATOMS_T);
	topstate->get_scene()->get_atom_vals();
	timers.stop(CALC_ATOMS_T);
}

void svs::make_common_syms() {
	cs.svs        = si->make_sym("svs");
	cs.ltm        = si->make_sym("ltm");
	cs.cmd        = si->make_sym("command");
	cs.scene      = si->make_sym("spatial-scene");
	cs.child      = si->make_sym("child");
	cs.result     = si->make_sym("result");
}

void svs::del_common_syms() {
	si->del_sym(cs.ltm);
	si->del_sym(cs.cmd);
	si->del_sym(cs.scene);
	si->del_sym(cs.child);
	si->del_sym(cs.result);
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

bool svs::do_cli_command(const vector<string> &args, string &output) const {
	stringstream ss;
	if (args.size() < 2) {
		ss << "specify a state level [0 - " << state_stack.size() - 1 << "]";
		output = ss.str();
		return false;
	}
	char *end;
	long level = strtol(args[1].c_str(), &end, 10);
	bool ret;
	if (*end != '\0') {
		if (args[1] == "timing") {
			timers.report(ss);
			output = ss.str();
			return true;
		} else if (args[1] == "filters") {
			get_filter_table().get_timers().report(ss);
			output = ss.str();
			return true;
		} else {
			output = "no such query";
			return false;
		}
	} else if (level < 0 || level >= state_stack.size()) {
		output = "invalid level";
		return false;
	}
	ret = state_stack[level]->cli_inspect(2, args, ss);
	output = ss.str();
	return ret;
}

int svs::parse_output_spec(const string &s) {
	vector<string> fields;
	vector<double> vals(4);
	output_dim_spec sp;
	char *end;
	
	split(s, " \t\n", fields);
	assert(fields[0] == "o");
	if ((fields.size() - 1) % 5 != 0) {
		return fields.size();
	}
	
	output_spec new_spec;
	for (int i = 1; i < fields.size(); i += 5) {
		sp.name = fields[i];
		for (int j = 0; j < 4; ++j) {
			vals[j] = strtod(fields[i + j + 1].c_str(), &end);
			if (*end != '\0') {
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
