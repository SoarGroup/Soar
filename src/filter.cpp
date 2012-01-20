#include <sstream>
#include <iterator>
#include <utility>
#include "scene.h"
#include "filter.h"

using namespace std;

void concat_filter_input::combine(const input_table &inputs) {
	input_table::const_iterator i;
	for (i = inputs.begin(); i != inputs.end(); ++i) {
		filter_result::iter j;
		filter_param_set *p;
		filter_result *r = i->res;

		for (j = r->added_begin(); j != r->added_end(); ++j) {
			p = new filter_param_set();
			(*p)[i->name] = *j;
			val2params[*j] = p;
			add(p);
		}
		for (j = r->removed_begin(); j != r->removed_end(); ++j) {
			if (!map_pop(val2params, *j, p)) {
				assert(false);
			}
			remove(p);
		}
		for (j = r->changed_begin(); j != r->changed_end(); ++j) {
			p = val2params[*j];
			change(p);
		}
	}
}

void product_filter_input::combine(const input_table &inputs) {
	input_table::const_iterator i;
	for (i = inputs.begin(); i != inputs.end(); ++i) {
		filter_result::iter j;
		val2param_map::iterator k;
		param_set_list::iterator l;
		filter_result *r = i->res;
		
		for (j = r->removed_begin(); j != r->removed_end(); ++j) {
			k = val2params.find(*j);
			assert(k != val2params.end());
			for (l = k->second.begin(); l != k->second.end(); ++l) {
				remove(*l);
				erase_param_set(*l);
			}
		}
		for (j = r->changed_begin(); j != r->changed_end(); ++j) {
			k = val2params.find(*j);
			assert(k != val2params.end());
			for (l = k->second.begin(); l != k->second.end(); ++l) {
				change(*l);
			}
		}
	}
	gen_new_combinations(inputs);
}

class product_gen {
public:
	product_gen(const vector<filter_result::iter> &begin, const vector<filter_result::iter> &end)
	: begin(begin), curr(begin), end(end), i(0), first(true) {}
	
	bool next() {
		if (first) {
			for (int i = 0; i < begin.size(); ++i) {
				if (begin[i] == end[i]) {
					return false;
				}
			}
			first = false;
			return true;
		}
		
		while (true) {
			if (++curr[i] == end[i]) {
				if (i == curr.size() - 1) {
					return false;
				}
				curr[i] = begin[i];
			} else {
				return true;
			}
			if (++i >= curr.size()) {
				i = 0;
			}
		}
	}
	
	vector<filter_result::iter> curr;

private:
	vector<filter_result::iter> begin;
	vector<filter_result::iter> end;
	int i;
	bool first;
};

/*
 Generate all combinations of results that involve at least one new
 result.  Do this by iterating over the result lists.  For the i^th
 result list, take the cartesian product of the old results from lists
 0..(i-1), the new results of list i, and both old and new results from
 lists (i+1)..n.  This will avoid generating duplicates.  I'm assuming
 that new results are at the end of each result list.
*/
void product_filter_input::gen_new_combinations(const input_table &inputs) {
	vector<filter_result::iter> begin, added_begin, end;
	vector<string> names;
	int i, j, k;
	input_table::const_iterator ti;
	for (ti = inputs.begin(); ti != inputs.end(); ++ti) {
		names.push_back(ti->name);
		begin.push_back(ti->res->curr_begin());
		end.push_back(ti->res->curr_end());
		added_begin.push_back(ti->res->added_begin());
	}
	for (i = 0; i < begin.size(); ++i) {
		vector<filter_result::iter> tbegin, tend;
		for (j = 0; j < begin.size(); ++j) {
			if (j < i) {
				tbegin.push_back(begin[j]);
				tend.push_back(added_begin[j]); // same as end of old results
			} else if (j == i) {
				tbegin.push_back(added_begin[j]);
				tend.push_back(end[j]);
			} else {
				tbegin.push_back(begin[j]);
				tend.push_back(end[j]);
			}
		}
		product_gen gen(tbegin, tend);
		while (gen.next()) {
			vector<filter_result::iter>::const_iterator ci;
			vector<string>::const_iterator ni;
			filter_param_set *p = new filter_param_set();
			for (ci = gen.curr.begin(), ni = names.begin(); ci != gen.curr.end(); ++ci, ++ni) {
				(*p)[*ni] = **ci;
				val2params[**ci].push_back(p);
			}
			add(p);
		}
	}
}

void product_filter_input::erase_param_set(filter_param_set *s) {
	filter_param_set::const_iterator i;
	for (i = s->begin(); i != s->end(); ++i) {
		param_set_list &l = val2params[i->second];
		l.erase(find(l.begin(), l.end(), s));
	}
}

filter_input::~filter_input() {
	input_table::iterator i;
	for (i = input_info.begin(); i != input_info.end(); ++i) {
		delete i->f;
	}
}

bool filter_input::update() {
	input_table::iterator i;
	for (i = input_info.begin(); i != input_info.end(); ++i) {
		if (!i->f->update()) {
			return false;
		}
	}

	combine(input_info);

	for (i = input_info.begin(); i != input_info.end(); ++i) {
		i->res->clear_changes();
	}
	
	return true;
}

void filter_input::add_param(string name, filter *f) {
	param_info i;
	i.name = name;
	i.f = f;
	i.res = f->get_result();
	input_info.push_back(i);
}
