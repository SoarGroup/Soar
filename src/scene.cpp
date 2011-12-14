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

scene::scene(const string &name, const string &rootname, bool display) 
: name(name), rootname(rootname), dt(1.0), display(display), draw(name), dirty(true)
{
	root = new sgnode(rootname);
	nodes[rootname].node = root;
	root->listen(this);
}

scene::~scene() {
	delete root;
}

scene *scene::copy() const {
	scene *copy = new scene(name, rootname, false);  // don't display copies
	string name;
	std::list<sgnode*> all_nodes;
	std::list<sgnode*>::const_iterator i;
	
	copy->root = root->copy();
	copy->root->walk(all_nodes);
	
	/*
	 Make a deep copy of the nodes table, which will result in
	 a table with pointers to other's nodes, then go through and
	 change to point to our nodes.
	*/
	copy->nodes = nodes;
	for(i = all_nodes.begin(); i != all_nodes.end(); ++i) {
		copy->nodes[(**i).get_name()].node = *i;
		(**i).listen(copy);
	}
	
	return copy;
}

sgnode* scene::get_node(const string &name) {
	node_map::const_iterator i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return NULL;
	}
	return i->second.node;
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

bool scene::add_node(const string &name, sgnode *n) {
	sgnode *par = get_node(name);
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

bool parse_n_floats(vector<string> &f, int &start, int n, float *x) {
	stringstream ss;
	if (start + n > f.size()) {
		start = f.size();
		return false;
	}
	for (int i = 0; i < n; ++start, ++i) {
		ss << f[start] << endl;
		if (!(ss >> x[i])) {  // conversion failure
			return false;
		}
	}
	return true;
}

bool parse_verts(vector<string> &f, int &start, ptlist &verts) {
	vec3 v;
	int i;
	if (start >= f.size() || f[start] != "v") {
		return true;
	}
	start++;
	verts.clear();
	while (start < f.size()) {
		i = start;
		if (!parse_n_floats(f, start, 3, v.a)) {
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
		if (!parse_n_floats(f, start, 3, t.a)) {
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
	ptlist verts;
	int p;
	sgnode *n, *par;
	vec3 pos, rot, scale(1., 1., 1.);

	if (f.size() < 2) {
		return f.size();
	}
	if (get_node(f[0])) {
		return 0;  // already exists
	}
	par = get_node(f[1]);
	if (!par || !par->is_group()) {
		return 1;
	}
	p = 2;
	if (!parse_verts(f, p, verts)) {
		return p;
	}
	if (!parse_transforms(f, p, pos, rot, scale)) {
		return p;
	}
	
	if (verts.size() == 0) {
		n = new sgnode(f[0]);
	} else {
		n = new sgnode(f[0], verts);
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

int scene::parse_dt(vector<string> &f) {
	if (f.size() != 1) {
		return f.size();
	}
	stringstream ss(f[0]);
	
	if (!(ss >> dt)) {
		return 1;
	}
	return -1;
}

void scene::parse_sgel(const string &s) {
	vector<string> lines, fields;
	vector<string>::iterator i;
	char cmd;
	int errfield;
	
	//cerr << "received sgel" << endl << "---------" << endl << s << endl << "---------" << endl;
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
			case 't':
				errfield = parse_dt(fields);
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

void scene::draw_all(const string &prefix, float r, float g, float b) {
	node_map::const_iterator i;
	draw.set_color(r, g, b);
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		sgnode *n = i->second.node;
		if (n->is_group()) {
			continue;
		}
		draw.set_transforms(n);
		draw.set_vertices(n);
		draw.add(prefix + n->get_name());
	}
}

void scene::undraw_all(const string &prefix) {
	node_map::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		sgnode *n = i->second.node;
		if (n->is_group()) {
			continue;
		}
		draw.del(prefix + n->get_name());
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

void scene::get_properties(floatvec &vals) const {
	node_map::const_iterator i;
	property_map::const_iterator j;
	int k1, k2, l = 0;
	const char *types = "prs";
	vec3 trans;
	
	vals.resize(get_dof());
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		for (k1 = 0; k1 < 3; ++k1) {
			trans = i->second.node->get_trans(types[k1]);
			for (k2 = 0; k2 < 3; ++k2) {
				vals[l++] = trans[k2];
			}
		}
		for (j = i->second.props.begin(); j != i->second.props.end(); ++j) {
			vals[l++] = j->second;
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

bool scene::add_property(const string &obj, const string &prop, float val) {
	node_map::iterator i;
	property_map::iterator j;
	char type; int d;
	if ((i = nodes.find(obj)) == nodes.end()) {
		return false;
	}
	if (is_native_prop(prop, type, d)) {
		return false;
	} else {
		if ((j = i->second.props.find(prop)) != i->second.props.end()) {
			return false;
		}
		i->second.props[prop] = val;
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
		if ((j = i->second.props.find(prop)) == i->second.props.end()) {
			return false;
		}
		j->second = val;
	}
	return true;
}

bool scene::set_properties(const floatvec &vals) {
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

float scene::get_dt() const {
	return dt;
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
	switch (t) {
		case sgnode::CHILD_ADDED:
			child = n->get_child(added_child);
			child->listen(this);
			nodes[child->get_name()].node = child;
			if (display && !child->is_group()) {
				draw.add(child);
			}
			break;
		case sgnode::DELETED:
			nodes.erase(n->get_name());
			if (display && !n->is_group()) {
				draw.del(n);
			}
			break;
		case sgnode::POINTS_CHANGED:
			if (display && !n->is_group()) {
				draw.set_vertices(n);
				draw.change(n->get_name(), drawer::VERTS);
			}
			break;
		case sgnode::TRANSFORM_CHANGED:
			if (display && !n->is_group()) {
				draw.set_transforms(n);
				draw.change(n->get_name(), drawer::POS | drawer::ROT | drawer::SCALE);
			}
			break;
	}
	dirty = true;
}

void scene::dump_sgel(ostream &os) {
	dump_sgel_rec(os, rootname, "");
}

void scene::dump_sgel_rec(ostream &os, const string &name, const string &parent) {
	property_map::const_iterator i;
	
	assert(nodes.find(name) != nodes.end());
	const node_info &info = nodes[name];
	sgnode *n = info.node;
	const property_map &m = info.props;
	if (name != rootname) {
		os << "a " << name << " " << parent << " ";
		if (!n->is_group()) {
			ptlist verts;
			n->get_local_points(verts);
			ptlist::const_iterator j;
			os << "v ";
			for (j = verts.begin(); j != verts.end(); ++j) {
				os << *j << " ";
			}
		}
		os << "p " << n->get_trans('p') << " ";
		os << "r " << n->get_trans('r') << " ";
		os << "s " << n->get_trans('s') << endl;
	}
	for (i = info.props.begin(); i != info.props.end(); ++i) {
		os << "p " << name << " " << i->first << " " << i->second << endl;
	}
	
	for (int j = 0; j < n->num_children(); ++j) {
		dump_sgel_rec(os, n->get_child(j)->get_name(), name);
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

void all_nodes(scene *scn, vector<vector<string> > &argset) {
	vector<sgnode*> nodes;
	vector<sgnode*>::const_iterator i;
	scn->get_all_nodes(nodes);
	for ( i = nodes.begin(); i != nodes.end(); ++i) {
		vector<string> p;
		p.push_back((**i).get_name());
		argset.push_back(p);
	}
}

void all_node_pairs_unordered_no_repeat(scene *scn, vector<vector<string> > &argset) {
	vector<sgnode*> nodes;
	vector<sgnode*>::const_iterator i;
	vector<string> names;
	vector<string>::const_iterator j, k;
	
	scn->get_all_nodes(nodes);
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		names.push_back((**i).get_name());
	}
	
	for (j = names.begin(); j != names.end(); ++j) {
		for (k = j + 1; k != names.end(); ++k) {
			vector<string> p;
			p.push_back(*j);
			p.push_back(*k);
			argset.push_back(p);
		}
	}
}

void all_node_pairs_ordered_no_repeat(scene *scn, vector<vector<string> > &argset) {
	vector<sgnode*> nodes;
	vector<sgnode*>::const_iterator i;
	vector<string> names;
	vector<string>::const_iterator j, k;
	
	scn->get_all_nodes(nodes);
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		names.push_back((**i).get_name());
	}
	
	for (j = names.begin(); j != names.end(); ++j) {
		for (k = names.begin(); k != names.end(); ++k) {
			if (k != j) {
				vector<string> p;
				p.push_back(*j);
				p.push_back(*k);
				argset.push_back(p);
			}
		}
	}
}

void all_node_triples_unordered_no_repeat(scene *scn, vector<vector<string> > &argset) {
	vector<sgnode*> nodes;
	vector<sgnode*>::const_iterator i;
	vector<string> names;
	vector<string>::const_iterator j, k, l;
	
	scn->get_all_nodes(nodes);
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		names.push_back((**i).get_name());
	}
	
	for (j = names.begin(); j != names.end(); ++j) {
		for (k = j + 1; k != names.end(); ++k) {
			for (l = k + 1; l != names.end(); ++l) {
				vector<string> p;
				p.push_back(*j);
				p.push_back(*k);
				p.push_back(*l);
				argset.push_back(p);
			}
		}
	}
}

void all_node_triples_ordered_no_repeat(scene *scn, vector<vector<string> > &argset) {
	vector<sgnode*> nodes;
	vector<sgnode*>::const_iterator i;
	vector<string> names;
	vector<string>::const_iterator j, k, l;
	
	scn->get_all_nodes(nodes);
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		names.push_back((**i).get_name());
	}
	
	for (j = names.begin(); j != names.end(); ++j) {
		for (k = names.begin(); k != names.end(); ++k) {
			for (l = names.begin(); l != names.end(); ++l) {
				if (j != k && j != l && k != l) {
					vector<string> p;
					p.push_back(*j);
					p.push_back(*k);
					p.push_back(*l);
					argset.push_back(p);
				}
			}
		}
	}
}
