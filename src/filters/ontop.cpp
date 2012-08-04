#include "filter.h"
#include "filter_table.h"
#include "common.h"
#include "scene.h"
#include "linalg.h"

using namespace std;

bool ontop(const sgnode *tn, const sgnode *bn) {
	vec3 tmin, tmax, bmin, bmax;
	
	const bbox &tb = tn->get_bounds();
	const bbox &bb = bn->get_bounds();
	tb.get_vals(tmin, tmax);
	bb.get_vals(bmin, bmax);
	return tb.intersects(bb) && tmin[2] == bmax[2];
}

bool standalone(scene *scn, const vector<string> &args) {
	sgnode *tn = scn->get_node(args[0]), *bn = scn->get_node(args[1]);
	return ontop(tn, bn);
}

class ontop_filter : public typed_map_filter<bool> {
public:
	ontop_filter(Symbol* root, soar_interface *si, filter_input *input) 
	: typed_map_filter<bool>(root, si, input)
	{}

	bool compute(const filter_param_set *params, bool adding, bool &res, bool &changed) {
		const sgnode *tn, *bn;
		if (!get_filter_param(this, params, "top", tn) || 
		    !get_filter_param(this, params, "bottom", bn))
		{
			return false;
		}
		bool newres = ontop(tn, bn);
		changed = (newres != res);
		res = newres;
		return true;
	}
};

filter* make_ontop_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new ontop_filter(root, si, input);
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
