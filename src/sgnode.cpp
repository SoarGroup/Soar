#include <assert.h>
#include <list>
#include <vector>
#include <iterator>
#include <algorithm>
#include "sgnode.h"
#include "ccd/ccd.h"
#include "params.h"

using namespace std;

/*
 Making this a global variable insures that all nodes in all scenes
 will have unique identifiers.
*/
int node_counter = 100;

typedef vector<sgnode*>::iterator childiter;
typedef vector<sgnode*>::const_iterator const_childiter;

sgnode::sgnode(const string &name, const string &type, bool group)
: name(name), type(type), parent(NULL), group(group), id(node_counter++),
  trans_dirty(true), shape_dirty(true), bounds_dirty(true),
  pos(0.0, 0.0, 0.0), rot(0.0, 0.0, 0.0), scale(1.0, 1.0, 1.0)
{
	set_help("Reports information about this node.");
}


sgnode::~sgnode() {
	if (parent) {
		parent->detach_child(this);
	}
	send_update(sgnode::DELETED);
}

void sgnode::set_trans(char type, const vec3 &t) {
	switch (type) {
		case 'p':
			if (pos != t) {
				pos = t;
				set_transform_dirty();
			}
			break;
		case 'r':
			if (rot != t) {
				rot = t;
				set_transform_dirty();
			}
			break;
		case 's':
			if (scale != t) {
				scale = t;
				set_transform_dirty();
			}
			break;
		default:
			assert(false);
	}
}

void sgnode::set_trans(const vec3 &p, const vec3 &r, const vec3 &s) {
	if (pos != p || rot != r || scale != s) {
		pos = p;
		rot = r;
		scale = s;
		set_transform_dirty();
	}
}

vec3 sgnode::get_trans(char type) const {
	switch (type) {
		case 'p':
			return pos;
		case 'r':
			return rot;
		case 's':
			return scale;
		default:
			assert (false);
	}
}

void sgnode::get_trans(vec3 &p, vec3 &r, vec3 &s) const {
	p = pos;
	r = rot;
	s = scale;
}

void sgnode::set_transform_dirty() {
	trans_dirty = true;
	bounds_dirty = true;
	if (parent) {
		parent->set_shape_dirty();
	}
	set_transform_dirty_sub();
	send_update(sgnode::TRANSFORM_CHANGED);
}

void sgnode::set_shape_dirty() {
	shape_dirty = true;
	bounds_dirty = true;
	if (parent) {
		parent->set_shape_dirty();
	}
	send_update(sgnode::SHAPE_CHANGED);
}

void sgnode::update_transform() const {
	if (!trans_dirty) {
		return;
	}
	
	ltransform = transform3(pos, rot, scale);
	if (parent) {
		parent->update_transform();
		wtransform = parent->wtransform * ltransform;
	} else {
		wtransform = ltransform;
	}
	trans_dirty = false;
}

/* if updates result in observers removing themselves, the iteration may
 * screw up, so make a copy of the std::list first */
void sgnode::send_update(sgnode::change_type t, const std::string& update_info) {
	std::list<sgnode_listener*>::iterator i;
	std::list<sgnode_listener*> c;
	std::copy(listeners.begin(), listeners.end(), back_inserter(c));
	for (i = c.begin(); i != c.end(); ++i) {
		(**i).node_update(this, t, update_info);
	}
}

void sgnode::listen(sgnode_listener *o) {
	listeners.push_back(o);
}

void sgnode::unlisten(sgnode_listener *o) {
	listeners.remove(o);
}

const bbox &sgnode::get_bounds() const {
	if (bounds_dirty) {
		const_cast<sgnode*>(this)->update_shape();
		bounds_dirty = false;
	}
	return bounds;
}

vec3 sgnode::get_centroid() const {
	if (shape_dirty || trans_dirty || bounds_dirty) {
		const_cast<sgnode*>(this)->update_shape();
	}
	return centroid;
}

void sgnode::set_bounds(const bbox &b) {
	bounds = b;
	centroid = bounds.get_centroid();
	shape_dirty = false;
}

const transform3 &sgnode::get_world_trans() const {
	if (trans_dirty) {
		const_cast<sgnode*>(this)->update_transform();
	}
	return wtransform;
}

