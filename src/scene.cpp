#include <cstdlib>
#include <map>
#include <iterator>
#include <iostream>
#include <sstream>
#include <limits>
#include <utility>
#include "scene.h"
#include "sgnode.h"
#include "common.h"
#include "drawer.h"
#include "filter.h"
#include "filter_table.h"

using namespace std;

const string root_name = "world";

/*
 Making this a global variable insures that all nodes in all scenes
 will have unique identifiers.
*/
int node_counter = 100;

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

bool parse_vec3(vector<string> &f, int &start, vec3 &v, string &error) {
	for (int i = 0; i < 3; ++start, ++i) {
		if (start >= f.size() || !parse_double(f[start], v[i])) {
			error = "expecting a number";
			return false;
		}
	}
	return true;
}

bool parse_verts(vector<string> &f, int &start, ptlist &verts, string &error) {
	verts.clear();
	while (start < f.size()) {
		vec3 v;
		int i = start;
		if (!parse_vec3(f, start, v, error)) {
			return (i == start);  // end of list
		}
		verts.push_back(v);
	}
	return true;
}

bool parse_inds(vector<string> &f, int &start, vector<int> &inds, string &error) {
	int i;
	while (parse_int(f[start], i)) {
		inds.push_back(i);
		++start;
	}
	return true;
}

bool parse_transforms(vector<string> &f, int &start, vec3 &pos, vec3 &rot, vec3 &scale, string &error) {	
	vec3 t;
	char type;
	
	if (f[start] != "p" && f[start] != "r" && f[start] != "s") {
		error = "expecting p, r, or s";
		return false;
	}
	type = f[start++][0];
	if (!parse_vec3(f, start, t, error)) {
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
	return true;
}

scene::scene(const string &name, drawer *d) 
: name(name), draw(d)
{
	root = new group_node(root_name, "world");
	root_id = node_counter++;
	nodes[root_id].node = root;
	root->listen(this);
	node_ids[root_name] = root_id;
	dirty = true;
	sig_dirty = true;
}

scene::~scene() {
	delete root;
}

scene *scene::clone(const string &cname, drawer *d) const {
	scene *c = new scene(cname, d);
	string name;
	std::vector<sgnode*> all_nodes;
	std::vector<sgnode*>::const_iterator i;
	
	c->root = root->clone()->as_group();
	c->root->walk(all_nodes);
	
	for(int i = 1; i < all_nodes.size(); ++i) {  // i = 0 is world, already in copy
		sgnode *n = all_nodes[i];
		const node_info *info = get_node_info(n->get_name());
		int id = node_counter++;
		c->node_ids[n->get_name()] = id;
		node_info &cinfo = c->nodes[id];
		cinfo.node = n;
		cinfo.props = info->props;
		n->listen(c);
		if (!n->is_group()) {
			c->cdetect.add_node(n);
			c->update_closest(n);
		}
		if (c->draw) {
			c->draw->add(c->name, n);
		}
	}
	
	return c;
}

scene::node_info *scene::get_node_info(int i) {
	node_info *info = map_getp(nodes, i);
	return info;
}

const scene::node_info *scene::get_node_info(int i) const {
	const node_info *info = map_getp(nodes, i);
	return info;
}

scene::node_info *scene::get_node_info(const string &name) {
	int i;
	if (!map_get(node_ids, name, i)) {
		return NULL;
	}
	return get_node_info(i);
}

const scene::node_info *scene::get_node_info(const string &name) const {
	int i;
	if (!map_get(node_ids, name, i)) {
		return NULL;
	}
	return get_node_info(i);
}

sgnode *scene::get_node(const string &name) {
	node_info *info = get_node_info(name);
	if (!info) {
		return NULL;
	}
	return info->node;
}

sgnode const *scene::get_node(const string &name) const {
	const node_info *info = get_node_info(name);
	if (!info) {
		return NULL;
	}
	return info->node;
}

sgnode *scene::get_node(int i) {
	node_info *info = get_node_info(i);
	if (!info) {
		return NULL;
	}
	return info->node;
}

const sgnode *scene::get_node(int i) const {
	const node_info *info = get_node_info(i);
	if (!info) {
		return NULL;
	}
	return info->node;
}

group_node *scene::get_group(const string &name) {
	sgnode *n = get_node(name);
	if (n) {
		return n->as_group();
	}
	return NULL;
}

void scene::get_all_nodes(vector<sgnode*> &n) {
	node_table::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		if (i->first != root_id) {
			n.push_back(i->second.node);
		}
	}
}

void scene::get_all_nodes(vector<const sgnode*> &n) const {
	n.reserve(nodes.size());
	node_table::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		n.push_back(i->second.node);
	}
}

void scene::get_all_node_indices(vector<int> &inds) const {
	node_table::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		inds.push_back(i->first);
	}
}

