#include <iostream>
#include "filter.h"
#include "filter_table.h"
#include "common.h"
#include "bullet_support.h"

using namespace std;

/*
 Return true if and only if the convex hull containing the bounding
 boxes of a and c intersects the bounding box of b
*/
bool between(const sgnode *a, const sgnode *b, const sgnode *c) {
	ptlist pa, pb, pc;
	a->get_bounds().get_points(pa);
	b->get_bounds().get_points(pb);
	c->get_bounds().get_points(pc);
	
	copy(pa.begin(), pa.end(), back_inserter(pc));
		
	return (hull_distance(pb, pc) < 0);
}

/*
 Given 3 objects a, b, and c, returns whether all points in c are behind
 or in front of all points in a w.r.t. the vector from the centroid of
 a to the centroid of b. The parameter dir should be positive if an in
 front query is desired, and negative if a behind query is desired.
*/
bool behind(const sgnode *a, const sgnode *b, const sgnode *c) {
	ptlist pa, pc;
	a->get_bounds().get_points(pa);
	c->get_bounds().get_points(pc);
	
	vec3 u = b->get_centroid() - a->get_centroid();
	
	return (dir_separation(pa, pc, u) <= 0);
}

class between_filter : public typed_map_filter<bool> {
public:
	between_filter(Symbol *root, soar_interface *si, filter_input *input) 
	: typed_map_filter<bool>(root, si, input) {}
	
	bool compute(const filter_param_set *params, bool adding, bool &res, bool &changed) {
		const sgnode *a, *b, *c;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b) ||
		    !get_filter_param(this, params, "c", c))
		{
			return false;
		}
		bool newres = between(a, b, c);
		changed = (newres != res);
		res = newres;
		return true;
	}
};

class behind_filter : public typed_map_filter<bool> {
public:
	behind_filter(Symbol *root, soar_interface *si, filter_input *input)
	: typed_map_filter<bool>(root, si, input)
	{}
	
	bool compute(const filter_param_set *params, bool adding, bool &res, bool &changed) {
		const sgnode *a, *b, *c;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b) ||
		    !get_filter_param(this, params, "c", c))
		{
			return false;
		}
		
		bool newres = behind(a, b, c);
		changed = (newres != res);
		res = newres;
		return true;
	}
};

filter* make_between_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new between_filter(root, si, input);
}

filter* make_behind_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new behind_filter(root, si, input);
}

bool calc_between(scene *scn, const vector<string> &args) {
	const sgnode *a, *b, *c;
	a = scn->get_node(args[0]);
	b = scn->get_node(args[1]);
	c = scn->get_node(args[2]);
	return between(a, b, c);
}

bool calc_behind(scene *scn, const vector<string> &args) {
	const sgnode *a, *b, *c;
	a = scn->get_node(args[0]);
	b = scn->get_node(args[1]);
	c = scn->get_node(args[2]);
	return behind(a, b, c);
}
	
filter_table_entry between_fill_entry() {
	filter_table_entry e;
	e.name = "between";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.parameters.push_back("c");
	e.create = &make_between_filter;
	if (!get_option("order3").empty()) {
		e.calc = &calc_between;
	} else {
		e.calc = NULL;
	}
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
	if (!get_option("order3").empty()) {
		e.calc = &calc_behind;
	} else {
		e.calc = NULL;
	}
	e.possible_args = &all_node_triples_ordered_no_repeat;
	return e;
}
