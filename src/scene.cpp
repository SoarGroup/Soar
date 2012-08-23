#include <stdlib.h>
#include <map>
#include <iterator>
#include <iostream>
#include <sstream>
#include <limits>
#include <utility>
#include "scene.h"
#include "sgnode.h"
#include "linalg.h"
#include "common.h"
#include "drawer.h"
#include "filter.h"
#include "filter_table.h"

using namespace std;

/*
 Native properties are currently the position, rotation, and scaling
 transforms of a node, named px, py, pz, rx, ry, rz, sx, sy, sz.
*/
const int NUM_NATIVE_PROPS = 9;
const char *NATIVE_PROPS[] = { "px", "py", "pz", "rx", "ry", "rz", "sx", "sy", "sz" };

bool is_native_prop(const string &name, char &type, int &dim) {
	int d;
	if (name.size() != 2) {
		return false;
	}
	if (name[0] != 'p' && name[0] != 'r' && name[0] != 's') {
		return false;
	}
	d = name[1] - 'x';
	if (d < 0 || d > 2) {
		return false;
	}
	type = name[0];
	dim = d;
	return true;
}

scene::scene(const string &name, drawer *d) 
: name(name), draw(d), dirty(true), nodes(1)
{
	root = new group_node("world");
	nodes[0].node = root;
	root->listen(this);
	node_ids["world"] = 0;
}

scene::~scene() {
	delete root;
}

scene *scene::clone() const {
	scene *c = new scene(name, NULL);  // don't display copies
	string name;
	std::vector<sgnode*> all_nodes;
	std::vector<sgnode*>::const_iterator i;
	
	c->root = root->clone()->as_group();
	c->root->walk(all_nodes);
	
	/*
	 Make a deep copy of the nodes table, which will result in
	 a table with pointers to other's nodes, then go through and
	 change to point to our nodes.
	*/
	c->nodes = nodes;
	for(i = all_nodes.begin(); i != all_nodes.end(); ++i) {
		c->nodes[c->node_ids[(**i).get_name()]].node = *i;
		(**i).listen(c);
		if (!(**i).is_group()) {
			c->cdetect.add_node(*i);
		}
	}
	
	return c;
}

sgnode *scene::get_node(const string &name) {
	int i;
	if (!map_get(node_ids, name, i)) {
		return NULL;
	}
	return nodes[i].node;
}

sgnode const *scene::get_node(const string &name) const {
	int i;
	if (!map_get(node_ids, name, i)) {
		return NULL;
	}
	return nodes[i].node;
}

sgnode *scene::get_node(int i) {
	assert(0 <= i && i < nodes.size());
	return nodes[i].node;
}

const sgnode *scene::get_node(int i) const {
	assert(0 <= i && i < nodes.size());
	return nodes[i].node;
}

group_node *scene::get_group(const string &name) {
	sgnode *n = get_node(name);
	if (n) {
		return n->as_group();
	}
	return NULL;
}

void scene::get_all_nodes(vector<sgnode*> &n) {
	node_vec::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		if (i->node && i->node->get_name() != "world") {
			n.push_back(i->node);
		}
	}
}

void scene::get_all_nodes(vector<const sgnode*> &n) const {
	node_vec::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		if (i->node && i->node->get_name() != "world") {
			n.push_back(i->node);
		}
	}
}

void scene::get_all_node_indices(vector<int> &inds) const {
	for (int i = 1; i < nodes.size(); ++i) {  // don't include the world node
		if (nodes[i].node) {
			inds.push_back(i);
		}
	}
}

void scene::get_nodes(const vector<int> &inds, vector<const sgnode*> &n) const {
	n.resize(inds.size());
	for (int i = 0; i < inds.size(); ++i) {
		n[i] = nodes[inds[i]].node;
	}
}

bool scene::add_node(const string &name, sgnode *n) {
	group_node *par = get_group(name);
	if (!par) {
		return false;
	}
	par->attach_child(n);
	/* rest is handled in node_update */
	return true;
}