void scene::get_nodes(const vector<int> &inds, vector<const sgnode*> &n) const {
	n.resize(inds.size(), NULL);
	for (int i = 0; i < inds.size(); ++i) {
		n[i] = map_get(nodes, inds[i]).node;
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
	delete get_node_info(name)->node;
	/* rest is handled in node_update */
	return true;
}

void scene::clear() {
	for (int i = 0; i < root->num_children(); ++i) {
		delete root->get_child(i);
	}
}

enum node_class { CONVEX, BALL, GROUP };

int scene::parse_add(vector<string> &f, string &error) {
	sgnode *n = NULL;
	group_node *par = NULL;
	vec3 pos = vec3::Zero(), rot = vec3::Zero(), scale = vec3::Constant(1.0);

	if (f.size() < 2) {
		return f.size();
	}
	string name = f[0], type = f[1];
	if (get_node(name)) {
		error = "node already exists";
		return 0;
	}
	par = get_group(f[2]);
	if (!par) {
		error = "parent node does not exist";
		return 1;
	}
	
	int p = 3;
	ptlist verts;
	vector<int> indexes;
	double radius;
	node_class c = GROUP;
	while (p < f.size()) {
		if (f[p] == "v") {
			if (!parse_verts(f, ++p, verts, error)) {
				return p;
			}
			c = CONVEX;
		} else if (f[p] == "i") {
			if (!parse_inds(f, ++p, indexes, error)) {
				return p;
			}
			c = CONVEX;
		} else if (f[p] == "b") {
			++p;
			if (p >= f.size() || !parse_double(f[p], radius)) {
				error = "invalid radius";
				return p;
			}
			c = BALL;
			++p;
		} else if (!parse_transforms(f, p, pos, rot, scale, error)) {
			return p;
		}
	}
	
	switch (c) {
		case GROUP:
			n = new group_node(name, type);
			break;
		case CONVEX:
			n = new convex_node(name, type, verts, indexes);
			break;
		case BALL:
			n = new ball_node(name, type, radius);
			break;
		default:
			assert(false);
	}
	
	n->set_trans(pos, rot, scale);
	par->attach_child(n);
	return -1;
}

int scene::parse_del(vector<string> &f, string &error) {
	if (f.size() < 1) {
		error = "expecting node name";
		return f.size();
	}
	if (!del_node(f[0])) {
		error = "node does not exist";
		return 0;
	}
	return -1;
}

int scene::parse_change(vector<string> &f, string &error) {
	int p;
	sgnode *n;
	vec3 pos, rot, scale;

	if (f.size() < 1) {
		error = "expecting node name";
		return f.size();
	}
	if (!(n = get_node(f[0]))) {
		error = "node does not exist";
		return 0;
	}
	n->get_trans(pos, rot, scale);
	p = 1;
	while (p < f.size()) {
		if (!parse_transforms(f, p, pos, rot, scale, error)) {
			return p;
		}
	}
	n->set_trans(pos, rot, scale);
	return -1;
}

int scene::parse_property(vector<string> &f, string &error) {
	int i, p = 0;
	if (p >= f.size()) {
		error = "expecting node name";
		return p;
	}
	if (!map_get(node_ids, f[p++], i) || !nodes[i].node) {
		error = "node does not exist";
		return p;
	}
	if (p >= f.size()) {
		error = "expecting property name";
		return p;
	}
	string prop = f[p++];
	
	double val;
	if (!parse_double(f[p], val)) {
		error = "expecting a number";
		return p;
	}
	nodes[i].props[prop] = val;
	return -1;
}

void scene::parse_sgel(const string &s) {
	vector<string> lines, fields;
	vector<string>::iterator i;
	char cmd;
	int errfield;
	string error;
	
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
		error = "unknown error";
		
		switch(cmd) {
			case 'a':
				errfield = parse_add(fields, error);
				break;
			case 'd':
				errfield = parse_del(fields, error);
				break;
			case 'c':
				errfield = parse_change(fields, error);
				break;
			case 'p':
				errfield = parse_property(fields, error);
				break;
			default:
				cerr << "expecting a|d|c|p|t at beginning of line '" << *i << "'" << endl;
				exit(1);
		}
		
		if (errfield >= 0) {
			cerr << "error in field " << errfield + 1 << " of line '" << *i << "': " << error << endl;
			exit(1);
		}
	}
}

void scene::get_property_names(int i, vector<string> &names) const {
	const node_info *info = get_node_info(i);
	assert(info);
	
	for (int j = 0; j < NUM_NATIVE_PROPS; ++j) {
		names.push_back(NATIVE_PROPS[j]);
	}
	
	property_map::const_iterator k = info->props.begin();
	property_map::const_iterator end = info->props.end();
	for (; k != end; ++k) {
		names.push_back(k->first);
	}
}

