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
		ptlist pts;
		n->get_world_points(pts);
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
	init();
}

svs_state::svs_state(Symbol *state, svs_state *parent)
: parent(parent), state(state), svsp(parent->svsp), si(parent->si), cs(parent->cs),
  level(parent->level+1), scene_num(-1),
  scene_num_wme(NULL), scn(NULL), scene_link(NULL), ltm_link(NULL)
{
	assert (si->get_parent_state(state) == parent->state);
	init();
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
	scn = new scene(name, "world", true);
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
	vector<string> curr_pnames, out_names;
	output_spec::const_iterator i;
	floatvec curr_pvals, out;
	double dt;
	
	if (level > 0) {
		/* No legitimate information to learn from in imagined states */
		return;
	}
	
	scn->get_property_names(curr_pnames);
	for (i = outspec.begin(); i != outspec.end(); ++i) {
		curr_pnames.push_back(string("output:") + i->name);
	}
	scn->get_properties(curr_pvals);
	get_output(out);
	dt = scn->get_dt();
	
	if (prev_pnames == curr_pnames) {
		floatvec x(prev_pvals);
		x.extend(out);
		mmdl->test(x, curr_pvals);
		mmdl->learn(x, curr_pvals, dt);
	} else {
		mmdl->set_property_vector(curr_pnames);
	}
	prev_pnames = curr_pnames;
	prev_pvals = curr_pvals;
}

void svs_state::set_output(const floatvec &out) {
	assert(out.size() == outspec.size());
	next_out = out;
}

void svs_state::set_default_output() {
	next_out.resize(outspec.size());
	for (int i = 0; i < outspec.size(); ++i) {
		next_out[i] = outspec[i].def;
	}
}

bool svs_state::get_output(floatvec &out) const {
	if (next_out.size() != outspec.size()) {
		out.resize(outspec.size());
		for (int i = 0; i < outspec.size(); ++i) {
			out[i] = outspec[i].def;
		}
		return false;
	} else {
		out = next_out;
		return true;
	}
}

svs::svs(agent *a)
{
	if (false) {
		envsock.reset(new ipcsocket('s', getnamespace() + "env", true, true));
	}
	si = new soar_interface(a);
	make_common_syms();
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
	if (envsock.get()) {
		if (!envsock->receive(env_input)) {
			assert(false);
		}
	}
	s->get_scene()->parse_sgel(env_input);
	env_input.clear();
}

void svs::output_callback() {
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
	output_spec *outspec = topstate->get_output_spec();
	floatvec out;
	topstate->get_output(out);
	
	assert(outspec->size() == out.size());
	
	stringstream ss;
	for (int i = 0; i < outspec->size(); ++i) {
		ss << (*outspec)[i].name << " " << out[i] << endl;
	}
	if (envsock.get()) {
		envsock->send(ss.str());
	} else {
		env_output = ss.str();
	}
	
}

void svs::input_callback() {
	svs_state *topstate = state_stack.front();
	proc_input(topstate);
	topstate->update_models();
	
	vector<svs_state*>::iterator i;
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).update_cmd_results(false);
	}
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
