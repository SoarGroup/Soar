#include <sstream>
#include <vector>
#include "filter.h"
#include "sgnode.h"
#include "scene.h"

using namespace std;
int gen_counter = 0;

/*
 Need to have better memory management of nodes in general. Right now
 whenever a node is deleted all its children are also deleted, which
 may break the caching that's going on in the map_filter class.
*/
class gen_filter : public map_filter<sgnode*> {
public:
	gen_filter(filter_input *input) : map_filter<sgnode*>(input) {}

	bool compute(filter_param_set *params, sgnode *&res, bool adding) {
		string name;
		vec3 pos, rot, scale, singlept;
		filter_val *ptsval;
		ptlist *pts = NULL;
		bool dealloc_pts = false;
		stringstream ss;
		
		if (!get_filter_param(NULL, params, "name", name)) {
			ss << "gen_node_" << gen_counter++;
			name = ss.str();
		}
		if (!get_filter_param(NULL, params, "pos", pos)) {
			pos = vec3(0., 0., 0.);
		}
		if (!get_filter_param(NULL, params, "rot", rot)) {
			rot = vec3(0., 0., 0.);
		}
		if (!get_filter_param(NULL, params, "scale", scale)) {
			scale = vec3(1., 1., 1.);
		}
		
		if (!get_filter_param(NULL, params, "points", pts)) {
			if (get_filter_param(this, params, "points", singlept)) {
				pts = new ptlist();
				dealloc_pts = true;
				pts->push_back(singlept);
				res = new sgnode(name, *pts);
			}
		}
		
		if (adding || name != res->get_name()) {
			if (!adding) {
				delete res;
			}
			if (pts) {
				res = new sgnode(name, *pts);
			} else {
				res = new sgnode(name);
			}
		} else {
			if (pts) {
				res->set_local_points(*pts);
			}
		}
		
		if (dealloc_pts) {
			delete pts;
		}
		
		res->set_trans(pos, rot, scale);
		return true;
	}
};

filter* _make_gen_filter_(scene *scn, filter_input *input) {
	return new gen_filter(input);
}