bool scene::del_node(const string &name) {
	int i;
	if (!map_get(node_ids, name, i)) {
		return false;
	}
	delete nodes[i].node;
	/* rest is handled in node_update */
	return true;
}

void scene::clear() {
	for (int i = 0; i < root->num_children(); ++i) {
		delete root->get_child(i);
	}
}

bool parse_vec3(vector<string> &f, int &start, int n, vec3 &v) {
	stringstream ss;
	if (start + n > f.size()) {
		start = f.size();
		return false;
	}
	for (int i = 0; i < n; ++start, ++i) {
		ss << f[start] << endl;
		if (!(ss >> v[i])) {  // conversion failure
			return false;
		}
	}
	return true;
}

bool parse_verts(vector<string> &f, int &start, ptlist &verts) {
	verts.clear();
	while (start < f.size()) {
		vec3 v;
		int i = start;
		if (!parse_vec3(f, start, 3, v)) {
			return (i == start);  // end of list
		}
		verts.push_back(v);
	}
	return true;
}

bool parse_transforms(vector<string> &f, int &start, vec3 &pos, vec3 &rot, vec3 &scale) {	
	vec3 t;
	char type;
	
	while (start < f.size()) {
		if (f[start] != "p" && f[start] != "r" && f[start] != "s") {
			return true;
		}
		type = f[start][0];
		start++;
		if (!parse_vec3(f, start, 3, t)) {
			return false;
		}
		switch (type) {
			case 'p':
				pos = t;
				break;
			case 'r':
				rot = t;
				break;
			case 's':
				scale = t;
				break;
			default:
				assert(false);
		}
	}
	return true;
}

int scene::parse_add(vector<string> &f) {
	int p;
	sgnode *n;
	group_node *par;
	vec3 pos = vec3::Zero(), rot = vec3::Zero(), scale = vec3::Constant(1.0);

	if (f.size() < 2) {
		return f.size();
	}
	if (get_node(f[0])) {
		return 0;  // already exists
	}
	par = get_group(f[1]);
	if (!par) {
		return 1;
	}
	
	if (f.size() >= 3 && f[2] == "v") {
		p = 3;
		ptlist verts;
		if (!parse_verts(f, p, verts)) {
			return p;
		}
		n = new convex_node(f[0], verts);
	} else if (f.size() >= 3 && f[2] == "b") {
		if (f.size() < 4) {
			return 4;
		}
		double radius;
		if (!parse_double(f[3], radius)) {
			return 4;
		}
		n = new ball_node(f[0], radius);
		p = 4;
	} else {
		n = new group_node(f[0]);
		p = 2;
	}
	
	if (!parse_transforms(f, p, pos, rot, scale)) {
		return p;
	}
	
	n->set_trans(pos, rot, scale);
	par->attach_child(n);
	return -1;
}

int scene::parse_del(vector<string> &f) {
	if (f.size() != 1) {
		return f.size();
	}
	if (!del_node(f[0])) {
		return 0;
	}
	return -1;
}

int scene::parse_change(vector<string> &f) {
	int p;
	sgnode *n;
	vec3 pos, rot, scale;

	if (f.size() < 1) {
		return f.size();
	}
	if (!(n = get_node(f[0]))) {
		return 0;
	}
	n->get_trans(pos, rot, scale);
	p = 1;
	if (!parse_transforms(f, p, pos, rot, scale)) {
		return p;
	}
	n->set_trans(pos, rot, scale);
	return -1;
}

int scene::parse_property(vector<string> &f) {
	int i;
	if (f.size() != 3) {
		return f.size();
	}
	stringstream ss(f[2]);
	float val;
	
	if (!(ss >> val)) {
		return 2;
	}
	if (!map_get(node_ids, f[0], i)) {
		return 0;
	}
	nodes[i].props[f[1]] = val;
	return -1;
}

