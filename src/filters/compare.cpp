#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

class compare_filter : public map_filter<bool> {
public:
	compare_filter(filter_input *input) : map_filter<bool>(input) {}
	
	bool compute(const filter_param_set *params, bool &result, bool adding) {
		float a, b;
		string comp;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b) ||
		    !get_filter_param(this, params, "compare", comp))
		{
			return false;
		}
		if (comp == "gt") {
			result = (a > b);
		} else if (comp == "gte") {
			result = (a >= b);
		} else if (comp == "lt") {
			result = (a < b);
		} else if (comp == "lte") {
			result = (a <= b);
		} else if (comp == "eq") {
			result = (a == b);
		} else {
			return false;
		}
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