void scene::get_properties(rvec &vals) const {
	node_table::const_iterator i;
	property_map::const_iterator j;
	int k = 0;
	
	vals.resize(get_dof());
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		const node_info &info = i->second;
		for (const char *t = "prs"; *t != '\0'; ++t) {
			vec3 trans = info.node->get_trans(*t);
			vals.segment(k, 3) = trans;
			k += 3;
		}
		for (j = info.props.begin(); j != info.props.end(); ++j) {
			vals[k++] = j->second;
		}
	}
}

bool scene::get_property(const string &name, const string &prop, double &val) const {
	property_map::const_iterator j;
	char type; int d;

	const node_info *info = get_node_info(name);
	if (is_native_prop(prop, type, d)) {
		val = info->node->get_trans(type)[d];
	} else {
		if ((j = info->props.find(prop)) == info->props.end()) {
			return false;
		}
		val = j->second;
	}
	return true;
}

bool scene::set_property(const string &obj, const string &prop, double val) {
	char type; int d;
	node_info *info = get_node_info(name);
	assert(info);
	if (is_native_prop(prop, type, d)) {
		vec3 trans = info->node->get_trans(type);
		trans[d] = val;
		info->node->set_trans(type, trans);
	} else {
		info->props[prop] = val;
	}
	return true;
}

bool scene::set_properties(const rvec &vals) {
	node_table::iterator i;
	const char *types = "prs";
	vec3 trans;
	
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		node_info &info = i->second;
		int k1, k2, l = 0;
		for (k1 = 0; k1 < 3; ++k1) {
			for (k2 = 0; k2 < 3; ++k2) {
				trans[k2] = vals[l++];
				if (l >= vals.size()) {
					return false;
				}
			}
			info.node->set_trans(types[k1], trans);
		}
		
		property_map::iterator j;
		for (j = info.props.begin(); j != info.props.end(); ++j) {
			j->second = vals[l++];
			if (l >= vals.size()) {
				return false;
			}
		}
	}
	return true;
}

void scene::remove_property(const std::string &name, const std::string &prop) {
	node_info *info = get_node_info(name);
	property_map::iterator j = info->props.find(prop);
	assert(j != info->props.end());
	info->props.erase(j);
}

int scene::num_nodes() const {
	return nodes.size();
}

int scene::get_dof() const {
	int dof = 0;
	node_table::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		dof += NUM_NATIVE_PROPS + i->second.props.size();
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
			i = node_counter++;
			node_ids[child->get_name()] = i;
			nodes[i].node = child;
			sig_dirty = true;
			if (!child->is_group()) {
				cdetect.add_node(child);
				update_closest(child);
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
			nodes.erase(i);
			node_ids.erase(n->get_name());
			sig_dirty = true;
			if (draw && n->get_name() != "world") {
				draw->del(name, n);
			}
			break;
		case sgnode::SHAPE_CHANGED:
			if (!n->is_group()) {
				cdetect.update_shape(n);
				update_closest(n);
				if (draw) {
					draw->change(name, n, drawer::SHAPE);
				}
			}
			break;
		case sgnode::TRANSFORM_CHANGED:
			if (!n->is_group()) {
				cdetect.update_transform(n);
				update_closest(n);
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

void scene::update_closest(const sgnode *n) {
	const geometry_node *g1 = dynamic_cast<const geometry_node*>(n);
	node_table::const_iterator i, j, end;
	
	for (i = nodes.begin(), end = nodes.end(); i != end; ++i) {
		const sgnode *n2 = i->second.node;
		if (n == n2 || n2->is_group())
			continue;
		
		const geometry_node *g2 = dynamic_cast<const geometry_node*>(n2);
		double dist = convex_distance(g1, g2);
		distances[make_pair(n, n2)] = dist;
		distances[make_pair(n2, n)] = dist;
	}
	
	for (i = nodes.begin(), end = nodes.end(); i != end; ++i) {
		const sgnode *n1 = i->second.node;
		if (n1->is_group())
			continue;
		
		close_info &c = closest[n1];
		c.id = -1;
		for (j = nodes.begin(); j != end; ++j) {
			const sgnode *n2 = j->second.node;
			if (n2->is_group())
				continue;
			
			double dist = distances[make_pair(n1, n2)];
			if (n1 != n2 && (c.id < 0 || c.dist > dist)) {
				c.id = j->first;
				c.dist = dist;
			}
		}
	}
}

int scene::get_closest(int i) const {
	const sgnode *n = map_get(nodes, i).node;
	assert(has(closest, n));
	return map_get(closest, n).id;
}

void scene::update_sig() const {
	sig.clear();
	node_table::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		scene_sig::entry e;
		e.id = i->first;
		e.name = i->second.node->get_name();
		e.type = i->second.node->get_type();
		get_property_names(i->first, e.props);
		sig.add(e);
	}
}

const scene_sig &scene::get_signature() const {
	if (sig_dirty) {
		update_sig();
	}
	return sig;
}

