#include <list>
#include "filter.h"
#include "sgnode.h"
#include "linalg.h"
#include "scene.h"

using namespace std;

class node_ptlist_filter : public map_filter<ptlist*> {
public:
	node_ptlist_filter(filter_input *input, bool local) : map_filter<ptlist*>(input), local(local) {}
	
	~node_ptlist_filter() {
		std::list<ptlist*>::iterator i;
		for (i = lists.begin(); i != lists.end(); ++i) {
			delete *i;
		}
	}
	
	bool compute(filter_param_set *params, ptlist *&res, bool adding) {
		sgnode *n;
		if (!get_filter_param(this, params, "node", n)) {
			return false;
		}
		
		if (adding) {
			res = new ptlist();
			lists.push_back(res);
		}
		res->clear();
		
		if (local) {
			n->get_local_points(*res);
		} else {
			n->get_world_points(*res);
		}
		
		return true;
	}
	
	void result_removed(ptlist *&res) {
		lists.remove(res);
		delete res;
	}
	
private:
	bool local;
	std::list<ptlist*> lists;
};

filter* _make_local_filter_(scene *scn, filter_input *input) {
	return new node_ptlist_filter(input, true);
}

filter* _make_world_filter_(scene *scn, filter_input *input) {
	return new node_ptlist_filter(input, false);
}

class ptlist_filter : public map_filter<ptlist*> {
public:
	ptlist_filter(filter_input *input) : map_filter<ptlist*>(input) {}
	
	~ptlist_filter() {
		std::list<ptlist*>::iterator i;
		for (i = lists.begin(); i != lists.end(); ++i) {
			delete *i;
		}
	}
	
	bool compute(filter_param_set *params, ptlist *&res, bool adding) {
		filter_param_set::iterator i;
		
		if (adding) {
			res = new ptlist();
			lists.push_back(res);
		}
		res->clear();
		
		for(i = params->begin(); i != params->end(); ++i) {
			vec3 v;
			if (!get_filter_val(i->second, v)) {
				set_error("all parameters must be vec3's");
				return false;
			}
			res->push_back(v);
		}
		return true;
	}
	
	void result_removed(ptlist *&res) {
		lists.remove(res);
		delete res;
	}
	
private:
	std::list<ptlist*> lists;
};

filter* _make_ptlist_filter_(scene *scn, filter_input *input) {
	return new ptlist_filter(input);
}
