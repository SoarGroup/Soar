#include <iostream>
#include <assert.h>
#include <string>
#include <map>
#include "filter.h"
#include "sgnode.h"
#include "scene.h"
#include "filter_table.h"

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
	
	bool compute(const filter_params *params, bool adding, const sgnode *&res, bool &changed) {
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
			node_info &info = map_get(nodes, const_cast<sgnode*>(res));
			changed = info.changed;
			info.changed = false;
		}
		return true;
	}
	
	void node_update(sgnode *n, sgnode::change_type t, const std::string& update_info) {
		if (t == sgnode::DELETED || t == sgnode::TRANSFORM_CHANGED || t == sgnode::SHAPE_CHANGED) {
			node_info &info = map_get(nodes, n);
			std::list<const filter_params*>::const_iterator i;
			for (i = info.params.begin(); i != info.params.end(); ++i) {
				mark_stale(*i);
			}
			info.changed = true;
			if (t == sgnode::DELETED) {
				nodes.erase(n);
			}
		}
	}

private:
	void add_entry(sgnode *n, const filter_params *params) {
		map<sgnode*, node_info>::iterator i = nodes.find(n);
		if (i == nodes.end()) {
			n->listen(this);
		}
		nodes[n].params.push_back(params);
	}
	
	void del_entry(sgnode *n, const filter_params *params) {
		map<sgnode*, node_info>::iterator i = nodes.find(n);
		assert(i != nodes.end());
		std::list<const filter_params*> &p = i->second.params;
		std::list<const filter_params*>::iterator j = find(p.begin(), p.end(), params);
		assert(j != p.end());
		p.erase(j);
		if (p.empty()) {
			i->first->unlisten(this);
			nodes.erase(i);
		}
	}
	
	struct node_info {
		std::list<const filter_params*> params;
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
		for (i = outputs.begin(); i != outputs.end(); ++i) {
			i->first->unlisten(this);
		}

		vector<sgnode*> nodes;
		scn->get_all_nodes(nodes);
		nodes[0]->unlisten(this);
	}
	
	bool update_outputs() {
		vector<sgnode*> nodes;
		
		if (!first) {
			return true;
		}
		
		scn->get_all_nodes(nodes);
		nodes[0]->listen(this);
		
		for (int i=1, iend=nodes.size(); i < iend; ++i) { // don't add world node
			add_node(nodes[i]);
		}
		first = false;
		return true;
	}
	
	void node_update(sgnode *n, sgnode::change_type t, const std::string& update_info) {
		filter_val *r;
		group_node *g;
		int added_child = 0;
		switch (t) {
			case sgnode::CHILD_ADDED:
				if(parse_int(update_info, added_child)){
					g = n->as_group();
					add_node(g->get_child(added_child));
				}
				break;
			case sgnode::DELETED:
				if (map_get(outputs, n, r)) {
					remove_output(r);
					outputs.erase(n);
				} else {
					assert(n->get_name() == "world");
				}
				break;
			case sgnode::TRANSFORM_CHANGED:
			case sgnode::SHAPE_CHANGED:
				if (map_get(outputs, n, r)) {
					change_output(r);
				} else {
					assert(n->get_name() == "world");
				}
				break;
		}
	}
	
private:
	filter_val *add_node(sgnode *n) {
		n->listen(this);
		filter_val *r = new filter_val_c<const sgnode*>(n);
		outputs[n] = r;
		add_output(r, NULL);
		return r;
	}
	
	scene *scn;
	bool first;
	
	map<sgnode*, filter_val*> outputs;
};

class node_centroid_filter : public typed_map_filter<vec3> {
public:
	node_centroid_filter(Symbol *root, soar_interface *si, filter_input *input)
	: typed_map_filter<vec3>(root, si, input)
	{}
	
	bool compute(const filter_params *params, bool adding, vec3 &res, bool &changed) {
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

filter *make_node_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new node_filter(root, si, scn, input);
}

filter *make_all_nodes_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new all_nodes_filter(root, si, scn);
}

filter *make_node_centroid_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new node_centroid_filter(root, si, input);
}

filter_table_entry *node_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "node";
	e->parameters.push_back("id");
	e->create = &make_node_filter;
	return e;
}

filter_table_entry *all_nodes_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "all_nodes";
	e->create = &make_all_nodes_filter;
	return e;
}

filter_table_entry *node_centroid_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "node_centroid";
	e->parameters.push_back("node");
	e->create = &make_node_centroid_filter;
	return e;
}