group_node *sgnode::as_group() {
	return dynamic_cast<group_node*>(this);
}

const group_node *sgnode::as_group() const {
	return dynamic_cast<const group_node*>(this);
}

sgnode *sgnode::clone() const {
	sgnode *c = clone_sub();
	c->set_trans(pos, rot, scale);
	c->string_props = string_props;
	c->numeric_props = numeric_props;
	return c;
}

bool sgnode::has_descendent(const sgnode *n) const {
	for (sgnode *p = n->parent; p; p = p->parent) {
		if (p == this)
			return true;
	}
	return false;
}

void sgnode::proxy_use_sub(const vector<string> &args, ostream &os) {
	vec3 lp, ls, wp, ws;
	vec4 lr, wr;
	table_printer t, t1, t2;
	
	t.add_row() << "id:"     << id;
	t.add_row() << "name:"   << name;
	t.add_row() << "type:"   << type;
	t.add_row() << "parent:" << (parent ? parent->get_name() : "none");
	t.print(os);
	
	os << endl << "Local transform:" << endl;
	update_transform();
	ltransform.to_prs(lp, lr, ls);
	t1.add_row() << "pos:";
	for (int i = 0; i < 3; ++i) {
		t1 << lp(i);
	}
	t1.add_row() << "rot:";
	for (int i = 0; i < 4; ++i) {
		t1 << lr(i);
	}
	t1.add_row() << "scale:";
	for (int i = 0; i < 3; ++i) {
		t1 << ls(i);
	}
	t1.print(os);

	wtransform.to_prs(wp, wr, ws);
	os << endl << "World transform:" << endl;
	t2.add_row() << "pos:";
	for (int i = 0; i < 3; ++i) {
		t2 << wp(i);
	}
	t2.add_row() << "rot:";
	for (int i = 0; i < 4; ++i) {
		t2 << wr(i);
	}
	t2.add_row() << "scale:";
	for (int i = 0; i < 3; ++i) {
		t2 << ws(i);
	}
	t2.print(os);

}

group_node::~group_node() {
	childiter i;
	for (i = children.begin(); i != children.end(); ++i) {
		(**i).parent = NULL;  // so it doesn't try to detach itself
		delete *i;
	}
}

sgnode* group_node::clone_sub() const {
	group_node *c = new group_node(get_name(), get_type());
	const_childiter i;
	for(i = children.begin(); i != children.end(); ++i) {
		c->attach_child((**i).clone());
	}
	return c;
}

sgnode* group_node::get_child(int i) {
	if (0 <= i && i < children.size()) {
		return children[i];
	}
	return NULL;
}

const sgnode *group_node::get_child(int i) const {
	if (0 <= i && i < children.size()) {
		return children[i];
	}
	return NULL;
}

void group_node::walk(vector<sgnode*> &result) {
	childiter i;
	result.push_back(this);
	for(i = children.begin(); i != children.end(); ++i) {
		(**i).walk(result);
	}
}

void group_node::walk_geoms(vector<geometry_node*> &g) {
	childiter i, iend;
	for(i = children.begin(), iend = children.end(); i != iend; ++i) {
		(**i).walk_geoms(g);
	}
}

void group_node::walk_geoms(vector<const geometry_node*> &g) const {
	const_childiter i, iend;
	for(i = children.begin(), iend = children.end(); i != iend; ++i) {
		(**i).walk_geoms(g);
	}
}

bool group_node::attach_child(sgnode *c) {
	children.push_back(c);
	c->parent = this;
	c->set_transform_dirty();
	set_shape_dirty();
	std::string added_num = tostring(children.size() - 1);
	send_update(sgnode::CHILD_ADDED, added_num);
	
	return true;
}

void group_node::update_shape() {
	if (children.empty()) {
		vec3 c = get_world_trans()(vec3(0.0,0.0,0.0));
		set_bounds(bbox(c));
		return;
	}
	
	bbox b = children[0]->get_bounds();
	for (int i = 1; i < children.size(); ++i) {
		b.include(children[i]->get_bounds());
	}
	set_bounds(b);
}

