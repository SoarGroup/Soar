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
: name(name), draw(d), dirty(true)
{
	root = new group_node("world");
	nodes["world"].node = root;
	root->listen(this);
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
	
	/*
	 Make a deep copy of the nodes table, which will result in
	 a table with pointers to other's nodes, then go through and
	 change to point to our nodes.
	*/
	c->nodes = nodes;
	for(i = all_nodes.begin(); i != all_nodes.end(); ++i) {
		string node_name = (**i).get_name();
		c->nodes[node_name].node = *i;
		(**i).listen(c);
		if (!(**i).is_group()) {
			c->cdetect.add_node(*i);
		}
		if (c->draw) {
			c->draw->add(c->name, *i);
		}
	}
	
	return c;
}

sgnode *scene::get_node(const string &name) {
	node_map::const_iterator i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return NULL;
	}
	return i->second.node;
}

group_node *scene::get_group(const string &name) {
	sgnode *n = get_node(name);
	if (n) {
		return n->as_group();
	}
	return NULL;
}

sgnode const *scene::get_node(const string &name) const {
	node_map::const_iterator i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return NULL;
	}
	return i->second.node;
}

void scene::get_all_nodes(vector<sgnode*> &n) {
	node_map::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		if (i->first != "world") {
			n.push_back(i->second.node);
		}
	}
}

void scene::get_all_nodes(vector<const sgnode*> &n) {
	node_map::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		if (i->first != "world") {
			n.push_back(i->second.node);
		}
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
	node_map::iterator i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return false;
	}
	delete i->second.node;
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

bool parse_mods(vector<string> &f, int &start, string &mods, vector<ptlist> &vals) {
	ptlist v;
	char m;
	while (start < f.size()) {
		if (f[start].size() != 1) {
			return true;
		}
		m = f[start][0];
		v.clear();
		switch (m) {
			case 'p':
			case 'r':
			case 's':
				v.push_back(vec3());
				if (!parse_vec3(f, ++start, 3, v[0])) {
					return false;
				}
				break;
			case 'v':
				if (!parse_verts(f, ++start, v)) {
					return false;
				}
				break;
			case 'b':
				++start;
				v.push_back(vec3());
				if (start >= f.size() || !parse_double(f[start], v[0](0))) {
					return false;
				}
				++start;
				break;
			default:
				// end of modifiers
				return true;
		}
		mods += m;
		vals.push_back(v);
	}
	return true;
}

int scene::parse_add(vector<string> &f) {
	int p;
	sgnode *n;
	group_node *par;
	string name, mods;
	vector<ptlist> vals;

	if (f.size() < 2) {
		return f.size();
	}
	
	name = f[0];
	if (get_node(name)) {
		return 0;  // already exists
	}
	par = get_group(f[1]);
	if (!par) {
		return 1;
	}
	
	p = 2;
	if (!parse_mods(f, p, mods, vals)) {
		return p;
	}
	assert(mods.size() == vals.size());
	
	/*
	 Go through once to figure out what type of node this should be
	*/
	n = NULL;
	for (int i = 0, iend = mods.size(); i < iend; ++i) {
		switch (mods[i]) {
			case 'v':
				n = new convex_node(name, vals[i]);
				break;
			case 'b':
				n = new ball_node(name, vals[i][0](0));
				break;
		}
	}
	if (!n) {
		n = new group_node(name);
	}
	
	/*
	 Go through again to apply transforms
	*/
	for (int i = 0, iend = mods.size(); i < iend; ++i) {
		switch (mods[i]) {
			case 'p':
			case 'r':
			case 's':
				n->set_trans(mods[i], vals[i][0]);
				break;
		}
	}
	
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
	convex_node *cn;
	ball_node *bn;
	string mods;
	vector<ptlist> vals;

	if (f.size() < 1) {
		return f.size();
	}
	if (!(n = get_node(f[0]))) {
		return 0;
	}
	
	p = 1;
	if (!parse_mods(f, p, mods, vals)) {
		return p;
	}
	
	for (int i = 0, iend = mods.size(); i < iend; ++i) {
		switch (mods[i]) {
			case 'p':
			case 'r':
			case 's':
				n->set_trans(mods[i], vals[i][0]);
				break;
			case 'v':
				cn = dynamic_cast<convex_node*>(n);
				if (!cn) {
					return 0; // maybe not as informative as it could be
				}
				cn->set_local_points(vals[i]);
				break;
			case 'b':
				bn = dynamic_cast<ball_node*>(n);
				if (!bn) {
					return 0;
				}
				bn->set_radius(vals[i][0](0));
				break;
		}
	}
	return -1;
}

int scene::parse_property(vector<string> &f) {
	node_map::iterator i;
	if (f.size() != 3) {
		return f.size();
	}
	stringstream ss(f[2]);
	float val;
	
	if (!(ss >> val)) {
		return 2;
	}
	if ((i = nodes.find(f[0])) == nodes.end()) {
		return 0;
	}
	i->second.props[f[1]] = val;
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
	node_map::const_iterator i;
	property_map::const_iterator j;
	int k;
	stringstream ss;
	
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		for (k = 0; k < NUM_NATIVE_PROPS; ++k) {
			ss.str("");
			ss << i->first << ":" << NATIVE_PROPS[k];
			names.push_back(ss.str());
		}
		for (j = i->second.props.begin(); j != i->second.props.end(); ++j) {
			ss.str("");
			ss << i->first << ":" << j->first;
			names.push_back(ss.str());
		}
	}
}

