#include <string>
#include "command.h"
#include "svs.h"

using namespace std;

const int DEFAULT_INCREMENTS = 10;

bool output_comp(const output_dim_spec &a, const output_dim_spec &b) {
	return a.name < b.name;
}

/*
 Parses a WME structure that describes the output the environment expects.
 Assumes this format:

 ^output (
 	^<name1> (
 		^min <val>
 		^max <val>
 		^def <val>
 	)
 	^<name2> (
 		^min <val>
 		^max <val>
 		^def <val>
 	)
		...
 )
*/
class output_command : public command {
public:
	output_command(svs_state *state, Symbol *root)
	: command(state, root), state(state), root(root)
	{
		si = state->get_svs()->get_soar_interface();
	}
	
	bool update_drv() {
		if (!changed()) {
			return true;
		}
		
		wme_list dim_wmes;
		wme_list::iterator i;
		Symbol *dim_id;
		wme *min_wme, *max_wme, *def_wme, *incr_wme;
		output_dim_spec d;
		output_spec outspec;
		
		if (!si->is_identifier(root)) {
			return false;
		}
		si->get_child_wmes(root, dim_wmes);
		for (i = dim_wmes.begin(); i != dim_wmes.end(); ++i) {
			dim_id = si->get_wme_val(*i);
			if (!si->get_val(si->get_wme_attr(*i), d.name)        ||
			    !si->is_identifier(dim_id)                        ||
			    !si->find_child_wme(dim_id, "min", min_wme)       ||
			    !si->get_val(si->get_wme_val(min_wme), d.min)     ||
			    !si->find_child_wme(dim_id, "max", max_wme)       ||
			    !si->get_val(si->get_wme_val(max_wme), d.max)     ||
			    !si->find_child_wme(dim_id, "default", def_wme)   ||
			    !si->get_val(si->get_wme_val(def_wme), d.def))
			{
				continue;
			}
			if (!si->find_child_wme(dim_id, "increment", incr_wme) ||
			    !si->get_val(si->get_wme_val(incr_wme), d.incr))
			{
				d.incr = (d.max - d.min) / DEFAULT_INCREMENTS;
			}
			outspec.push_back(d);
		}
		sort(outspec.begin(), outspec.end(), output_comp);
		state->set_output_spec(outspec);
		set_status("success");
		return true;
	}
	
	bool early() {
		return true;
	}
	
	string description() {
		return "output";
	}
	
private:
	soar_interface *si;
	svs_state      *state;
	Symbol         *root;
};

command *_make_output_command_(svs_state *state, Symbol *root) {
	return new output_command(state, root);
}