void group_node::detach_child(sgnode *c) {
	childiter i;
	for (i = children.begin(); i != children.end(); ++i) {
		if (*i == c) {
			children.erase(i);
			set_shape_dirty();
			return;
		}
	}
}

void group_node::set_transform_dirty_sub() {
	for (childiter i = children.begin(); i != children.end(); ++i) {
		(**i).set_transform_dirty();
	}
}

void group_node::proxy_get_children(map<string, cliproxy*> &c) {
	for (int i = 0, iend = children.size(); i < iend; ++i) {
		c[children[i]->get_name()] = children[i];
	}
}

/*
 Based on the fact that the support s_T(v) of a geometry under transformation
 T(x) = Bx + c is T(s(Bt(v))), where Bt is the transpose of B.
 
 For more information see
 
 "Bergen, G. (1999) A Fast and Robust GJK Implementation for Collision
  Detection of Convex Objects."
*/
void geometry_node::gjk_support(const vec3 &dir, vec3 &support) const {
	vec3 tdir;
	mat B;
	transform3 t;
	
	t = get_world_trans();
	t.get_matrix(B);
	tdir = B.block(0, 0, 3, 3).transpose() * dir;
	gjk_local_support(tdir, support);
	support = t(support);
}

void geometry_node::walk_geoms(std::vector<geometry_node*> &g) {
	g.push_back(this);
}

void geometry_node::walk_geoms(std::vector<const geometry_node*> &g) const {
	g.push_back(this);
}

convex_node::convex_node(const string &name, const string &type, const ptlist &v)
: geometry_node(name, type), verts(v), world_verts_dirty(true)
{}

sgnode *convex_node::clone_sub() const {
	return new convex_node(get_name(), get_type(), verts);
}

void convex_node::update_shape() {
	set_bounds(bbox(get_world_verts()));
}

void convex_node::set_transform_dirty_sub() {
	world_verts_dirty = true;
}

void convex_node::set_verts(const ptlist &v) {
	verts = v;
	world_verts_dirty = true;
	set_shape_dirty();
}

const ptlist &convex_node::get_world_verts() const {
	if (world_verts_dirty) {
		world_verts.clear();
		world_verts.resize(verts.size());
		transform(verts.begin(), verts.end(), world_verts.begin(), get_world_trans());
		world_verts_dirty = false;
	}
	return world_verts;
}

void convex_node::get_shape_sgel(string &s) const {
	stringstream ss;
	ss << "v ";
	for (int i = 0; i < verts.size(); ++i) {
		ss << verts[i](0) << " " << verts[i](1) << " " << verts[i](2) << " ";
	}
	s = ss.str();
}

/*
 This is a naive implementation. Should be able to get complexity to sublinear.
*/
void convex_node::gjk_local_support(const vec3 &dir, vec3 &support) const {
	double dp, best;
	int best_i = -1;
	
	for (int i = 0; i < verts.size(); ++i) {
		dp = dir.dot(verts[i]);
		if (best_i == -1 || dp > best) {
			best = dp;
			best_i = i;
		}
	}
	support = verts[best_i];
}

void convex_node::proxy_use_sub(const vector<string> &args, ostream &os) {
	sgnode::proxy_use_sub(args, os);
	
	table_printer t;
	for (int i = 0, iend = verts.size(); i < iend; ++i) {
		t.add_row() << verts[i](0) << verts[i](1) << verts[i](2);
	}
	
	os << endl << "vertices" << endl;
	t.print(os);
}

ball_node::ball_node(const string &name, const string &type, double radius)
: geometry_node(name, type), radius(radius)
{}

void ball_node::get_shape_sgel(string &s) const {
	stringstream ss;
	ss << "b " << radius;
	s = ss.str();
}

sgnode *ball_node::clone_sub() const {
	return new ball_node(get_name(), get_type(), radius);
}

/*
 This will overestimate the bounding box right now.
*/
void ball_node::update_shape() {
	transform3 t = get_world_trans();
	bbox bb(t(vec3(-radius,-radius,-radius)));
	bb.include(t(vec3(-radius,-radius, radius)));
	bb.include(t(vec3(-radius, radius,-radius)));
	bb.include(t(vec3(-radius, radius, radius)));
	bb.include(t(vec3( radius,-radius,-radius)));
	bb.include(t(vec3( radius,-radius, radius)));
	bb.include(t(vec3( radius, radius,-radius)));
	bb.include(t(vec3( radius, radius, radius)));
	set_bounds(bb);
}

