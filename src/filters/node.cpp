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
class node_filter : public typed_map_filter<const sgnode*>, public sgnode_listener {
public:
	node_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input)
	: typed_map_filter<const sgnode*>(root, si, input), scn(scn)
	{}
	
	~node_filter() {
		map<sgnode*, node_info>::iterator i;
		for (i = nodes.begin(); i != nodes.end(); ++i) {
			i->first->unlisten(this);
		}
	}
	
	bool compute(const filter_param_set *params, bool adding, const sgnode *&res, bool &changed) {
		sgnode *newres;
		string id;
		
		if (!get_filter_param(this, params, "id", id)) {
			set_status("expecting parameter id");
			return false;
		}
		if ((newres = scn->get_node(id)) == NULL) {
			stringstream ss;
			ss << "no node with id \"" << id << "\"";
			set_status(ss.str());
			return false;
		}
		
		if (newres != res) {
			add_entry(newres, params);
			if (!adding) {
				del_entry(const_cast<sgnode*>(res), params);
			}
			res = newres;
			changed = true;
		} else {
			// report whether the node itself changed since the last time
			node_info *info = map_get(nodes, const_cast<sgnode*>(res));
			assert(info);
			changed = info->changed;
			info->changed = false;
		}
		return true;
	}
	
	void add_entry(sgnode *n, const filter_param_set *params) {
		map<sgnode*, node_info>::iterator i = nodes.find(n);
		if (i == nodes.end()) {
			n->listen(this);
		}
		nodes[n].params.push_back(params);
	}
	
	void del_entry(sgnode *n, const filter_param_set *params) {
		map<sgnode*, node_info>::iterator i = nodes.find(n);
		assert(i != nodes.end());
		std::list<const filter_param_set*> &p = i->second.params;
		std::list<const filter_param_set*>::iterator j = find(p.begin(), p.end(), params);
		assert(j != p.end());
		p.erase(j);
		if (p.empty()) {
			i->first->unlisten(this);
			nodes.erase(i);
		}
	}
	
	void node_update(sgnode *n, sgnode::change_type t, int added) {
		if (t == sgnode::DELETED || t == sgnode::TRANSFORM_CHANGED || t == sgnode::SHAPE_CHANGED) {
			node_info *info = map_get(nodes, n);
			assert(info);
			std::list<const filter_param_set*>::const_iterator i;
			for (i = info->params.begin(); i != info->params.end(); ++i) {
				mark_stale(*i);
			}
			info->changed = true;
			if (t == sgnode::DELETED) {
				nodes.erase(n);
			}
		}
	}

private:
	struct node_info {
		std::list<const filter_param_set*> params;
		bool changed;
	};
	
	scene *scn;
	map<sgnode*, node_info> nodes;
};

