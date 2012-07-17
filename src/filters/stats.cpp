#include <vector>
#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

bool get_double(const filter_param_set *params, double &v) {
	if (params->empty()) {
		return false;
	}

	if (!get_filter_val(params->begin()->second, v)) {
		return false;
	}
	return true;
}

class max_filter : public reduce_filter<double> {
public:
	max_filter(filter_input *input) : reduce_filter<double>(input) {}

private:
	bool input_added(const filter_param_set *params, double &res) {
		double x;
		if (!get_double(params, x)) {
			return false;
		}

		elems.push_back(make_pair(x, params));
		res = max_element(elems.begin(), elems.end())->first;
		return true;
	}

	bool input_changed(const filter_param_set *params, double &res) {
		for (int i = 0; i < elems.size(); ++i) {
			if (elems[i].second == params) {
				if (!get_double(params, elems[i].first)) {
					return false;
				}
				res = max_element(elems.begin(), elems.end())->first;
				return true;
			}
		}
		assert(false);
	}

	bool input_removed(const filter_param_set *params, double &res) {
		for (int i = 0; i < elems.size(); ++i) {
			if (elems[i].second == params) {
				elems.erase(elems.begin() + i);
				if (!elems.empty()) {
					res = max_element(elems.begin(), elems.end())->first;
				}
				return true;
			}
		}
		assert(false);
	}

	vector<pair<double, const filter_param_set*> > elems;
};

filter *make_max_filter(scene *scn, filter_input *input) {
	return new max_filter(input);
}

filter_table_entry max_fill_entry() {
	filter_table_entry e;
	e.name = "max";
	e.create = &make_max_filter;
	return e;
}
