#include <vector>
#include <string>
#include "sgnode.h"
#include "scene.h"
#include "filter.h"
#include "filter_table.h"
#include "common.h"

using namespace std;

/*
 Percentage of the smaller of two bodies that they can overlap before being
 considered overlapping.
*/
const double OVERLAP_MARGIN = .05;

/*
Calculates whether a bounding box is located to the left (-1), overlapping
(0), or right (1) of another bounding box along the specified axis.
*/
bool direction(const sgnode *a, const sgnode *b, int axis, int comp) {
	assert(0 <= axis && axis < 3);
	
	int dir;
	vec3 amin, amax, bmin, bmax;
	double asize, bsize, margin;

	a->get_bounds().get_vals(amin, amax);
	b->get_bounds().get_vals(bmin, bmax);
	
	asize = amax(axis) - amin(axis);
	bsize = bmax(axis) - bmin(axis);
	margin = OVERLAP_MARGIN * ( asize < bsize ? asize : bsize );
	
	/*
	 dir = [-1, 0, 1] if a is [less than, overlapping,
	 greater than] b in that dimension.
	*/
	if (amax[axis] <= bmin[axis] + margin) {
		dir = -1;
	} else if (bmax[axis] <= amin[axis] + margin) {
		dir = 1;
	} else {
		dir = 0;
	}
	return comp == dir;
}

bool north_of(const scene *scn, const vector<const sgnode*> &args) {
	assert(args.size() == 2);
	return direction(args[0], args[1], 1, 1);
}

bool south_of(const scene *scn, const vector<const sgnode*> &args) {
	assert(args.size() == 2);
	return direction(args[0], args[1], 1, -1);
}

bool east_of(const scene *scn, const vector<const sgnode*> &args) {
	assert(args.size() == 2);
	return direction(args[0], args[1], 0, 1);
}

bool west_of(const scene *scn, const vector<const sgnode*> &args) {
	assert(args.size() == 2);
	return direction(args[0], args[1], 0, -1);
}

bool x_overlap(const scene *scn, const vector<const sgnode*> &args) {
	assert(args.size() == 2);
	return direction(args[0], args[1], 0, 0);
}

bool y_overlap(const scene *scn, const vector<const sgnode*> &args) {
	assert(args.size() == 2);
	return direction(args[0], args[1], 1, 0);
}

bool z_overlap(const scene *scn, const vector<const sgnode*> &args) {
	assert(args.size() == 2);
	return direction(args[0], args[1], 2, 0);
}

bool above(const scene *scn, const vector<const sgnode*> &args) {
	assert(args.size() == 2);
	return direction(args[0], args[1], 2, 1);
}

bool below(const scene *scn, const vector<const sgnode*> &args) {
	assert(args.size() == 2);
	return direction(args[0], args[1], 2, -1);
}

/*
Filter version
*/

class direction_filter : public typed_map_filter<bool> {
public:
	direction_filter(Symbol *root, soar_interface *si, filter_input *input, int axis, int comp)
	: typed_map_filter<bool>(root, si, input), axis(axis), comp(comp) {}
	
	bool compute(const filter_params *p, bool adding, bool &res, bool &changed) {
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

filter *make_x_overlap(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 0, 0);
}

filter *make_y_overlap(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 1, 0);
}

filter *make_z_overlap(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 2, 0);
}

filter *make_above(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 2, 1);
}

filter *make_below(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new direction_filter(root, si, input, 2, -1);
}


filter_table_entry *north_of_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "north-of";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = true;
	e->allow_repeat = false;
	e->create = &make_north_of;
	e->calc = &north_of;
	return e;
}

filter_table_entry *south_of_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "south-of";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = true;
	e->allow_repeat = false;
	e->create = &make_south_of;
	e->calc = &south_of;
	return e;
}

filter_table_entry *east_of_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "east-of";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = true;
	e->allow_repeat = false;
	e->create = &make_east_of;
	e->calc = &east_of;
	return e;
}

filter_table_entry *west_of_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "west-of";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = true;
	e->allow_repeat = false;
	e->create = &make_west_of;
	e->calc = &west_of;
	return e;
}

filter_table_entry *x_overlap_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "x-overlap";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = false;
	e->allow_repeat = false;
	e->create = &make_x_overlap;
	e->calc = &x_overlap;
	return e;
}

filter_table_entry *y_overlap_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "y-overlap";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = false;
	e->allow_repeat = false;
	e->create = &make_y_overlap;
	e->calc = &y_overlap;
	return e;
}

filter_table_entry *z_overlap_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "z-overlap";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = false;
	e->allow_repeat = false;
	e->create = &make_z_overlap;
	e->calc = &z_overlap;
	return e;
}

filter_table_entry *above_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "above";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = true;
	e->allow_repeat = false;
	e->create = &make_above;
	e->calc = &above;
	return e;
}

filter_table_entry *below_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "below";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = true;
	e->allow_repeat = false;
	e->create = &make_below;
	e->calc = &below;
	return e;
}
