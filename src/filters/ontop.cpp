#include "filter.h"
#include "filter_table.h"
#include "common.h"
#include "scene.h"
#include "linalg.h"

using namespace std;

bool ontop(const sgnode *tn, const sgnode *bn) {
	vec3 tmin, tmax, bmin, bmax;
	
	bbox tb(tn->get_world_points()), bb(bn->get_world_points());
	tb.get_vals(bmin, bmax);
	bb.get_vals(tmin, tmax);
	return tb.intersects(bb) && tmin[2] == bmax[2];
}

bool standalone(scene *scn, const vector<string> &args) {
	sgnode *tn = scn->get_node(args[0]), *bn = scn->get_node(args[1]);
	return ontop(tn, bn);
}

class ontop_filter : public map_filter<bool> {
public:
	ontop_filter(filter_input *input) : map_filter<bool>(input) {}

	bool compute(const filter_param_set *params, bool &result, bool adding) {
		const sgnode *tn, *bn;
		if (!get_filter_param(this, params, "top", tn) || 
		    !get_filter_param(this, params, "bottom", tn))
		{
			return false;
		}
		result = ontop(tn, bn);
		return true;
	}
};

filter* make_ontop_filter(scene *scn, filter_input *input) {
	return new ontop_filter(input);
}

filter_table_entry ontop_fill_entry() {
	filter_table_entry e;
	e.name = "on-top";
	e.parameters.push_back("top");
	e.parameters.push_back("bottom");
	e.create = &make_ontop_filter;
	e.calc = &standalone;
	e.possible_args = &all_node_pairs_ordered_no_repeat;
	return e;
}
