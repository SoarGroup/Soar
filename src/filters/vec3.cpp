#include "filter.h"

using namespace std;

class vec3_filter : public typed_map_filter<vec3> {
public:
	vec3_filter(filter_input *input) : typed_map_filter<vec3>(input) {}

	bool compute(const filter_param_set *params, bool adding, vec3 &res, bool &changed) {
		vec3 newres;
		if (!get_filter_param(this, params, "x", newres[0]) ||
		    !get_filter_param(this, params, "y", newres[1]) ||
		    !get_filter_param(this, params, "z", newres[2]))
		{
			return false;
		}
		changed = (res != newres);
		res = newres;
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
