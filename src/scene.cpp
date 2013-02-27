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
#include "params.h"
#include "ccd/ccd.h"

using namespace std;

const string root_name = "world";

/*
 Native properties are currently the position, rotation, and scaling
 transforms of a node, named px, py, pz, rx, ry, rz, sx, sy, sz.
*/
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

void ccd_support(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *v) {
	vec3 d, support;
	const geometry_node *n = static_cast<const geometry_node*>(obj);
	
	for (int i = 0; i < 3; ++i) {
		d(i) = dir->v[i];
	}
	n->gjk_support(d, support);
	for (int i = 0; i < 3; ++i) {
		v->v[i] = support(i);
	}
}

double geom_convex_dist(const geometry_node *n1, const geometry_node *n2) {
	geometry_node *g1, *g2;
	ccd_t ccd;
	double dist;
	
	CCD_INIT(&ccd);
	ccd.support1       = ccd_support;
	ccd.support2       = ccd_support;
	ccd.max_iterations = 100;
	
	dist = ccdGJKDist(n1, n2, &ccd);
	return dist > 0.0 ? dist : 0.0;
}

double convex_dist(const sgnode *n1, const sgnode *n2) {
	vector<const geometry_node*> g1, g2;
	
	if (n1 == n2 || n1->has_descendent(n2) || n2->has_descendent(n1)) {
		return 0.0;
	}
	
	n1->walk_geoms(g1);
	n2->walk_geoms(g2);
	double d, mindist = -1.0;
	for (int i = 0, iend = g1.size(); i < iend; ++i) {
		for (int j = 0, jend = g2.size(); j < jend; ++j) {
			d = geom_convex_dist(g1[i], g2[j]);
			if (mindist < 0 || d < mindist) {
				mindist = d;
			}
		}
	}
	return mindist;
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
	while (start < f.size() && parse_int(f[start], i)) {
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
: name(name), draw(d), nodes(1)
{
	root = new group_node(root_name, "world");
	nodes[0].node = root;
	root->listen(this);
	sig_dirty = true;
	closest_dirty = false;
}

scene::~scene() {
	delete root;
}

scene *scene::clone(const string &cname, drawer *d) const {
	scene *c = new scene(cname, d);
	string name;
	std::vector<sgnode*> all_nodes;
	std::vector<sgnode*>::const_iterator i;
	
	delete c->root;
	c->root = root->clone()->as_group();
	c->root->walk(all_nodes);
	c->nodes.resize(all_nodes.size());
	
	update_closest();
	for(int i = 1, iend = all_nodes.size(); i < iend; ++i) {  // i = 0 is world, already in copy
		sgnode *n = all_nodes[i];
		node_info &cinfo = c->nodes[i];
		cinfo = *find_name(n->get_name());
		cinfo.node = n;
		n->listen(c);
		if (c->draw) {
			c->draw->add(c->name, n);
		}
	}
	return c;
}

scene::node_info *scene::find_name(const string &name) {
	node_table::iterator i, iend;
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		if (i->node->get_name() == name) {
			return &(*i);
		}
	}
	return NULL;
}

const scene::node_info *scene::find_name(const string &name) const {
	node_table::const_iterator i, iend;
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		if (i->node->get_name() == name) {
			return &(*i);
		}
	}
	return NULL;
}

sgnode *scene::get_node(const string &name) {
	node_info *info = find_name(name);
	if (!info) {
		return NULL;
	}
	return info->node;
}

sgnode const *scene::get_node(const string &name) const {
	const node_info *info = find_name(name);
	if (!info) {
		return NULL;
	}
	return info->node;
}

sgnode *scene::get_node(int id) {
	node_table::iterator i, iend;
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		if (i->node->get_id() == id) {
			return i->node;
		}
	}
	return NULL;
}

const sgnode *scene::get_node(int id) const {
	node_table::const_iterator i, iend;
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		if (i->node->get_id() == id) {
			return i->node;
		}
	}
	return NULL;
}

group_node *scene::get_group(const string &name) {
	sgnode *n = get_node(name);
	if (n) {
		return n->as_group();
	}
	return NULL;
}

