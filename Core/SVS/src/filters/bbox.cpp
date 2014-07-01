#include "filter.h"
#include "filter_table.h"
#include "common.h"

/* bbox of a single node */
class bbox_filter : public typed_map_filter<bbox> {
public:
	bbox_filter(Symbol *root, soar_interface *si, filter_input *input)
	: typed_map_filter<bbox>(root, si, input)
	{}
	
	bool compute(const filter_params *params, bool adding, bbox &res, bool &changed) {
		const sgnode *n;
		
		if (!get_filter_param(this , params, "node", n)) {
			return false;
		}
		changed = (res != n->get_bounds());
		res = n->get_bounds();
		return true;
	}
};

/*
 Handles all binary operations between bounding boxes. Currently this
 includes intersection and containment.
*/
class bbox_binary_filter : public typed_map_filter<bool> {
public:
	bbox_binary_filter(Symbol *root, soar_interface *si, filter_input *input, char type) 
	: typed_map_filter<bool>(root, si, input), type(type)
	{}
	
	bool compute(const filter_params *params, bool adding, bool &res, bool &changed) {
		filter_val *av, *bv;
		bbox a, b;
		
		if (!get_filter_param(this, params, "a", a)) {
			return false;
		}
		if (!get_filter_param(this, params, "b", b)) {
			return false;
		}
		
		bool newres;
		switch (type) {
			case 'i':
				newres = a.intersects(b);
				break;
			case 'c':
				newres = a.contains(b);
				break;
			default:
				assert(false);
		}
		changed = (newres != res);
		res = newres;
		return true;
	}

private:
	char type;
};

filter* make_bbox(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new bbox_filter(root, si, input);
}

filter* make_bbox_int(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new bbox_binary_filter(root, si, input, 'i');
}

filter* make_bbox_contains(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new bbox_binary_filter(root, si, input, 'c');
}

filter_table_entry *bbox_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "bbox";
	e->parameters.push_back("node");
	e->create = &make_bbox;
	e->calc = NULL;
	return e;
}

filter_table_entry *bbox_int_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "bbox_int";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = false;
	e->allow_repeat = false;
	e->create = &make_bbox_int;
	e->calc = NULL;
	return e;
}

filter_table_entry *bbox_contains_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "bbox_contains";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = false;
	e->allow_repeat = false;
	e->create = &make_bbox_contains;
	e->calc = NULL;
	return e;
}
