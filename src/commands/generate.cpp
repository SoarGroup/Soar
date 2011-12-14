#include <string>
#include "command.h"
#include "scene.h"
#include "filter.h"
#include "svs.h"

using namespace std;

class generate_command : public command {
public:
	generate_command(svs_state *state, Symbol *root)
	: command(state, root), root(root), scn(state->get_scene()), gen_filter(NULL)
	{
		si = state->get_svs()->get_soar_interface();
	}
	
	~generate_command() {
		reset();
	}
	
	string description() {
		return string("generate");
	}
	
	bool update() {
		wme *parent_wme, *gen_wme;
		sgnode *gen_node;
		
		if (changed()) {
			reset();
			if (!si->find_child_wme(root, "parent", parent_wme) ||
			    !si->find_child_wme(root, "node", gen_wme))
			{
				set_status("missing parameters");
				return false;
			}
			if (!si->get_val(si->get_wme_val(parent_wme), parent)) {
				set_status("parent name must be a string");
				return false;
			}
			if ((gen_filter = parse_filter_spec(si, si->get_wme_val(gen_wme), scn)) == NULL) {
				set_status("incorrect gen filter syntax");
				return false;
			}
		}
		if (gen_filter) {
			if (!proc_changes()) {
				return false;
			}
			set_status("success");
			return true;
		}
		return false;
	}
	
	bool early() { return false; }

private:
	bool proc_changes() {
		filter_result::iter i;
		filter_result *res;
		
		if (!gen_filter->update()) {
			set_status(gen_filter->get_error());
			return false;
		}
		res = gen_filter->get_result();
		for (i = res->added_begin(); i != res->added_end(); ++i) {
			if (!add_node(*i)) {
				return false;
			}
		}
		for (i = res->removed_begin(); i != res->removed_end(); ++i) {
			if (!del_node(*i)) {
				return false;
			}
		}
		res->clear_changes();
		return true;
	}
	
	bool add_node(filter_val *v) {
		sgnode *n;
		stringstream ss;
		if (!get_filter_val(v, n)) {
			set_status("filter result must be a node");
			return false;
		}
		if (!scn->add_node(parent, n)) {
			ss << "error adding node " << n->get_name() << " to scene";
			set_status(ss.str());
			return false;
		}
		nodes.push_back(n->get_name());
		return true;
	}
	
	bool del_node(filter_val *v) {
		sgnode *n;
		stringstream ss;
		if (!get_filter_val(v, n)) {
			set_status("filter result must be a node");
			return false;
		}
		if (!scn->del_node(n->get_name())) {
			ss << "error deleting node " << n->get_name() << " from scene";
			set_status(ss.str());
			return false;
		}
		nodes.erase(find(nodes.begin(), nodes.end(), n->get_name()));
		return true;
	}
	
	void reset() {
		std::list<string>::iterator i;
		if (gen_filter) {
			delete gen_filter;
		}
		
		for (i = nodes.begin(); i != nodes.end(); ++i) {
			scn->del_node(*i);
		}
	}
	
	scene             *scn;
	Symbol            *root;
	soar_interface    *si;
	string             parent;
	
	filter            *gen_filter;
	std::list<string>  nodes;
};

command *_make_generate_command_(svs_state *state, Symbol *root) {
	return new generate_command(state, root);
}
