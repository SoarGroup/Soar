#include <iostream>
#include <string>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"

using namespace std;

class save_snapshot_command : public command {
public:
	save_snapshot_command(svs_state *state, Symbol *root)
	: command(state, root), root(root), target(NULL), first(true)
	{
		si = state->get_svs()->get_soar_interface();
		scn = state->get_scene();
	}

	string description() {
		return string("save-snapshot");
	}

	bool update_sub(){
		if(first){
			first = false;
			return parse();
		}
		return true;
	}
	

	bool early() { return false; }

	bool parse() {
		// Get the ^id wme
		wme *idwme;
		if(!si->find_child_wme(root, "id", idwme)){
			set_status("no object id specified");
			return false;
		}

		// Get the value of the ^id wme
		string id;
		if(!si->get_val(si->get_wme_val(idwme), id)){
			set_status("object id must be a string");
			return false;
		}

		// Find the node with the given name
		target = scn->get_node(id);
		if(!target){
			set_status("Could not find the given node");
			return false;
		}

		target->save_snapshot();
		set_status("success");
		return true;
	}

private:
	Symbol         *root;
	scene          *scn;
	soar_interface *si;
	sgnode* target;
	bool first;
};

command *_make_save_snapshot_command_(svs_state *state, Symbol *root) {
	return new save_snapshot_command(state, root);
}

