#include <iostream>
#include <map>
#include "filter.h"
#include "filter_table.h"
#include "common.h"
#include "scene.h"

using namespace std;

class intersect_filter : public map_filter<bool> {
public:
	intersect_filter(filter_input *input, scene *scn) : map_filter<bool>(input), scn(scn) {}
	
	bool compute(const filter_param_set *p, bool &res, bool adding) {
		const sgnode *a, *b;
		
		if (!get_filter_param(this, p, "a", a)) {
			return false;
		}
		if (!get_filter_param(this, p, "b", b)) {
			return false;
		}
		
		res = scn->intersects(a, b);
		return true;
	}

private:
	scene *scn;

};

filter *make_intersect_filter(scene *scn, filter_input *input) {
	return new intersect_filter(input, scn);
}

bool standalone_intersect(scene *scn, const vector<string> &args) {
	assert(args.size() == 2);
	return scn->intersects(args[0], args[1]);
}

filter_table_entry intersect_fill_entry() {
	filter_table_entry e;
	e.name = "intersect";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_intersect_filter;
	e.calc = &standalone_intersect;
	e.possible_args = &all_node_pairs_unordered_no_repeat;
	return e;
}
