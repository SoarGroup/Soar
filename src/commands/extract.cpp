#include <iostream>
#include "command.h"
#include "filter.h"
#include "svs.h"

using namespace std;

class extract_command : public command {
public:
	extract_command(svs_state *state, Symbol *root)
	: command(state, root), root(root), state(state), fltr(NULL), res(NULL), res_root(NULL), first(true)
	{
		si = state->get_svs()->get_soar_interface();
	}
	
	~extract_command() {
		if (fltr) {
			delete fltr;
		}
	}
	
	string description() {
		return string("extract");
	}
	
	bool update() {
		if (changed()) {
			clear_results();
			if (fltr) {
				delete fltr;
			}
			
			string neg;
			negated = (get_str_param("negate", neg) && (neg == "t" || neg == "true" || neg == "1"));
			fltr = parse_filter_spec(state->get_svs()->get_soar_interface(), root, state->get_scene());
			if (!fltr) {
				set_status("incorrect filter syntax");
				return false;
			}
			res = fltr->get_result();
			first = true;
		}
		
		if (fltr) {
			fltr->update();
				
			if (fltr->is_error()) {
				set_status(fltr->get_error());
				clear_results();
				return false;
			}
			if (first) {
				reset_results();
				first = false;
			} else {
				update_results();
			}
			set_status("success");
		}
		return true;
	}
	
	bool early() { return false; }
	
	void reset_results() {
		filter_result::iter i;
		
		clear_results();
		for (i = res->curr_begin(); i != res->curr_end(); ++i) {
			handle_result(*i);
		}
		res->clear_changes();
	}
	
	void update_results() {
		filter_result::iter i;
		wme *w;
		
		for (i = res->added_begin(); i != res->added_end(); ++i) {
			handle_result(*i);
		}
		for (i = res->removed_begin(); i != res->removed_end(); ++i) {
			if (!map_pop(res2wme, *i, w)) {
				assert(false);
			}
			si->remove_wme(w);
		}
		for (i = res->changed_begin(); i != res->changed_end(); ++i) {
			handle_result(*i);
		}
	}
	
	/*
	 If the result is false and there's a wme for it, remove the
	 wme. If the result is true and there's no wme for it, create
	 a new wme.
	*/
	void handle_result(filter_val *result) {
		filter_param_set *params;
		sym_wme_pair sw;
		filter_param_set::const_iterator i;
		bool val;
		wme *w;
		
		if (!get_filter_val(result, val)) {
			set_status("extract filter must have boolean results");
			return;
		}
		/*
		 list the parameters if result is (true and not negated) or (false and negated)
		*/
		if (val == negated) {
			if (map_pop(res2wme, result, w)) {
				si->remove_wme(w);
			}
			return;
		}
		if (!fltr->get_result_params(result, params)) {
			assert(false);
		}
		
		if (map_get(res2wme, result, w)) {
			return;
		}
		
		if (res_root == NULL) {
			sym_wme_pair p;
			p = si->make_id_wme(root, "result");
			res_root = p.first;
		}
		sw = si->make_id_wme(res_root, "literal");
		for (i = params->begin(); i != params->end(); ++i) {
			si->make_wme(sw.first, i->first, i->second->get_string());
		}
		res2wme[result] = sw.second;
	}
	
	void clear_results() {
		std::map<filter_val*, wme*>::iterator i;
		for (i = res2wme.begin(); i != res2wme.end(); ++i) {
			si->remove_wme(i->second);
		}
		res2wme.clear();
	}
	
private:
	Symbol         *root;
	Symbol         *res_root;
	svs_state      *state;
	soar_interface *si;
	filter         *fltr;
	filter_result  *res;
	bool            first;
	bool            negated;
	
	std::map<filter_val*, wme*> res2wme;
};

command *_make_extract_command_(svs_state *state, Symbol *root) {
	return new extract_command(state, root);
}