void scene::parse_sgel(const string &s) {
	vector<string> lines, fields;
	vector<string>::iterator i;
	char cmd;
	int errfield;
	
	LOG(SGEL) << "received sgel" << endl << "---------" << endl << s << endl << "---------" << endl;
	split(s, "\n", lines);
	for (i = lines.begin(); i != lines.end(); ++i) {
		split(*i, " \t", fields);
		
		if (fields.size() == 0)
			continue;
		if (fields[0].size() != 1) {
			cerr << "expecting a|d|c|p|t at beginning of line '" << *i << "'" << endl;
			exit(1);
		}
		
		cmd = fields[0][0];
		fields.erase(fields.begin());
		
		switch(cmd) {
			case 'a':
				errfield = parse_add(fields);
				break;
			case 'd':
				errfield = parse_del(fields);
				break;
			case 'c':
				errfield = parse_change(fields);
				break;
			case 'p':
				errfield = parse_property(fields);
				break;
			default:
				cerr << "expecting a|d|c|p|t at beginning of line '" << *i << "'" << endl;
				exit(1);
		}
		
		if (errfield >= 0) {
			cerr << "error in field " << errfield + 1 << " of line '" << *i << "' " << endl;
			exit(1);
		}
	}
}

void scene::get_property_names(vector<string> &names) const {
	node_vec::const_iterator i;
	property_map::const_iterator j;
	int k;
	stringstream ss;
	
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		if (!i->node) {
			continue;
		}
		string name = i->node->get_name();
		for (k = 0; k < NUM_NATIVE_PROPS; ++k) {
			ss.str("");
			ss << name << ":" << NATIVE_PROPS[k];
			names.push_back(ss.str());
		}
		for (j = i->props.begin(); j != i->props.end(); ++j) {
			ss.str("");
			ss << name << ":" << j->first;
			names.push_back(ss.str());
		}
	}
}

void scene::get_properties(rvec &vals) const {
	node_vec::const_iterator i;
	property_map::const_iterator j;
	int k = 0;
	
	vals.resize(get_dof());
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		if (!i->node) {
			continue;
		}
		for (const char *t = "prs"; *t != '\0'; ++t) {
			vec3 trans = i->node->get_trans(*t);
			vals.segment(k, 3) = trans;
			k += 3;
		}
		for (j = i->props.begin(); j != i->props.end(); ++j) {
			vals[k++] = j->second;
		}
	}
}

bool scene::get_property(const string &name, const string &prop, float &val) const {
	int i;
	property_map::const_iterator j;
	char type; int d;

	if (!map_get(node_ids, name, i)) {
		return false;
	}
	if (is_native_prop(prop, type, d)) {
		val = nodes[i].node->get_trans(type)[d];
	} else {
		if ((j = nodes[i].props.find(prop)) == nodes[i].props.end()) {
			return false;
		}
		val = j->second;
	}
	return true;
}

bool scene::add_property(const string &name, const string &prop, float val) {
	int i;
	property_map::iterator j;
	char type; int d;
	if (!map_get(node_ids, name, i)) {
		return false;
	}
	if (is_native_prop(prop, type, d)) {
		return false;
	} else {
		if ((j = nodes[i].props.find(prop)) != nodes[i].props.end()) {
			return false;
		}
		nodes[i].props[prop] = val;
	}
	return true;
}

bool scene::set_property(const string &obj, const string &prop, float val) {
	int i;
	property_map::iterator j;
	char type; int d;
	if (!map_get(node_ids, name, i)) {
		return false;
	}
	if (is_native_prop(prop, type, d)) {
		vec3 trans = nodes[i].node->get_trans(type);
		trans[d] = val;
		nodes[i].node->set_trans(type, trans);
	} else {
		if ((j = nodes[i].props.find(prop)) == nodes[i].props.end()) {
			return false;
		}
		j->second = val;
	}
	return true;
}

