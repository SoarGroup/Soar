#include <iostream>
#include <map>
#include "filter.h"
#include "filter_table.h"
#include "common.h"
#include "scene.h"

using namespace std;

class overlap_filter : public typed_map_filter<double> {
public:
	overlap_filter(Symbol *root, soar_interface *si, filter_input *input, scene *scn)
	: typed_map_filter<double>(root, si, input), scn(scn) {}
	
	bool compute(const filter_params *p, bool adding, double &res, bool &changed) {
		double newres;
		const sgnode *a, *b;
		
		if (!get_filter_param(this, p, "a", a)) {
			return false;
		}
		if (!get_filter_param(this, p, "b", b)) {
			return false;
		}
		
		newres = overlap(a, b);
		changed = (res != newres);
		res = newres;
		return true;
	}

private:
	scene *scn;

};

filter *make_overlap_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new overlap_filter(root, si, input, scn);
}

double standalone_overlap(const scene *scn, const vector<const sgnode*> &args) {
	assert(args.size() == 2);
	return overlap(args[0], args[1]);
}

filter_table_entry *overlap_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "overlap";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = false;
	e->allow_repeat = false;
	e->create = &make_overlap_filter;
	return e;
}
