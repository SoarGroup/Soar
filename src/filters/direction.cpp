#include <vector>
#include <string>
#include "sgnode.h"
#include "scene.h"
#include "filter.h"
#include "filter_table.h"
#include "common.h"

using namespace std;

/*
Calculates whether a bounding box is located to the left (-1), aligned
(0), or right (1) of another bounding box along the specified axis.
*/
bool direction(sgnode *a, sgnode *b, int axis, int comp) {
	int i, dir[3];
	vec3 amin, amax, bmin, bmax;
	ptlist pa, pb;
	
	a->get_world_points(pa);
	b->get_world_points(pb);
	
	bbox ba(pa), bb(pb);
	ba.get_vals(amin, amax);
	bb.get_vals(bmin, bmax);
	
	/*
	 dir[i] = [-1, 0, 1] if a is [less than, overlapping,
	 greater than] b in that dimension.
	*/
	for(i = 0; i < 3; i++) {
		if (amax[i] <= bmin[i]) {
			dir[i] = -1;
		} else if (bmax[i] <= amin[i]) {
			dir[i] = 1;
		} else {
			dir[i] = 0;
		}
	}

	assert(0 <= axis && axis < 3);
	return comp == dir[axis];
}

/*
Standalone versions for use with model learning.
*/
bool standalone (scene *scn, const vector<string> &args, int axis, int comp) {
	sgnode *a = scn->get_node(args[0]), *b = scn->get_node(args[1]);
	
	assert(a != NULL && b != NULL);
	
	return direction(a, b, axis, comp);
}

bool standalone_north_of(scene *scn, const vector<string> &args) {
	return standalone(scn, args, 1, 1);
}

bool standalone_south_of(scene *scn, const vector<string> &args) {
	return standalone(scn, args, 1, -1);
}

bool standalone_east_of(scene *scn, const vector<string> &args) {
	return standalone(scn, args, 0, 1);
}

bool standalone_west_of(scene *scn, const vector<string> &args) {
	return standalone(scn, args, 0, -1);
}

bool standalone_vertically_aligned(scene *scn, const vector<string> &args) {
	return standalone(scn, args, 0, 0);
}

bool standalone_horizontally_aligned(scene *scn, const vector<string> &args) {
	return standalone(scn, args, 1, 0);
}

/*
Filter version
*/

class direction_filter : public map_filter<bool> {
public:
	direction_filter(filter_input *input, int axis, int comp)
	: map_filter<bool>(input), axis(axis), comp(comp) {}
	
	bool compute(filter_param_set *p, bool &res, bool adding) {
		sgnode *a, *b;
		
		if (!get_filter_param(this, p, "a", a)) {
			return false;
		}
		if (!get_filter_param(this, p, "b", b)) {
			return false;
		}
		
		res = direction(a, b, axis, comp);
		return true;
	}
	
private:
	int axis, comp;
};

filter *make_north_of(scene *scn, filter_input *input) {
	return new direction_filter(input, 1, 1);
}

filter *make_south_of(scene *scn, filter_input *input) {
	return new direction_filter(input, 1, -1);
}

filter *make_east_of(scene *scn, filter_input *input) {
	return new direction_filter(input, 0, 1);
}

filter *make_west_of(scene *scn, filter_input *input) {
	return new direction_filter(input, 0, -1);
}

filter *make_vertically_aligned(scene *scn, filter_input *input) {
	return new direction_filter(input, 0, 0);
}

filter *make_horizontally_aligned(scene *scn, filter_input *input) {
	return new direction_filter(input, 1, 0);
}

filter_table_entry north_of_fill_entry() {
	filter_table_entry e;
	e.name = "north-of";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_north_of;
	e.calc = &standalone_north_of;
	e.possible_args = &all_node_pairs_ordered_no_repeat;
	return e;
}

filter_table_entry south_of_fill_entry() {
	filter_table_entry e;
	e.name = "south-of";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_south_of;
	e.calc = &standalone_south_of;
	e.possible_args = &all_node_pairs_ordered_no_repeat;
	return e;
}

filter_table_entry east_of_fill_entry() {
	filter_table_entry e;
	e.name = "east-of";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_east_of;
	e.calc = &standalone_east_of;
	e.possible_args = &all_node_pairs_ordered_no_repeat;
	return e;
}

filter_table_entry west_of_fill_entry() {
	filter_table_entry e;
	e.name = "west-of";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_west_of;
	e.calc = &standalone_west_of;
	e.possible_args = &all_node_pairs_ordered_no_repeat;
	return e;
}

filter_table_entry horizontally_aligned_fill_entry() {
	filter_table_entry e;
	e.name = "horizontally-aligned";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_horizontally_aligned;
	e.calc = &standalone_horizontally_aligned;
	e.possible_args = &all_node_pairs_unordered_no_repeat;
	return e;
}

filter_table_entry vertically_aligned_fill_entry() {
	filter_table_entry e;
	e.name = "vertically-aligned";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_vertically_aligned;
	e.calc = &standalone_vertically_aligned;
	e.possible_args = &all_node_pairs_unordered_no_repeat;
	return e;
}
