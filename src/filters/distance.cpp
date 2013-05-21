#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

class distance_filter : public typed_map_filter<double> {
public:
	distance_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input)
	: typed_map_filter<double>(root, si, input), scn(scn)
	{}

	bool compute(const filter_params *params, bool adding, double &res, bool &changed) {
		double newres;
		const sgnode *a, *b;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b))
		{
			return false;
		}
		
		if (scn->tracking_distances()) {
			newres = scn->convex_distance(a, b);
		} else {
			newres = convex_distance(a, b);
		}
		if ((changed = (newres != res))) {
			res = newres;
		}
		return true;
	}
private:
	scene *scn;
};

filter *make_distance_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new distance_filter(root, si, scn, input);
}

filter_table_entry *distance_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "distance";
	e->create = &make_distance_filter;
	return e;
}

class centroid_distance_filter : public typed_map_filter<double> {
public:
	centroid_distance_filter(Symbol *root, soar_interface *si, filter_input *input)
	: typed_map_filter<double>(root, si, input)
	{}

	bool compute(const filter_params *params, bool adding, double &res, bool &changed) {
		double newres;
		const sgnode *a, *b;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b))
		{
			return false;
		}
		
		newres = (a->get_centroid() - b->get_centroid()).norm();
		if ((changed = (newres != res))) {
			res = newres;
		}
		return true;
	}
};

filter *make_centroid_distance_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new centroid_distance_filter(root, si, input);
}

filter_table_entry *centroid_distance_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "centroid_distance";
	e->create = &make_centroid_distance_filter;
	return e;
}

class closest_filter : public rank_filter {
public:
	closest_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input)
	: rank_filter(root, si, input), scn(scn)
	{}
	
	bool rank(const filter_params *params, double &rank) {
		const sgnode *a, *b;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b))
		{
			return false;
		}
		
		if (scn->tracking_distances()) {
			rank = -scn->convex_distance(a, b);
		} else {
			rank = -convex_distance(a, b);
		}
		return true;
	}
private:
	scene *scn;
};

filter *make_closest_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new closest_filter(root, si, scn, input);
}

filter_table_entry *closest_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "closest";
	e->create = &make_closest_filter;
	return e;
}
