#include <iostream>
#include <map>
#include "filter.h"
#include "filter_table.h"
#include "common.h"
#include "scene.h"

using namespace std;

class intersect_filter : public typed_map_filter<bool> {
public:
	intersect_filter(Symbol *root, soar_interface *si, filter_input *input, scene *scn)
	: typed_map_filter<bool>(root, si, input), scn(scn) {}
	
	bool compute(const filter_params *p, bool adding, bool &res, bool &changed) {
		bool newres;
		const sgnode *a, *b;
		
		if (!get_filter_param(this, p, "a", a)) {
			return false;
		}
		if (!get_filter_param(this, p, "b", b)) {
			return false;
		}
		
		if (scn->tracking_distances()) {
			newres = scn->intersects(a, b);
		} else {
			newres = intersects(a, b);
		}
		changed = (res != newres);
		res = newres;
		return true;
	}

private:
	scene *scn;

};

filter *make_intersect_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new intersect_filter(root, si, input, scn);
}

bool standalone_intersect(const scene *scn, const vector<const sgnode*> &args) {
	assert(args.size() == 2);
	if (scn->tracking_distances()) {
		return scn->intersects(args[0], args[1]);
	}
	return intersects(args[0], args[1]);
}

filter_table_entry *intersect_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "intersect";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = false;
	e->allow_repeat = false;
	e->create = &make_intersect_filter;
	e->calc = &standalone_intersect;
	return e;
}
