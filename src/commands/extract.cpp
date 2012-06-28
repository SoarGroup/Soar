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
	
	bool update_sub() {
		if (changed()) {
			clear_results();
			if (fltr) {
				delete fltr;
			}
			
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
			res->clear_changes();
			set_status("success");
		}
		return true;
	}
	
	bool early() { return false; }
	
	void reset_results() {
		clear_results();
		for (int i = 0; i < res->num_current(); ++i) {
			handle_result(res->get_current(i));
		}
		res->clear_changes();
	}
	
	void update_results() {
		wme *w;
		
		for (int i = res->first_added(); i < res->num_current(); ++i) {
			handle_result(res->get_current(i));
		}
		for (int i = 0; i < res->num_removed(); ++i) {
			if (!map_pop(res2wme, res->get_removed(i), w)) {
				assert(false);
			}
			si->remove_wme(w);
		}
		for (int i = 0; i < res->num_changed(); ++i) {
			handle_result(res->get_changed(i));
		}
	}
	
	/*
	 If the result is false and there's a wme for it, remove the
	 wme. If the result is true and there's no wme for it, create
	 a new wme.
	*/
	void handle_result(filter_val *result) {
		const filter_param_set *params;
		sym_wme_pair sw;
		filter_param_set::const_iterator i;
		bool val;
		wme *w;
		
		if (!get_filter_val(result, val)) {
			set_status("extract filter must have boolean results");
			return;
		}

		if (map_pop(res2wme, result, w)) {
			si->remove_wme(w);
		}

		if (!fltr->get_result_params(result, params)) {
			assert(false);
		}
		
		if (res_root == NULL) {
			sym_wme_pair p;
			p = si->make_id_wme(root, "result");
			res_root = p.first;
			pos_root = si->make_id_wme(res_root, "positive").first;
			neg_root = si->make_id_wme(res_root, "negative").first;
		}
		Symbol *r = val ? pos_root : neg_root;

		sw = si->make_id_wme(r, "atom");
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
	Symbol         *pos_root;  // identifier for positive atoms
	Symbol         *neg_root;  // identifier for negative atoms
	svs_state      *state;
	soar_interface *si;
	filter         *fltr;
	filter_result  *res;
	bool            first;
	
	std::map<filter_val*, wme*> res2wme;
};

command *_make_extract_command_(svs_state *state, Symbol *root) {
	return new extract_command(state, root);
}
