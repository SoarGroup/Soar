#include <assert.h>
#include <list>
#include <vector>
#include <algorithm>
#include "sgnode.h"

using namespace std;

typedef vector<sgnode*>::iterator childiter;
typedef vector<sgnode*>::const_iterator const_childiter;

sgnode::sgnode(const string &name, const string &type, bool group)
: name(name), type(type), parent(NULL), group(group), trans_dirty(true), shape_dirty(true), bounds_dirty(true),
  pos(0.0, 0.0, 0.0), rot(0.0, 0.0, 0.0), scale(1.0, 1.0, 1.0)
{}


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

vec4 sgnode::get_quaternion() const {
	double halfyaw = rot(0) / 2, halfpitch = rot(1) / 2, halfroll = rot(2) / 2;  
	double cosyaw = cos(halfyaw);
 	double sinyaw = sin(halfyaw);
	double cospitch = cos(halfpitch);
	double sinpitch = sin(halfpitch);
	double cosroll = cos(halfroll);
	double sinroll = sin(halfroll);
	return vec4(cosroll * sinpitch * cosyaw + sinroll * cospitch * sinyaw,
	            cosroll * cospitch * sinyaw - sinroll * sinpitch * cosyaw,
	            sinroll * cospitch * cosyaw - cosroll * sinpitch * sinyaw,
	            cosroll * cospitch * cosyaw + sinroll * sinpitch * sinyaw);
}

void sgnode::get_trans(vec3 &p, vec3 &r, vec3 &s) const {
	p = pos;
	r = rot;
	s = scale;
}

void sgnode::get_world_trans(vec3 &p, vec3 &r, vec3 &s) const {
	if (parent) {
		parent->update_transform();
		p = parent->wtransform(pos);
		r = parent->wtransform(rot);
		s = parent->wtransform(scale);
	} else {
		p = pos;
		r = rot;
		s = scale;
	}
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
void sgnode::send_update(sgnode::change_type t, int added) {
	std::list<sgnode_listener*>::iterator i;
	std::list<sgnode_listener*> c;
	std::copy(listeners.begin(), listeners.end(), back_inserter(c));
	for (i = c.begin(); i != c.end(); ++i) {
		(**i).node_update(this, t, added);
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
	if (shape_dirty || trans_dirty) {
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
	group_node *g = dynamic_cast<group_node*>(this);
	assert(g);
	return g;
}

const group_node *sgnode::as_group() const {
	const group_node *g = dynamic_cast<const group_node*>(this);
	assert(g);
	return g;
}

sgnode *sgnode::clone() const {
	sgnode *c = clone_sub();
	c->set_trans(pos, rot, scale);
	return c;
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

bool group_node::attach_child(sgnode *c) {
	children.push_back(c);
	c->parent = this;
	c->set_transform_dirty();
	set_shape_dirty();
	send_update(sgnode::CHILD_ADDED, children.size() - 1);
	
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

convex_node::convex_node(const string &name, const string &type, const ptlist &v, const vector<int> &tris)
: geometry_node(name, type), verts(v), triangles(tris), dirty(true)
{
	assert(triangles.size() % 3 == 0);
	for (int i = 0; i < triangles.size(); ++i) {
		assert(triangles[i] < verts.size());
	}
}

sgnode *convex_node::clone_sub() const {
	return new convex_node(get_name(), get_type(), verts, triangles);
}

void convex_node::update_shape() {
	set_bounds(bbox(get_world_verts()));
}

void convex_node::set_transform_dirty_sub() {
	dirty = true;
}

const ptlist &convex_node::get_world_verts() const {
	if (dirty) {
		world_verts.clear();
		world_verts.resize(verts.size());
		transform(verts.begin(), verts.end(), world_verts.begin(), get_world_trans());
		dirty = false;
	}
	return world_verts;
}

void convex_node::get_shape_sgel(string &s) const {
	stringstream ss;
	ss << "v ";
	for (int i = 0; i < verts.size(); ++i) {
		ss << verts[i](0) << " " << verts[i](1) << " " << verts[i](2) << " ";
	}
	ss << "i ";
	for (int i = 0; i < triangles.size(); ++i) {
		ss << triangles[i] << " ";
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
