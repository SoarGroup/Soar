#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

class compare_filter : public typed_map_filter<bool> {
public:
	compare_filter(filter_input *input) : typed_map_filter<bool>(input) {}
	
	bool compute(const filter_param_set *params, bool adding, bool &res, bool &changed) {
		float a, b;
		string comp;
		bool newres;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b) ||
		    !get_filter_param(this, params, "compare", comp))
		{
			return false;
		}
		if (comp == "gt") {
			newres = (a > b);
		} else if (comp == "gte") {
			newres = (a >= b);
		} else if (comp == "lt") {
			newres = (a < b);
		} else if (comp == "lte") {
			newres = (a <= b);
		} else if (comp == "eq") {
			newres = (a == b);
		} else {
			return false;
		}
		changed = (newres != res);
		res = newres;
		return true;
	}
};

filter *make_compare_filter(scene *scn, filter_input *input) {
	return new compare_filter(input);
}

filter_table_entry compare_fill_entry() {
	filter_table_entry e;
	e.name = "compare";
	e.parameters.push_back("compare");
	e.create = &make_compare_filter;
	return e;
}