void scene::get_all_nodes(vector<sgnode*> &n) {
	n.reserve(nodes.size() - 1);
	node_table::const_iterator i, iend;
	for (i = nodes.begin() + 1, iend = nodes.end(); i != iend; ++i) {
		n.push_back(i->node);
	}
}

void scene::get_all_nodes(vector<const sgnode*> &n) const {
	n.reserve(nodes.size() - 1);
	node_table::const_iterator i, iend;
	for (i = nodes.begin() + 1, iend = nodes.end(); i != iend; ++i) {
		n.push_back(i->node);
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
	delete find_name(name)->node;
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
	int p = 0;
	node_info *ninfo;
	
	if (p >= f.size()) {
		error = "expecting node name";
		return p;
	}
	
	if (!(ninfo = find_name(f[p++]))) {
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
	ninfo->props[prop] = val;
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
		split(*i, "", fields);
		
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
	for (int j = 0; j < NUM_NATIVE_PROPS; ++j) {
		names.push_back(NATIVE_PROPS[j]);
	}
	
	const node_info &info = nodes[i];
	property_map::const_iterator k, kend;
	for (k = info.props.begin(), kend = info.props.end(); k != kend; ++k) {
		names.push_back(k->first);
	}
}

void scene::get_properties(rvec &vals) const {
	node_table::const_iterator i, iend;
	property_map::const_iterator j, jend;
	int k = 0;
	
	vals.resize(get_dof());
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		const node_info &info = *i;
		for (const char *t = "prs"; *t != '\0'; ++t) {
			vec3 trans = info.node->get_trans(*t);
			vals.segment(k, 3) = trans;
			k += 3;
		}
		for (j = info.props.begin(), jend = info.props.end(); j != jend; ++j) {
			vals[k++] = j->second;
		}
	}
}

bool scene::set_property(const string &obj, const string &prop, double val) {
	char type; int d;
	node_info *info = find_name(obj);
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
	node_table::iterator i, iend;;
	const char *types = "prs";
	vec3 trans;
	
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		int k1, k2, l = 0;
		for (k1 = 0; k1 < 3; ++k1) {
			for (k2 = 0; k2 < 3; ++k2) {
				trans[k2] = vals[l++];
				if (l >= vals.size()) {
					return false;
				}
			}
			i->node->set_trans(types[k1], trans);
		}
		
		property_map::iterator j, jend;
		for (j = i->props.begin(), jend = i->props.end(); j != jend; ++j) {
			j->second = vals[l++];
			if (l >= vals.size()) {
				return false;
			}
		}
	}
	return true;
}

void scene::remove_property(const std::string &name, const std::string &prop) {
	node_info *info = find_name(name);
	property_map::iterator j = info->props.find(prop);
	assert(j != info->props.end());
	info->props.erase(j);
}

int scene::get_dof() const {
	int dof = 0;
	node_table::const_iterator i, iend;
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		dof += NUM_NATIVE_PROPS + i->props.size();
	}
	return dof;
}

void scene::node_update(sgnode *n, sgnode::change_type t, int added_child) {
	sgnode *child;
	group_node *g;
	relation *tr;
	
	if (t == sgnode::CHILD_ADDED) {
		g = n->as_group();
		child = g->get_child(added_child);
		child->listen(this);
		node_info &ninfo = grow_vec(nodes);
		ninfo.node = child;
		sig_dirty = true;
		update_dists(nodes.size() - 1);
		
		tr = map_getp(type_rels, child->get_type());
		if (!tr) {
			tr = &type_rels[child->get_type()];
			tr->reset(2);
		}
		tr->add(0, n->get_id());
		
		if (draw) {
			draw->add(name, child);
		}
		return;
	}
	
	int i, iend;
	for (i = 0, iend = nodes.size(); i < iend && nodes[i].node != n; ++i)
		;
	assert(i != iend);
	
	switch (t) {
		case sgnode::DELETED:
			nodes.erase(nodes.begin() + i);
			
			// update distance vectors for other nodes
			for (int j = 0, jend = nodes.size(); j < jend; ++j) {
				node_info &info = nodes[j];
				assert(info.dists.size() == nodes.size() + 1);
				info.dists.erase(info.dists.begin() + i);
			}
			closest_dirty = true;
			tr = map_getp(type_rels, n->get_type());
			if (tr) {
				tr->del(0, i);
			}
			sig_dirty = true;
			if (draw && i != 0) {
				draw->del(name, n);
			}
			break;
		case sgnode::SHAPE_CHANGED:
			update_dists(i);
			if (!n->is_group() && draw) {
				draw->change(name, n, drawer::SHAPE);
			}
			nodes[i].rels_dirty = true;
			break;
		case sgnode::TRANSFORM_CHANGED:
			update_dists(i);
			if (!n->is_group() && draw) {
				draw->change(name, n, drawer::POS | drawer::ROT | drawer::SCALE);
			}
			nodes[i].rels_dirty = true;
			break;
	}
}

