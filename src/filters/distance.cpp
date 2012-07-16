#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

class distance_filter : public typed_map_filter<float> {
public:
	distance_filter(filter_input *input) : typed_map_filter<float>(input) {}

	bool compute(const filter_param_set *params, bool adding, float &res, bool &changed) {
		float newres;
		const sgnode *a, *b;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b))
		{
			return false;
		}
		
		newres = (a->get_world_centroid() - b->get_world_centroid()).norm();
		if (changed = (newres != res)) {
			res = newres;
		}
		return true;
	}
};

filter *make_distance_filter(scene *scn, filter_input *input) {
	return new distance_filter(input);
}

filter_table_entry distance_fill_entry() {
	filter_table_entry e;
	e.name = "distance";
	e.create = &make_distance_filter;
	return e;
}

class closest_filter : public rank_filter {
public:
	closest_filter(filter_input *input) : rank_filter(input) {}
	
	bool rank(const filter_param_set *params, double &rank) {
		const sgnode *a, *b;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b))
		{
			return false;
		}
		
		rank = -(a->get_world_centroid() - b->get_world_centroid()).norm();
		return true;
	}
};

filter *make_closest_filter(scene *scn, filter_input *input) {
	return new closest_filter(input);
}

filter_table_entry closest_fill_entry() {
	filter_table_entry e;
	e.name = "closest";
	e.create = &make_closest_filter;
	return e;
}
