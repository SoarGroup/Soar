#include <iostream>
#include <assert.h>
#include <string>
#include <map>
#include "filter.h"
#include "sgnode.h"
#include "scene.h"

using namespace std;

/*
 This filter takes a "name" parameter and outputs a pointer to the node
 with that name in the scene graph.
*/
class node_filter : public map_filter<const sgnode*>, public sgnode_listener {
public:
	node_filter(scene *scn, filter_input *input) : map_filter<const sgnode*>(input), scn(scn) {}
	
	~node_filter() {
		map<sgnode*, const filter_param_set*>::iterator i;
		for (i = node2param.begin(); i != node2param.end(); ++i) {
			i->first->unlisten(this);
		}
	}
	
	bool compute(const filter_param_set *params, const sgnode *&n, bool adding) {
		sgnode *n1;
		string name;
		if (!adding) {
			n1 = const_cast<sgnode*>(n);
			map<sgnode*, const filter_param_set*>::iterator i = node2param.find(n1);
			assert(i != node2param.end());
			i->first->unlisten(this);
			node2param.erase(i);
		}
		
		if (!get_filter_param(this, params, "name", name)) {
			return false;
		}
		if ((n1 = scn->get_node(name)) == NULL) {
			stringstream ss;
			ss << "no node called \"" << name << "\"";
			set_error(ss.str());
			return false;
		}
		
		n1->listen(this);
		node2param[n1] = params;
		n = n1;
		return true;
	}
	
	void node_update(sgnode *n, sgnode::change_type t, int added) {
		if (t == sgnode::DELETED || t == sgnode::TRANSFORM_CHANGED || t == sgnode::POINTS_CHANGED) {
			const filter_param_set *s;
			if (!map_get(node2param, n, s)) {
				assert(false);
			}
			mark_stale(s);
		}
	}

private:
	scene *scn;
	map<sgnode*, const filter_param_set*> node2param;
};

/* Return all nodes from the scene */
class all_nodes_filter : public filter, public sgnode_listener {
public:
	all_nodes_filter(scene *scn) : scn(scn), first(true) {}
	
	~all_nodes_filter() {
		map<sgnode*, filter_val*>::iterator i;
		for (i = results.begin(); i != results.end(); ++i) {
			i->first->unlisten(this);
		}
	}
	
	bool update_results() {
		vector<sgnode*> nodes;
		vector<sgnode*>::iterator i;
		
		if (!first) {
			return true;
		}
		
		scn->get_all_nodes(nodes);
		for (i = nodes.begin(); i != nodes.end(); ++i) {
			add_node(*i);
		}
		first = false;
		return true;
	}
	
	void node_update(sgnode *n, sgnode::change_type t, int added_child) {
		filter_val *r;
		switch (t) {
			case sgnode::CHILD_ADDED:
				add_node(n->get_child(added_child));
				break;
			case sgnode::DELETED:
				if (!map_get(results, n, r)) {
					assert(false);
				}
				remove_result(r);
				break;
			case sgnode::TRANSFORM_CHANGED:
			case sgnode::POINTS_CHANGED:
				if (!map_get(results, n, r)) {
					assert(false);
				}
				change_result(r);
				break;
		}
	}
	
private:
	filter_val *add_node(sgnode *n) {
		n->listen(this);
		filter_val *r = new filter_val_c<const sgnode*>(n);
		results[n] = r;
		add_result(r, NULL);
		return r;
	}
	
	scene *scn;
	bool first;
	
	map<sgnode*, filter_val*> results;
};

class node_centroid_filter : public map_filter<vec3> {
public:
	node_centroid_filter(filter_input *input) : map_filter<vec3>(input) {}
	
	bool compute(const filter_param_set *params, vec3 &v, bool adding) {
		const sgnode *n;
		ptlist pts;
		
		if (!get_filter_param(this, params, "node", n)) {
			return false;
		}
		
		n->get_world_points(pts);
		v = calc_centroid(pts);
		return true;
	}
};

filter *make_node_filter(scene *scn, filter_input *input) {
	return new node_filter(scn, input);
}

filter *make_all_nodes_filter(scene *scn, filter_input *input) {
	return new all_nodes_filter(scn);
}

filter *make_node_centroid_filter(scene *scn, filter_input *input) {
	return new node_centroid_filter(input);
}

filter_table_entry node_fill_entry() {
	filter_table_entry e;
	e.name = "node";
	e.parameters.push_back("name");
	e.create = &make_node_filter;
	e.calc = NULL;
	e.possible_args = NULL;
	return e;
}

filter_table_entry all_nodes_fill_entry() {
	filter_table_entry e;
	e.name = "all_nodes";
	e.create = &make_all_nodes_filter;
	e.calc = NULL;
	e.possible_args = NULL;
	return e;
}

filter_table_entry node_centroid_fill_entry() {
	filter_table_entry e;
	e.name = "node_centroid";
	e.parameters.push_back("name");
	e.create = &make_node_centroid_filter;
	e.calc = NULL;
	e.possible_args = NULL;
	return e;
}
