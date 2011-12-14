#include <iostream>
#include "filter.h"
#include "filter_table.h"
#include "common.h"
#include "bullet_support.h"

using namespace std;

class between_filter : public map_filter<bool> {
public:
	between_filter(filter_input *input) : map_filter<bool>(input) {}
	
	bool compute(filter_param_set *params, bool &result, bool adding) {
		sgnode *na, *nb, *nc;
		ptlist pa, pb, pc;
		
		if (!get_filter_param(this, params, "a", na) ||
		    !get_filter_param(this, params, "b", nb) ||
		    !get_filter_param(this, params, "c", nc))
		{
			return false;
		}
		
		na->get_world_points(pa);
		nb->get_world_points(pb);
		nc->get_world_points(pc);
		
		copy(pa.begin(), pa.end(), back_inserter(pc));
		
		result = (hull_distance(pb, pc) < 0);
		return true;
	}
};

/*
 Given 3 objects a, b, and c, returns whether all points in c are behind
 or in front of all points in a w.r.t. the vector from the centroid of
 a to the centroid of b. The parameter dir should be positive if an in
 front query is desired, and negative if a behind query is desired.
*/
class behind_filter : public map_filter<bool> {
public:
	behind_filter(filter_input *input) : map_filter<bool>(input) {}
	
	bool compute(filter_param_set *params, bool &result, bool adding) {
		sgnode *na, *nb, *nc;
		ptlist pa, pb, pc;
		
		if (!get_filter_param(this, params, "a", na) ||
		    !get_filter_param(this, params, "b", nb) ||
		    !get_filter_param(this, params, "c", nc))
		{
			return false;
		}
		
		na->get_world_points(pa);
		nb->get_world_points(pb);
		nc->get_world_points(pc);
		
		vec3 ca = calc_centroid(pa);
		vec3 cb = calc_centroid(pb);
		vec3 u = cb - ca;
		
		result = (dir_separation(pa, pc, u) <= 0);
		return true;
	}
};

filter* make_between_filter(scene *scn, filter_input *input) {
	return new between_filter(input);
}

filter* make_behind_filter(scene *scn, filter_input *input) {
	return new behind_filter(input);
}

filter_table_entry between_fill_entry() {
	filter_table_entry e;
	e.name = "between";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.parameters.push_back("c");
	e.create = &make_between_filter;
	e.calc = NULL;
	e.possible_args = &all_node_triples_ordered_no_repeat;
	return e;
}

filter_table_entry behind_fill_entry() {
	filter_table_entry e;
	e.name = "behind";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.parameters.push_back("c");
	e.create = &make_behind_filter;
	e.calc = NULL;
	e.possible_args = &all_node_triples_ordered_no_repeat;
	return e;
}
