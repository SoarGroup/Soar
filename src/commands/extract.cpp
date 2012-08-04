#include <iostream>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include "soar_interface.h"

using namespace std;

class extract_command : public command, public filter_input_listener {
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
		if (!res_root) {
			res_root = si->get_wme_val(si->make_id_wme(root, "result"));
		}
		
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
			fltr->listen_for_input(this);
		}
		
		if (fltr) {
			if (!fltr->update()) {
				clear_results();
				return false;
			}
			update_results();
			res->clear_changes();
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
			filter_val *fv = res->get_removed(i);
			record r;
			if (!map_pop(records, fv, r)) {
				assert(false);
			}
			si->remove_wme(r.rec_wme);
		}
		for (int i = 0; i < res->num_changed(); ++i) {
			handle_result(res->get_changed(i));
		}
	}
	
	void clear_results() {
		record_map::iterator i;
		for (i = records.begin(); i != records.end(); ++i) {
			si->remove_wme(i->second.rec_wme);
		}
		records.clear();
	}
	
private:
	Symbol *make_filter_val_sym(filter_val *v) {
		int iv;
		double fv;
		bool bv;
		
		if (get_filter_val(v, iv)) {
			return si->make_sym(iv);
		}
		if (get_filter_val(v, fv)) {
			return si->make_sym(fv);
		}
		if (get_filter_val(v, bv)) {
			return si->make_sym(bv ? "t" : "f");
		}
		return si->make_sym(v->get_string());
	}
	
	wme *make_value_wme(filter_val *v, Symbol *root) {
		return si->make_wme(root, "value", make_filter_val_sym(v));
	}
	
	wme *make_param_struct(const filter_param_set *params, Symbol *rec_root) {
		wme *w = si->make_id_wme(rec_root, "params");
		Symbol *id = si->get_wme_val(w);
		
		filter_param_set::const_iterator i;
		for (i = params->begin(); i != params->end(); ++i) {
			si->make_wme(id, i->first, make_filter_val_sym(i->second));
		}
		return w;
	}
	
	void make_record(filter_val *result) {
		record r;
		r.rec_wme = si->make_id_wme(res_root, "record");
		r.rec_id = si->get_wme_val(r.rec_wme);
		r.val_wme = make_value_wme(result, r.rec_id);
		fltr->get_result_params(result, r.params);
		if (r.params) {
			r.params_wme = make_param_struct(r.params, r.rec_id);
		}
		records[result] = r;
	}
	
	void handle_result(filter_val *result) {
		record *r;
		if (r = map_get(records, result)) {
			si->remove_wme(r->val_wme);
			r->val_wme = make_value_wme(result, r->rec_id);
		} else {
			make_record(result);
		}
	}
	
	void handle_ctlist_change(const filter_param_set *p) {
		record_map::iterator i;
		for (i = records.begin(); i != records.end(); ++i) {
			if (i->second.params == p) {
				si->remove_wme(i->second.params_wme);
				i->second.params_wme = make_param_struct(p, i->second.rec_id);
				break;
			}
		}
	}
	
	Symbol         *root;
	Symbol         *res_root;
	Symbol         *pos_root;  // identifier for positive atoms
	Symbol         *neg_root;  // identifier for negative atoms
	svs_state      *state;
	soar_interface *si;
	filter         *fltr;
	filter_result  *res;
	bool            first;
	
	struct record {
		const filter_param_set *params;
		wme *rec_wme;
		wme *val_wme;
		wme *params_wme;
		Symbol *rec_id;
	};
	
	typedef map<filter_val*, record> record_map;
	record_map records;
};

command *_make_extract_command_(svs_state *state, Symbol *root) {
	return new extract_command(state, root);
}
