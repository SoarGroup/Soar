#include <iostream>
#include <string>
#include "command.h"
#include "filter.h"
#include "svs.h"

using namespace std;

class property_command : public command {
public:
	property_command(svs_state *state, Symbol *root)
	: command(state, root), root(root), fltr(NULL), res(NULL), added(false)
	{
		si = state->get_svs()->get_soar_interface();
		scn = state->get_scene();
	}
	
	~property_command() {
		if (fltr) {
			delete fltr;
		}
		if (added) {
			scn->remove_property(obj, prop);
		}
	}
	
	string description() {
		return string("property");
	}
	
	bool update() {
		float val;
		if (changed()) {
			if (fltr) {
				delete fltr;
			}
			if (!parse()) {
				return false;
			}
			res = fltr->get_result();
		}
		
		if (fltr) {
			fltr->update();
			if (fltr->is_error()) {
				set_status(fltr->get_error());
				return false;
			}
			if (res->size() == 0) {
				set_status("no results");
				return false;
			}
			if (!get_filter_val(*res->curr_begin(), val)) {
				set_status("result not of type float");
				return false;
			}
			if (!added) {
				if (!scn->add_property(obj, prop, val)) {
					set_status("failed to add property");
					return false;
				}
				added = true;
			} else {
				if (!scn->set_property(obj, prop, val)) {
					set_status("failed to set property");
					return false;
				}
			}
			set_status("success");
		}
		return true;
	}
	
	bool early() { return false; }
	
	bool parse() {
		wme *objwme, *propwme, *filterwme;
		
		if (!si->find_child_wme(root, "object", objwme)) {
			set_status("no object specified");
			return false;
		}
		if (!si->get_val(si->get_wme_val(objwme), obj)) {
			set_status("object name must be a string");
			return false;
		}
		
		if (!si->find_child_wme(root, "property", propwme)) {
			set_status("no property specified");
			return false;
		}
		if (!si->get_val(si->get_wme_val(propwme), prop)) {
			set_status("property name must be a string");
			return false;
		}
		
		if (!si->find_child_wme(root, "filter", filterwme)) {
			set_status("no filter specified");
			return false;
		}
		fltr = parse_filter_spec(si, si->get_wme_val(filterwme), scn);
		if (!fltr) {
			set_status("incorrect filter syntax");
			return false;
		}
		return true;
	}
	
private:
	Symbol         *root;
	scene          *scn;
	soar_interface *si;
	filter         *fltr;
	filter_result  *res;
	string          obj;
	string          prop;
	bool            added;
};

command *_make_property_command_(svs_state *state, Symbol *root) {
	return new property_command(state, root);
}
