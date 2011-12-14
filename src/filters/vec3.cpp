#include "filter.h"

using namespace std;

class vec3_filter : public map_filter<vec3> {
public:
	vec3_filter(filter_input *input) : map_filter<vec3>(input) {}

	bool compute(filter_param_set *params, vec3 &v, bool adding) {
		if (!get_filter_param(this, params, "x", v.a[0]) ||
		    !get_filter_param(this, params, "y", v.a[1]) ||
		    !get_filter_param(this, params, "z", v.a[2]))
		{
			return false;
		}
		return true;
	}
};

filter *_make_vec3_filter_(scene *scn, filter_input *input) {
	return new vec3_filter(input);
}

class origin_filter : public filter {
public:
	origin_filter() : added(false) {}
	
	bool update_results() {
		if (!added) {
			add_result(new filter_val_c<vec3>(vec3()), NULL);
			added = true;
		}
		return true;
	}
	
private:
	bool added;
};

filter *_make_origin_filter_(scene *scn, filter_input *input) {
	return new origin_filter();
}
