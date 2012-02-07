#include "filter.h"
#include "filter_table.h"
#include "common.h"

/* bbox of a single node */
class bbox_filter : public map_filter<bbox> {
public:
	bbox_filter(filter_input *input) : map_filter<bbox>(input) {}
	
	bool compute(const filter_param_set *params, bbox &b, bool adding) {
		const sgnode *n;
		ptlist pts;
		
		if (!get_filter_param(this , params, "node", n)) {
			return false;
		}
		n->get_world_points(pts);
		b = bbox(pts);
		return true;
	}
};

/*
 Handles all binary operations between bounding boxes. Currently this
 includes intersection and containment.
*/
class bbox_binary_filter : public map_filter<bool> {
public:
	bbox_binary_filter(filter_input *input, char type) : map_filter<bool>(input), type(type) {}
	
	bool compute(const filter_param_set *params, bool &res, bool adding) {
		filter_val *av, *bv;
		bbox a, b;
		
		if (!get_filter_param(this, params, "a", a)) {
			return false;
		}
		if (!get_filter_param(this, params, "b", b)) {
			return false;
		}
		
		switch (type) {
			case 'i':
				res = a.intersects(b);
				return true;
			case 'c':
				res = a.contains(b);
				return true;
			default:
				assert(false);
		}
	}

private:
	char type;
};

filter* make_bbox(scene *scn, filter_input *input) {
	return new bbox_filter(input);
}

filter* make_bbox_int(scene *scn, filter_input *input) {
	return new bbox_binary_filter(input, 'i');
}

filter* make_bbox_contains(scene *scn, filter_input *input) {
	return new bbox_binary_filter(input, 'c');
}

filter_table_entry bbox_fill_entry() {
	filter_table_entry e;
	e.name = "bbox";
	e.parameters.push_back("node");
	e.create = &make_bbox;
	e.calc = NULL;
	e.possible_args = &all_nodes;
	return e;
}

filter_table_entry bbox_int_fill_entry() {
	filter_table_entry e;
	e.name = "bbox_int";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_bbox_int;
	e.calc = NULL;
	e.possible_args = &all_node_pairs_unordered_no_repeat;
	return e;
}

filter_table_entry bbox_contains_fill_entry() {
	filter_table_entry e;
	e.name = "bbox_contains";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_bbox_contains;
	e.calc = NULL;
	e.possible_args = &all_node_pairs_unordered_no_repeat;
	return e;
}
