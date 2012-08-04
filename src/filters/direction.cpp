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
bool direction(const sgnode *a, const sgnode *b, int axis, int comp) {
	int i, dir[3];
	vec3 amin, amax, bmin, bmax;

	a->get_bounds().get_vals(amin, amax);
	b->get_bounds().get_vals(bmin, bmax);
	
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
bool dir(scene *scn, const vector<string> &args, int axis, int comp) {
	const sgnode *a = scn->get_node(args[0]), *b = scn->get_node(args[1]);
	
	assert(a != NULL && b != NULL);
	
	return direction(a, b, axis, comp);
}

bool north_of(scene *scn, const vector<string> &args) {
	return dir(scn, args, 1, 1);
}

bool south_of(scene *scn, const vector<string> &args) {
	return dir(scn, args, 1, -1);
}

bool east_of(scene *scn, const vector<string> &args) {
	return dir(scn, args, 0, 1);
}

bool west_of(scene *scn, const vector<string> &args) {
	return dir(scn, args, 0, -1);
}

bool vertically_aligned(scene *scn, const vector<string> &args) {
	return dir(scn, args, 0, 0);
}

bool horizontally_aligned(scene *scn, const vector<string> &args) {
	return dir(scn, args, 1, 0);
}

bool planar_aligned(scene *scn, const vector<string> &args) {
	return dir(scn, args, 2, 0);
}

bool above(scene *scn, const vector<string> &args) {
	return dir(scn, args, 2, 1);
}

bool below(scene *scn, const vector<string> &args) {
	return dir(scn, args, 2, -1);
}



/*
Filter version
*/

class direction_filter : public typed_map_filter<bool> {
public:
	direction_filter(Symbol *root, soar_interface *si, filter_input *input, int axis, int comp)
	: typed_map_filter<bool>(root, si, input), axis(axis), comp(comp) {}
	
	bool compute(const filter_param_set *p, bool adding, bool &res, bool &changed) {
		const sgnode *a, *b;
		
		if (!get_filter_param(this, p, "a", a)) {
			return false;
		}
		if (!get_filter_param(this, p, "b", b)) {
			return false;
		}
		
		bool newres = direction(a, b, axis, comp);
		changed = (newres != res);
		res = newres;
		return true;
	}
	
private:
	int axis, comp;
};

filter *make_north_of(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 1, 1);
}

filter *make_south_of(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 1, -1);
}

filter *make_east_of(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 0, 1);
}

filter *make_west_of(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 0, -1);
}

filter *make_vertically_aligned(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 0, 0);
}

filter *make_horizontally_aligned(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 1, 0);
}

filter *make_planar_aligned(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 2, 0);
}

filter *make_above(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 2, 1);
}

filter *make_below(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 2, -1);
}


filter_table_entry north_of_fill_entry() {
	filter_table_entry e;
	e.name = "north-of";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_north_of;
	e.calc = &north_of;
	e.possible_args = &all_node_pairs_ordered_no_repeat;
	return e;
}

filter_table_entry south_of_fill_entry() {
	filter_table_entry e;
	e.name = "south-of";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_south_of;
	e.calc = &south_of;
	e.possible_args = &all_node_pairs_ordered_no_repeat;
	return e;
}

filter_table_entry east_of_fill_entry() {
	filter_table_entry e;
	e.name = "east-of";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_east_of;
	e.calc = &east_of;
	e.possible_args = &all_node_pairs_ordered_no_repeat;
	return e;
}

filter_table_entry west_of_fill_entry() {
	filter_table_entry e;
	e.name = "west-of";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_west_of;
	e.calc = &west_of;
	e.possible_args = &all_node_pairs_ordered_no_repeat;
	return e;
}

filter_table_entry horizontally_aligned_fill_entry() {
	filter_table_entry e;
	e.name = "horizontally-aligned";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_horizontally_aligned;
	e.calc = &horizontally_aligned;
	e.possible_args = &all_node_pairs_unordered_no_repeat;
	return e;
}

filter_table_entry vertically_aligned_fill_entry() {
	filter_table_entry e;
	e.name = "vertically-aligned";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_vertically_aligned;
	e.calc = &vertically_aligned;
	e.possible_args = &all_node_pairs_unordered_no_repeat;
	return e;
}

filter_table_entry planar_aligned_fill_entry() {
	filter_table_entry e;
	e.name = "planar-aligned";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_planar_aligned;
	e.calc = &planar_aligned;
	e.possible_args = &all_node_pairs_unordered_no_repeat;
	return e;
}

filter_table_entry above_fill_entry() {
	filter_table_entry e;
	e.name = "above";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_above;
	e.calc = &above;
	e.possible_args = &all_node_pairs_ordered_no_repeat;
	return e;
}

filter_table_entry below_fill_entry() {
	filter_table_entry e;
	e.name = "below";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_below;
	e.calc = &below;
	e.possible_args = &all_node_pairs_ordered_no_repeat;
	return e;
}

