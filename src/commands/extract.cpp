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
			record_wmes r;
			if (!map_pop(res2wme, res->get_removed(i), r)) {
				assert(false);
			}
			param_wmes.erase(fltr->get_result_params(result, params));
			si->remove_wme(r.record);
		}
		for (int i = 0; i < res->num_changed(); ++i) {
			handle_result(res->get_changed(i));
		}
	}
	
	void clear_results() {
		std::map<filter_val*, record_wmes>::iterator i;
		for (i = res2wme.begin(); i != res2wme.end(); ++i) {
			si->remove_wme(i->second.record);
		}
		res2wme.clear();
		param_wmes.clear();
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
	
	wme *make_param_struct(const filter_param_set *params, Symbol *root) {
		wme *w = si->make_id_wme(root, "params");
		Symbol *id = si->get_wme_val(w);
		
		filter_param_set::const_iterator i;
		for (i = params->begin(); i != params->end(); ++i) {
			si->make_wme(id, i->first, make_filter_val_sym(i->second));
		}
		return w;
	}
	
	void make_record(filter_val *result) {
		record_wmes r;
		r.record = si->make_id_wme(res_root, "record");
		Symbol *rec_root = si->get_wme_val(r.record);
		r.value = make_value_wme(result, rec_root);

		const filter_param_set *params;
		if (!fltr->get_result_params(result, params)) {
			assert(false);
		}
		param_wmes[params] = make_param_struct(params, rec_root);
		
		res2wme[result] = r;
	}
	
	void handle_result(filter_val *result) {
		record_wmes *r;
		if (r = map_get(res2wme, result)) {
			si->remove_wme(r->value);
			r->value = make_value_wme(result, si->get_wme_val(r->record));
		} else {
			if (res_root == NULL) {
				res_root = si->get_wme_val(si->make_id_wme(root, "result"));
			}
			make_record(result);
		}
	}
	
	void handle_ctlist_change(const filter_param_set *p) {
		map<const filter_param_set*, wme*>::iterator i;
		i = param_wmes.find(p);
		assert(i != param_wmes.end());
		Symbol *root = si->get_wme_id(i->second);
		si->remove_wme(i->second);
		i->second = make_param_struct(p, root);
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
	
	struct record_wmes {
		wme *record;
		wme *value;
	};
	
	map<filter_val*, record_wmes> res2wme;
	map<const filter_param_set*, wme*> param_wmes;
};

command *_make_extract_command_(svs_state *state, Symbol *root) {
	return new extract_command(state, root);
}
