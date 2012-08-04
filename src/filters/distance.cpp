#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

class distance_filter : public typed_map_filter<float> {
public:
	distance_filter(Symbol *root, soar_interface *si, filter_input *input)
	: typed_map_filter<float>(root, si, input)
	{}

	bool compute(const filter_param_set *params, bool adding, float &res, bool &changed) {
		float newres;
		const sgnode *a, *b;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b))
		{
			return false;
		}
		
		newres = (a->get_centroid() - b->get_centroid()).norm();
		if (changed = (newres != res)) {
			res = newres;
		}
		return true;
	}
};

filter *make_distance_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new distance_filter(root, si, input);
}

filter_table_entry distance_fill_entry() {
	filter_table_entry e;
	e.name = "distance";
	e.create = &make_distance_filter;
	return e;
}

class closest_filter : public rank_filter {
public:
	closest_filter(Symbol *root, soar_interface *si, filter_input *input)
	: rank_filter(root, si, input)
	{}
	
	bool rank(const filter_param_set *params, double &rank) {
		const sgnode *a, *b;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b))
		{
			return false;
		}
		
		rank = -(a->get_centroid() - b->get_centroid()).norm();
		return true;
	}
};

filter *make_closest_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new closest_filter(root, si, input);
}

filter_table_entry closest_fill_entry() {
	filter_table_entry e;
	e.name = "closest";
	e.create = &make_closest_filter;
	return e;
}