bool scene::intersects(const sgnode *a, const sgnode *b) const {
	int i, j;
	for (i = 0; i < nodes.size() && nodes[i].node != a; ++i)
		;
	for (j = 0; j < nodes.size() && nodes[j].node != b; ++j)
		;
	assert(i != nodes.size() && j != nodes.size());
	return nodes[i].dists[j] == 0.0;
}

void scene::update_dists(int i) {
	node_info &n1 = nodes[i];
	
	n1.dists.resize(nodes.size(), -1);
	n1.dists[i] = 0.0;
	for (int j = 0, jend = nodes.size(); j < jend; ++j) {
		if (i == j)
			continue;
		
		node_info &n2 = nodes[j];
		n2.dists.resize(nodes.size(), -1);
		double d = convex_dist(n1.node, n2.node);
		n1.dists[j] = d;
		n2.dists[i] = d;
	}
	closest_dirty = true;
}

void scene::update_closest() const {
	if (closest_dirty) {
		for (int i = 0, iend = nodes.size(); i < iend; ++i) {
			int c = -1;
			const vector<double> &d = nodes[i].dists;
			for (int j = 0, jend = d.size(); j < jend; ++j) {
				if (d[j] >= 0 && (c < 0 || d[j] < d[c])) {
					c = j;
				}
			}
			nodes[i].closest = c;
		}
		closest_dirty = false;
	}
}

void scene::update_sig() const {
	sig.clear();
	node_table::const_iterator i, iend;
	for (int i = 0, iend = nodes.size(); i < iend; ++i) {
		scene_sig::entry e;
		e.id = nodes[i].node->get_id();
		e.name = nodes[i].node->get_name();
		e.type = nodes[i].node->get_type();
		get_property_names(i, e.props);
		sig.add(e);
	}
}

const scene_sig &scene::get_signature() const {
	if (sig_dirty) {
		update_sig();
	}
	return sig;
}

void scene::get_relations(relation_table &rt) const {
	tuple closest(2);
	vector<int> dirty_nodes;
	node_table::const_iterator i, iend;
	
	rt = type_rels;
	relation &closest_rel = rt["closest"];
	closest_rel.reset(3);
	update_closest();
	for (i = nodes.begin() + 1, iend = nodes.end(); i != iend; ++i) {
		if (i->rels_dirty) {
			dirty_nodes.push_back(i->node->get_id());
			i->rels_dirty = false;
		}
		closest[0] = i->node->get_id();
		closest[1] = nodes[i->closest].node->get_id();
		closest_rel.add(0, closest);
	}
	
	relation_table::iterator j, jend;
	for (j = cached_rels.begin(), jend = cached_rels.end(); j != jend; ++j) {
		relation &r = j->second;
		for (int k = 1, kend = r.arity(); k < kend; ++k) {
			r.filter(k, dirty_nodes, true);
		}
	}
	get_filter_table().update_relations(this, dirty_nodes, 0, cached_rels);
	
	for (j = cached_rels.begin(), jend = cached_rels.end(); j != jend; ++j) {
		rt[j->first] = j->second;
	}
}
