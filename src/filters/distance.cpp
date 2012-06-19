#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

class distance_filter : public map_filter<float> {
public:
	distance_filter(filter_input *input) : map_filter<float>(input) {}

	bool compute(const filter_param_set *params, float &v, bool adding) {
		const sgnode *a, *b;
		vec3 ac, bc;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b))
		{
			return false;
		}
		ac = calc_centroid(a->get_world_points());
		bc = calc_centroid(b->get_world_points());
		
		v = (ac - bc).norm();
		return true;
	}
};

filter *make_distance_filter(scene *scn, filter_input *input) {
	return new distance_filter(input);
}

filter_table_entry distance_fill_entry() {
	filter_table_entry e;
	e.name = "distance";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_distance_filter;
	e.calc = NULL;
	e.possible_args = &all_node_pairs_unordered_no_repeat;
	return e;
}