bool scene::set_properties(const rvec &vals) {
	node_vec::iterator i;
	property_map::iterator j;
	int k1, k2, l = 0;
	const char *types = "prs";
	vec3 trans;
	
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		if (!i->node) {
			continue;
		}
		for (k1 = 0; k1 < 3; ++k1) {
			for (k2 = 0; k2 < 3; ++k2) {
				trans[k2] = vals[l++];
				if (l >= vals.size()) {
					return false;
				}
			}
			i->node->set_trans(types[k1], trans);
		}
		for (j = i->props.begin(); j != i->props.end(); ++j) {
			j->second = vals[l++];
			if (l >= vals.size()) {
				return false;
			}
		}
	}
	return true;
}

bool scene::remove_property(const std::string &name, const std::string &prop) {
	int i;
	property_map::iterator j;
	
	if (!map_get(node_ids, name, i)) {
		return false;
	}
	if ((j = nodes[i].props.find(prop)) == nodes[i].props.end()) {
		return false;
	}
	nodes[i].props.erase(j);
	return true;
}

int scene::num_nodes() const {
	int s = 0;
	for (int i = 0; i < nodes.size(); ++i) {
		if (nodes[i].node) {
			s++;
		}
	}
	return s;
}

int scene::get_dof() const {
	int dof = 0;
	node_vec::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		if (i->node) {
			dof += NUM_NATIVE_PROPS + i->props.size();
		}
	}
	return dof;
}

void scene::node_update(sgnode *n, sgnode::change_type t, int added_child) {
	sgnode *child;
	group_node *g;
	int i;
	
	switch (t) {
		case sgnode::CHILD_ADDED:
			g = n->as_group();
			child = g->get_child(added_child);
			child->listen(this);
			node_ids[child->get_name()] = nodes.size();
			nodes.push_back(node_info());
			nodes.back().node = child;
			if (!child->is_group()) {
				cdetect.add_node(child);
			}
			if (draw) {
				draw->add(name, child);
			}
			break;
		case sgnode::DELETED:
			if (!n->is_group()) {
				cdetect.del_node(n);
			}
			if (!map_get(node_ids, n->get_name(), i)) {
				assert(false);
			}
			nodes[i].node = NULL;
			nodes[i].props.clear();
			node_ids.erase(n->get_name());
			if (draw && n->get_name() != "world") {
				draw->del(name, n);
			}
			break;
		case sgnode::SHAPE_CHANGED:
			if (!n->is_group()) {
				cdetect.update_points(n);
				if (draw) {
					draw->change(name, n, drawer::SHAPE);
				}
			}
			break;
		case sgnode::TRANSFORM_CHANGED:
			if (!n->is_group()) {
				cdetect.update_transform(n);
			}
			if (draw) {
				draw->change(name, n, drawer::POS | drawer::ROT | drawer::SCALE);
			}
			break;
	}
	dirty = true;
}

bool scene::intersects(const sgnode *a, const sgnode *b) const {
	if (a == b) {
		return true;
	}
	const collision_table &c = const_cast<scene*>(this)->cdetect.get_collisions();
	return c.find(make_pair(a, b)) != c.end() || c.find(make_pair(b, a)) != c.end();
}

void scene::calc_relations(relation_table &rels) const {
	get_filter_table().update_relations(this, 0, rels);
}

void scene::print_relations(ostream &os) const {
	relation_table rels;
	relation_table::const_iterator i;
	get_filter_table().update_relations(this, 0, rels);
	for (i = rels.begin(); i != rels.end(); ++i) {
		set<tuple> args;
		set<tuple>::iterator j;
		i->second.drop_first(args);
		for (j = args.begin(); j != args.end(); ++j) {
			os << i->first << "(";
			for (int k = 0; k < j->size() - 1; ++k) {
				assert(nodes[(*j)[k]].node);
				os << nodes[(*j)[k]].node->get_name() << ",";
			}
			assert(nodes[j->back()].node);
			os << nodes[j->back()].node->get_name() << ")" << endl;
		}
	}
}