/* Return all nodes from the scene */
class all_nodes_filter : public filter, public sgnode_listener {
public:
	all_nodes_filter(Symbol *root, soar_interface *si, scene *scn) 
	: filter(root, si, NULL), scn(scn), first(true) {}
	
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
		group_node *g;
		switch (t) {
			case sgnode::CHILD_ADDED:
				g = n->as_group();
				add_node(g->get_child(added_child));
				break;
			case sgnode::DELETED:
				if (!map_get(results, n, r)) {
					assert(false);
				}
				remove_result(r);
				results.erase(n);
				break;
			case sgnode::TRANSFORM_CHANGED:
			case sgnode::SHAPE_CHANGED:
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

class node_centroid_filter : public typed_map_filter<vec3> {
public:
	node_centroid_filter(Symbol *root, soar_interface *si, filter_input *input)
	: typed_map_filter<vec3>(root, si, input)
	{}
	
	bool compute(const filter_param_set *params, bool adding, vec3 &res, bool &changed) {
		const sgnode *n;
		
		if (!get_filter_param(this, params, "node", n)) {
			return false;
		}
		
		vec3 newres = n->get_centroid();
		changed = (newres != res);
		res = newres;
		return true;
	}
};

/*
 Generate a new node. Memory management is a little tricky. First note that
 although the filter result list owns the memory of the filter_val objects that
 wrap the generated nodes, it doesn't own the memory of the actual nodes. Those
 are owned by the filter, and it is responsible in most cases for deallocating
 them when they are removed from the scene. There are 3 ways this can happen:

 1. The filter is deleted. Then all generated nodes are deallocated in the
    filter's destructor.

 2. The filter input associated with the node is deleted. Then the single node
    associated with the input is deallocated in "result_removed".

 3. The generated node's parent is deleted. In this case all the parent's
    children are recursively deallocated in the parent's destructor, so the
    filter should not try to deallocate it again. This case is handled in the
    node listener callback "node_update", where the deleted node is taken off
    the list of generated nodes. This case also needs to be handled when the
    filter tries to update the already deleted node. 
*/
class gen_node_filter : public typed_map_filter<sgnode*>, public sgnode_listener {
public:
	gen_node_filter(Symbol *root, soar_interface *si, filter_input *input)
	: typed_map_filter<sgnode*>(root, si, input) {}

	~gen_node_filter() {
		std::list<sgnode*>::iterator i;
		for (i = nodes.begin(); i != nodes.end(); ++i) {
			(**i).unlisten(this);
			delete *i;
		}
	}
	
	bool compute(const filter_param_set *params, bool adding, sgnode *&res, bool &changed) {
		string id;
		vec3 pos, rot, scale, singlept;
		ptlist *pts = NULL;
		double radius;
		
		if (!adding) {
			if (find(nodes.begin(), nodes.end(), res) == nodes.end()) {
				// See case 3 in the class comment above
				adding = true;
				changed = true;
			}
		}
		
		if (adding && !get_filter_param(NULL, params, "id", id)) {
			set_status("no id");
			return false;
		}
		
		if (get_filter_param(NULL, params, "points", pts)) {
			if (adding) {
				res = new convex_node(id, *pts);
			} else {
				convex_node *c = dynamic_cast<convex_node*>(res);
				if (!c) {
					set_status("not a convex node");
					return false;
				}
				if (c->get_local_points() != *pts) {
					c->set_local_points(*pts);
					changed = true;
				}
			}
		} else if (get_filter_param(this, params, "points", singlept)) {
			ptlist l;
			l.push_back(singlept);
			if (adding) {
				res = new convex_node(id, l);
			} else {
				convex_node *c = dynamic_cast<convex_node*>(res);
				if (!c) {
					set_status("not a convex node");
					return false;
				}
				if (c->get_local_points() != l) {
					c->set_local_points(l);
					changed = true;
				}
			}
		} else if (get_filter_param(this, params, "radius", radius)) {
			if (adding) {
				res = new ball_node(id, radius);
			} else {
				ball_node *b = dynamic_cast<ball_node*>(res);
				if (!b) {
					set_status("not a ball node");
					return false;
				}
				if (b->get_radius() != radius) {
					b->set_radius(radius);
					changed = true;
				}
			}
		} else if (adding) {
			res = new group_node(id);
		}
		
		if (adding) {
			nodes.push_back(res);
			res->listen(this);
		}
		
		if (!get_filter_param(NULL, params, "pos", pos)) {
			pos = vec3::Zero();
		}
		if (!get_filter_param(NULL, params, "rot", rot)) {
			rot = vec3::Zero();
		}
		if (!get_filter_param(NULL, params, "scale", scale)) {
			scale = vec3::Constant(1.0);
		}
		
		if (res->get_trans('p') != pos) {
			res->set_trans('p', pos);
			changed = true;
		}
		if (res->get_trans('r') != rot) {
			res->set_trans('r', rot);
			changed = true;
		}
		if (res->get_trans('s') != scale) {
			res->set_trans('s', scale);
			changed = true;
		}
		
		return true;
	}

	void result_removed(const sgnode *&res) {
		remove_node(res);
		delete res;
	}
	
	void node_update(sgnode *n, sgnode::change_type t, int added) {
		if (t == sgnode::DELETED) {
			remove_node(n);
		}
	}
	
	void remove_node(const sgnode *n) {
		std::list<sgnode*>::iterator i = find(nodes.begin(), nodes.end(), n);
		assert(i != nodes.end());
		(**i).unlisten(this);
		nodes.erase(i);
	}
	
private:
	std::list<sgnode*> nodes;
};

filter *make_node_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new node_filter(root, si, scn, input);
}

filter *make_all_nodes_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new all_nodes_filter(root, si, scn);
}

filter *make_node_centroid_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new node_centroid_filter(root, si, input);
}
filter* _make_gen_node_filter_(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new gen_node_filter(root, si, input);
}

filter_table_entry node_fill_entry() {
	filter_table_entry e;
	e.name = "node";
	e.parameters.push_back("id");
	e.create = &make_node_filter;
	return e;
}

filter_table_entry all_nodes_fill_entry() {
	filter_table_entry e;
	e.name = "all_nodes";
	e.create = &make_all_nodes_filter;
	return e;
}

filter_table_entry node_centroid_fill_entry() {
	filter_table_entry e;
	e.name = "node_centroid";
	e.parameters.push_back("node");
	e.create = &make_node_centroid_filter;
	return e;
}

filter_table_entry gen_node_fill_entry() {
	filter_table_entry e;
	e.name = "gen_node";
	e.create = &_make_gen_node_filter_;
	return e;
}