void scene::get_properties(rvec &vals) const {
	node_map::const_iterator i;
	property_map::const_iterator j;
	int k = 0;
	
	vals.resize(get_dof());
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		for (const char *t = "prs"; *t != '\0'; ++t) {
			vec3 trans = i->second.node->get_trans(*t);
			vals.segment(k, 3) = trans;
			k += 3;
		}
		for (j = i->second.props.begin(); j != i->second.props.end(); ++j) {
			vals[k++] = j->second;
		}
	}
}

bool scene::get_property(const string &obj, const string &prop, float &val) const {
	node_map::const_iterator i;
	property_map::const_iterator j;
	char type; int d;
	if ((i = nodes.find(obj)) == nodes.end()) {
		return false;
	}
	if (is_native_prop(prop, type, d)) {
		val = i->second.node->get_trans(type)[d];
	} else {
		if ((j = i->second.props.find(prop)) == i->second.props.end()) {
			return false;
		}
		val = j->second;
	}
	return true;
}

bool scene::set_property(const string &obj, const string &prop, float val) {
	node_map::iterator i;
	property_map::iterator j;
	char type; int d;
	if ((i = nodes.find(obj)) == nodes.end()) {
		return false;
	}
	if (is_native_prop(prop, type, d)) {
		vec3 trans = i->second.node->get_trans(type);
		trans[d] = val;
		i->second.node->set_trans(type, trans);
	} else {
		i->second.props[prop] = val;
	}
	return true;
}

bool scene::set_properties(const rvec &vals) {
	node_map::iterator i;
	property_map::iterator j;
	int k1, k2, l = 0;
	const char *types = "prs";
	vec3 trans;
	
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		for (k1 = 0; k1 < 3; ++k1) {
			for (k2 = 0; k2 < 3; ++k2) {
				trans[k2] = vals[l++];
				if (l >= vals.size()) {
					return false;
				}
			}
			i->second.node->set_trans(types[k1], trans);
		}
		for (j = i->second.props.begin(); j != i->second.props.end(); ++j) {
			j->second = vals[l++];
			if (l >= vals.size()) {
				return false;
			}
		}
	}
	return true;
}

bool scene::remove_property(const std::string &obj, const std::string &prop) {
	node_map::iterator i;
	property_map::iterator j;
	
	if ((i = nodes.find(obj)) == nodes.end()) {
		return false;
	}
	if ((j = i->second.props.find(prop)) == i->second.props.end()) {
		return false;
	}
	i->second.props.erase(j);
	return true;
}

int scene::num_nodes() const {
	return nodes.size();
}

int scene::get_dof() const {
	int dof = 0;
	node_map::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		dof += NUM_NATIVE_PROPS + i->second.props.size();
	}
	return dof;
}

void scene::node_update(sgnode *n, sgnode::change_type t, int added_child) {
	sgnode *child;
	group_node *g;
	switch (t) {
		case sgnode::CHILD_ADDED:
			g = n->as_group();
			child = g->get_child(added_child);
			child->listen(this);
			nodes[child->get_name()].node = child;
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
			nodes.erase(n->get_name());
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

void scene::dump_sgel(ostream &os) {
	vector<sgnode*> all_nodes;
	root->walk(all_nodes);
	
	for (int i = 0; i < all_nodes.size(); ++i) {
		sgnode *n = all_nodes[i];
		if (n->get_name() == "world") {
			continue;
		}
		assert(nodes.find(n->get_name()) != nodes.end());
		const node_info &info = nodes[name];
		const property_map &m = info.props;
		string shape_sgel;
		n->get_shape_sgel(shape_sgel);
		os << "a " << n->get_name() << " " << n->get_parent()->get_name() << " "
		   << shape_sgel << " "
		   << "p " << n->get_trans('p') << " "
		   << "r " << n->get_trans('r') << " "
		   << "s " << n->get_trans('s') << endl;

		property_map::const_iterator j;
		for (j = info.props.begin(); j != info.props.end(); ++j) {
			os << "p " << n->get_name() << " " << j->first << " " << j->second << endl;
		}
	}
}

const vector<bool>& scene::get_atom_vals() {
	if (dirty) {
		atomvals.clear();
		get_filter_table().calc_all_atoms(this, atomvals);
		dirty = false;
	}
	return atomvals;
}

bool scene::intersects(const string &a, const string &b) {
	sgnode *na = get_node(a), *nb = get_node(b);
	if (!na || !nb) {
		return false;
	}
	return intersects(na, nb);
}

bool scene::intersects(const sgnode *a, const sgnode *b) {
	if (a == b) {
		return true;
	}
	const collision_table &c = cdetect.get_collisions();
	return c.find(make_pair(a, b)) != c.end() || c.find(make_pair(b, a)) != c.end();
}

void scene::print_object_verts(std::ostream &os) const {
	node_map::const_iterator i, iend;
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		const convex_node *cn = dynamic_cast<convex_node*>(i->second.node);
		if (!cn) {
			continue;
		}
		os << i->first << endl;
		ptlist verts = cn->get_world_points();
		for (int j = 0, jend = verts.size(); j < jend; ++j) {
			os << '\t' << verts[j] << endl;
		}
	}
}
