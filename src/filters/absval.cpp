#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

class absval_filter : public typed_map_filter<float> {
public:
	absval_filter(Symbol *root, soar_interface *si, filter_input *input) 
	: typed_map_filter<float>(root, si, input)
	{}

	bool compute(const filter_params *params, bool adding, float &res, bool &changed) {
		float newres;
		if (params->empty()) {
			return false;
		}
		filter_val *fv = params->begin()->second;
		if (!get_filter_val(fv, newres)) {
			return false;
		}
		newres = fabs(newres);
		changed = (newres != res);
		res = newres;
		return true;
	}
};

filter *make_absval_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new absval_filter(root, si, input);
}

filter_table_entry absval_fill_entry() {
	filter_table_entry e;
	e.name = "absval";
	e.parameters.push_back("a");
	e.create = &make_absval_filter;
	return e;
}