void ball_node::set_radius(double r) {
	radius = r;
	set_shape_dirty();
}

void ball_node::gjk_local_support(const vec3 &dir, vec3 &support) const {
	support = radius * dir.normalized();
}

void ball_node::proxy_use_sub(const vector<string> &args, ostream &os) {
	sgnode::proxy_use_sub(args, os);
	
	os << endl << "radius: " << radius << endl;
}

void point_ccd_support(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *v) {
	const vec3 *point = static_cast<const vec3*>(obj);
	for (int i = 0; i < 3; ++i) {
		v->v[i] = (*point)(i);
	}
}

void geom_ccd_support(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *v) {
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
	ccd_t ccd;
	double dist;
	
	CCD_INIT(&ccd);
	ccd.support1       = geom_ccd_support;
	ccd.support2       = geom_ccd_support;
	ccd.max_iterations = 100;
	ccd.dist_tolerance = INTERSECT_THRESH;
	
	dist = ccdGJKDist(n1, n2, &ccd);
	return dist > 0.0 ? dist : 0.0;
}

double point_geom_convex_dist(const vec3 &p, const geometry_node *g) {
	ccd_t ccd;
	double dist;
	
	CCD_INIT(&ccd);
	ccd.support1       = point_ccd_support;
	ccd.support2       = geom_ccd_support;
	ccd.max_iterations = 100;
	ccd.dist_tolerance = INTERSECT_THRESH;
	
	dist = ccdGJKDist(&p, g, &ccd);
	return dist > 0.0 ? dist : 0.0;
}

double convex_distance(const sgnode *n1, const sgnode *n2) {
	vector<const geometry_node*> g1, g2;
	vector<double> dists;
	vec3 c;
	
	if (n1 == n2 || n1->has_descendent(n2) || n2->has_descendent(n1)) {
		return 0.0;
	}
	
	n1->walk_geoms(g1);
	n2->walk_geoms(g2);
	
	if (g1.empty() && g2.empty()) {
		return (n1->get_centroid() - n2->get_centroid()).norm();
	}
	
	if (g1.empty()) {
		dists.reserve(g2.size());
		c = n1->get_centroid();
		for (int i = 0, iend = g2.size(); i < iend; ++i) {
			dists.push_back(point_geom_convex_dist(c, g2[i]));
		}
	} else if (g2.empty()) {
		dists.reserve(g1.size());
		c = n2->get_centroid();
		for (int i = 0, iend = g1.size(); i < iend; ++i) {
			dists.push_back(point_geom_convex_dist(c, g1[i]));
		}
	} else {
		dists.reserve(g1.size() * g2.size());
		for (int i = 0, iend = g1.size(); i < iend; ++i) {
			for (int j = 0, jend = g2.size(); j < jend; ++j) {
				dists.push_back(geom_convex_dist(g1[i], g2[j]));
			}
		}
	}
	return *min_element(dists.begin(), dists.end());
}

bool intersects(const sgnode *n1, const sgnode *n2) {
	if (n1->get_bounds().intersects(n2->get_bounds())) {
		return convex_distance(n1, n2) < INTERSECT_THRESH;
	}
	return false;
}

void sgnode::set_property(const std::string& propertyName, const std::string& value){
	double numericValue;
	if(parse_double(value, numericValue)){
		string_props.erase(propertyName);
		numeric_props[propertyName] = numericValue;
	} else {
		numeric_props.erase(propertyName);
		string_props[propertyName] = value;
	}
	send_update(sgnode::PROPERTY_CHANGED, propertyName);
}

void sgnode::delete_property(const std::string& propertyName){
	bool deleted = false;
	if(numeric_props.erase(propertyName) > 0){
		deleted = true;
	}
	if(string_props.erase(propertyName) > 0){
		deleted = true;
	}
	if(deleted){
		send_update(sgnode::PROPERTY_DELETED, propertyName);
	}
}
