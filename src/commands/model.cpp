#include <string>
#include "svs.h"
#include "command.h"
#include "model.h"

using namespace std;

model *_make_null_model_(soar_interface *si, Symbol* root, scene *scn, const string &name);
model *_make_velocity_model_(soar_interface *si, Symbol *root, scene *scn, const string &name);
model *_make_lwr_model_(soar_interface *si, Symbol *root, scene *scn, const string &name);
model *_make_splinter_model_(soar_interface *si, Symbol *root, scene *scn, const string &name);
model *_make_em_model_(soar_interface *si, Symbol *root, scene *scn, const string &name);
model *_make_targets_model_(soar_interface *si, Symbol *root, scene *scn, const string &name);

struct model_constructor_table_entry {
	const char *type;
	model* (*func)(soar_interface*, Symbol*, scene*, const string&);
};

static model_constructor_table_entry constructor_table[] = {
	{ "null",        _make_null_model_},
	{ "velocity",    _make_velocity_model_},
	{ "lwr",         _make_lwr_model_},
	{ "splinter",    _make_splinter_model_},
	{ "em",          _make_em_model_},
	{ "targets",     _make_targets_model_},
};

model *parse_model_struct(soar_interface *si, Symbol *cmd, scene *scn) {
	wme *type_wme, *name_wme;
	string name, type;
	int table_size = sizeof(constructor_table) / sizeof(model_constructor_table_entry);
	
	if (!si->find_child_wme(cmd, "type", type_wme) ||
		!si->get_val(si->get_wme_val(type_wme), type))
	{
		return NULL;
	}
	
	if (!si->find_child_wme(cmd, "name", name_wme) ||
		!si->get_val(si->get_wme_val(name_wme), name))
	{
		return NULL;
	}
	
	for (int i = 0; i < table_size; ++i) {
		if (type == constructor_table[i].type) {
			return constructor_table[i].func(si, cmd, scn, name);
		}
	}
	return NULL;
}

class create_model_command : public command {
public:
	create_model_command(svs_state *state, Symbol *root)
	 : command(state, root), root(root), svsp(state->get_svs()), broken(false)
	{
		si = svsp->get_soar_interface();
	}
	
	string description() {
		return string("create model");
	}
	
	bool update_sub() {
		string name;
		
		if (!changed()) {
			return !broken;
		}
		
		model *m = parse_model_struct(si, root, get_state()->get_scene());
		if (!m) {
			set_status("invalid syntax");
			broken = true;
			return false;
		}
		if (!svsp->add_model(m->get_name(), m)) {
			set_status("nonunique name");
			delete m;
			broken = true;
			return false;
		}
		set_status("success");
		broken = false;
		return true;
	}
	
	bool early() { return true; }
	
	
private:
	svs            *svsp;
	soar_interface *si;
	Symbol         *root;
	bool            broken;
};


class assign_model_command : public command {
public:
	assign_model_command(svs_state *state, Symbol *root)
	: command(state, root), root(root), mmdl(state->get_model()), broken(false), scn(state->get_scene())
	{
		si = state->get_svs()->get_soar_interface();
	}
	
	~assign_model_command() {
		mmdl->unassign_model(name);
	}
	
	string description() {
		return string("activate model");
	}
	
	bool early() {
		return true;
	}
	
	bool update_sub() {
		int i;
		wme *w;
		vector<string> inputs;
		vector<string> outputs;
		vector<string> new_scene_props;
		bool all_inputs = false, all_outputs = false;
		
		scn->get_property_names(new_scene_props);
		if (!changed() && !broken && scene_props == new_scene_props) {
			return true;
		}
		
		if (!si->find_child_wme(root, "name", w) ||
		    !si->get_val(si->get_wme_val(w), name))
		{
			set_status("need model name");
			broken = true;
			return false;
		}
		
		if (si->find_child_wme(root, "inputs", w)) {
			string v;
			if (si->get_val(si->get_wme_val(w), v) && v == "all") {
				all_inputs = true;
			} else {
				si->read_list(si->get_wme_val(w), inputs);
			}
		}
		if (si->find_child_wme(root, "outputs", w)) {
			string v;
			if (si->get_val(si->get_wme_val(w), v) && v == "all") {
				all_outputs = true;
			} else {
				si->read_list(si->get_wme_val(w), outputs);
			}
		}
		
		string error = mmdl->assign_model(name, inputs, all_inputs, outputs, all_outputs);
		if (error != "") {
			set_status(error);
			broken = true;
			return false;
		}
		
		set_status("success");
		broken = false;
		scene_props = new_scene_props;
		return true;
	}

private:
	soar_interface *si;
	Symbol         *root;
	multi_model    *mmdl;
	string          name;
	bool            broken;
	vector<string>  scene_props;
	scene          *scn;
};

command *_make_create_model_command_(svs_state *state, Symbol *root) {
	return new create_model_command(state, root);
}

command *_make_assign_model_command_(svs_state *state, Symbol *root) {
	return new assign_model_command(state, root);
}
