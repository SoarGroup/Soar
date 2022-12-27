#include <cassert>
#include <list>
#include <vector>
#include <iterator>
#include <algorithm>
#include "sgnode.h"
#include "sgnode_algs.h"
#include "ccd/ccd.h"
#include "params.h"
#include "scene.h"
#include <iostream>

typedef std::vector<sgnode*>::iterator childiter;
typedef std::vector<sgnode*>::const_iterator const_childiter;

sgnode::sgnode(const std::string& id, bool group)
    : parent(NULL), group(group), id(id),
      trans_dirty(true), shape_dirty(true), bounds_dirty(true),
      pos(0.0, 0.0, 0.0), rot(0.0, 0.0, 0.0), scale(1.0, 1.0, 1.0)
{
    set_help("Reports information about this node.");
}


sgnode::~sgnode()
{
    if (parent)
    {
        parent->detach_child(this);
    }
    send_update(sgnode::DELETED);
}

void sgnode::set_trans(char type, const vec3& t)
{
    switch (type)
    {
        case 'p':
            if (pos != t)
            {
                pos = t;
                set_transform_dirty();
            }
            break;
        case 'r':
            if (rot != t)
            {
                rot = t;
                set_transform_dirty();
            }
            break;
        case 's':
            if (scale != t)
            {
                scale = t;
                set_transform_dirty();
            }
            break;
        default:
            assert(false);
    }
}

void sgnode::set_trans(const vec3& p, const vec3& r, const vec3& s)
{
    if (pos != p || rot != r || scale != s)
    {
        pos = p;
        rot = r;
        scale = s;
        set_transform_dirty();
    }
}

vec3 sgnode::get_trans(char type) const
{
    switch (type)
    {
        case 'p':
            return pos;
        case 'r':
            return rot;
        case 's':
            return scale;
        default:
            assert(false);
            return pos;
    }
}

void sgnode::get_trans(vec3& p, vec3& r, vec3& s) const
{
    p = pos;
    r = rot;
    s = scale;
}

void sgnode::set_transform_dirty()
{
    trans_dirty = true;
    bounds_dirty = true;
    if (parent)
    {
        parent->set_shape_dirty();
    }
    set_transform_dirty_sub();
    send_update(sgnode::TRANSFORM_CHANGED);
}

void sgnode::set_shape_dirty()
{
    shape_dirty = true;
    bounds_dirty = true;
    if (parent)
    {
        parent->set_shape_dirty();
    }
    send_update(sgnode::SHAPE_CHANGED);
}

void sgnode::update_transform() const
{
    if (!trans_dirty)
    {
        return;
    }

    ltransform = transform3(pos, rot, scale);
    if (parent)
    {
        parent->update_transform();
        wtransform = parent->wtransform * ltransform;
    }
    else
    {
        wtransform = ltransform;
    }
    trans_dirty = false;
}

/* if updates result in observers removing themselves, the iteration may
 * screw up, so make a copy of the std::list first */
void sgnode::send_update(sgnode::change_type t, const std::string& update_info)
{
    std::list<sgnode_listener*>::iterator i;
    std::list<sgnode_listener*> c;
    std::copy(listeners.begin(), listeners.end(), back_inserter(c));
    for (i = c.begin(); i != c.end(); ++i)
    {
        (**i).node_update(this, t, update_info);
    }
}

void sgnode::listen(sgnode_listener* o)
{
    listeners.push_back(o);
}

void sgnode::unlisten(sgnode_listener* o)
{
    listeners.remove(o);
}

const bbox& sgnode::get_bounds() const
{
    if (bounds_dirty)
    {
        const_cast<sgnode*>(this)->update_shape();
        bounds_dirty = false;
    }
    return bounds;
}

vec3 sgnode::get_centroid() const
{
    if (shape_dirty || trans_dirty || bounds_dirty)
    {
        const_cast<sgnode*>(this)->update_shape();
    }
    return centroid;
}

void sgnode::set_bounds(const bbox& b)
{
    bounds = b;
    centroid = bounds.get_centroid();
    shape_dirty = false;
}

const transform3& sgnode::get_world_trans() const
{
    if (trans_dirty)
    {
        const_cast<sgnode*>(this)->update_transform();
    }
    return wtransform;
}

group_node* sgnode::as_group()
{
    return dynamic_cast<group_node*>(this);
}

const group_node* sgnode::as_group() const
{
    return dynamic_cast<const group_node*>(this);
}

sgnode* sgnode::clone() const
{
    sgnode* c = clone_sub();
    c->set_trans(pos, rot, scale);
    c->tags = tags;
    return c;
}

bool sgnode::has_descendent(const sgnode* n) const
{
    for (sgnode* p = n->parent; p; p = p->parent)
    {
        if (p == this)
        {
            return true;
        }
    }
    return false;
}

void sgnode::proxy_use_sub(const std::vector<std::string>& args, std::ostream& os)
{
    vec3 lp, ls, wp, ws;
    vec4 lr, wr;
    table_printer t, t1, t2, t3;

    t.add_row() << "id:"   << id;
    t.add_row() << "parent:" << (parent ? parent->get_id() : "none");
    t.print(os);

    os << std::endl << "Local transform:" << std::endl;
    update_transform();
    t1.add_row() << "pos:";
    for (int i = 0; i < 3; ++i)
    {
        t1 << pos(i);
    }
    t1.add_row() << "rot:";
    for (int i = 0; i < 3; ++i)
    {
        t1 << rot(i);
    }
    t1.add_row() << "scale:";
    for (int i = 0; i < 3; ++i)
    {
        t1 << scale(i);
    }
    t1.print(os);

    wtransform.to_prs(wp, wr, ws);
    os << std::endl << "World transform:" << std::endl;
    t2.add_row() << "pos:";
    for (int i = 0; i < 3; ++i)
    {
        t2 << wp(i);
    }
    t2.add_row() << "rot (quaternion):";
    for (int i = 0; i < 4; ++i)
    {
        t2 << wr(i);
    }
    t2.add_row() << "scale:";
    for (int i = 0; i < 3; ++i)
    {
        t2 << ws(i);
    }
    t2.print(os);

    tag_map::const_iterator ti;
    os << std::endl << "Tags:" << std::endl;
    for (ti = tags.begin(); ti != tags.end(); ti++)
    {
        t3.add_row() << ti->first << ti->second;
    }
    t3.print(os);
}

group_node::~group_node()
{
    childiter i;
    for (i = children.begin(); i != children.end(); ++i)
    {
        (**i).parent = NULL;  // so it doesn't try to detach itself
        delete *i;
    }
}

sgnode* group_node::clone_sub() const
{
    group_node* c = new group_node(get_id());
    const_childiter i;
    for (i = children.begin(); i != children.end(); ++i)
    {
        c->attach_child((**i).clone());
    }
    return c;
}

sgnode* group_node::get_child(size_t i)
{
    if (i < children.size())
    {
        return children[i];
    }
    return NULL;
}

const sgnode* group_node::get_child(size_t i) const
{
    if (i < children.size())
    {
        return children[i];
    }
    return NULL;
}

void group_node::walk(std::vector<sgnode*>& result)
{
    childiter i;
    result.push_back(this);
    for (i = children.begin(); i != children.end(); ++i)
    {
        (**i).walk(result);
    }
}

void group_node::walk_geoms(std::vector<geometry_node*>& g)
{
    childiter i, iend;
    for (i = children.begin(), iend = children.end(); i != iend; ++i)
    {
        (**i).walk_geoms(g);
    }
}

void group_node::walk_geoms(std::vector<const geometry_node*>& g) const
{
    const_childiter i, iend;
    for (i = children.begin(), iend = children.end(); i != iend; ++i)
    {
        (**i).walk_geoms(g);
    }
}

bool group_node::attach_child(sgnode* c)
{
    children.push_back(c);
    c->parent = this;
    c->set_transform_dirty();
    set_shape_dirty();
    std::string added_num = tostring(children.size() - 1);
    send_update(sgnode::CHILD_ADDED, added_num);

    return true;
}

void group_node::update_shape()
{
    if (children.empty())
    {
        vec3 c = get_world_trans()(vec3(0.0, 0.0, 0.0));
        set_bounds(bbox(c));
        return;
    }

    bbox b = children[0]->get_bounds();
    for (size_t i = 1; i < children.size(); ++i)
    {
        b.include(children[i]->get_bounds());
    }
    set_bounds(b);
}

void group_node::detach_child(sgnode* c)
{
    childiter i;
    for (i = children.begin(); i != children.end(); ++i)
    {
        if (*i == c)
        {
            children.erase(i);
            set_shape_dirty();
            return;
        }
    }
}

void group_node::set_transform_dirty_sub()
{
    for (childiter i = children.begin(); i != children.end(); ++i)
    {
        (**i).set_transform_dirty();
    }
}

void group_node::proxy_get_children(std::map<std::string, cliproxy*>& c)
{
    for (size_t i = 0, iend = children.size(); i < iend; ++i)
    {
        c[children[i]->get_id()] = children[i];
    }
}

double group_node::max_project_on_axis(const vec3& axis) const
{
    double max = get_centroid().dot(axis);
    for (size_t i = 0, iend = children.size(); i < iend; ++i)
    {
        double child_proj = children[i]->max_project_on_axis(axis);
        if(child_proj > max)
        {
            max = child_proj;
        }
    }
    return max;
}

double group_node::min_project_on_axis(const vec3& axis) const
{
    double min = get_centroid().dot(axis);
    for (size_t i = 0, iend = children.size(); i < iend; ++i)
    {
        double child_proj = children[i]->min_project_on_axis(axis);
        if(child_proj < min)
        {
            min = child_proj;
        }
    }
    return min;
}

/*
 Based on the fact that the support s_T(v) of a geometry under transformation
 T(x) = Bx + c is T(s(Bt(v))), where Bt is the transpose of B.

 For more information see

 "Bergen, G. (1999) A Fast and Robust GJK Implementation for Collision
  Detection of Convex Objects."
*/
void geometry_node::gjk_support(const vec3& dir, vec3& support) const
{
    vec3 tdir;
    mat B;
    transform3 t;

    t = get_world_trans();
    t.get_matrix(B);
    tdir = B.block(0, 0, 3, 3).transpose() * dir;
    gjk_local_support(tdir, support);
    support = t(support);
}

void geometry_node::walk_geoms(std::vector<geometry_node*>& g)
{
    g.push_back(this);
}

void geometry_node::walk_geoms(std::vector<const geometry_node*>& g) const
{
    g.push_back(this);
}

convex_node::convex_node(const std::string& id, const ptlist& v)
    : geometry_node(id), verts(v), world_verts_dirty(true)
{}

sgnode* convex_node::clone_sub() const
{
    return new convex_node(get_id(), verts);
}

void convex_node::update_shape()
{
    set_bounds(bbox(get_world_verts()));
}

void convex_node::set_transform_dirty_sub()
{
    world_verts_dirty = true;
}

void convex_node::set_verts(const ptlist& v)
{
    verts = v;
    world_verts_dirty = true;
    set_shape_dirty();
}

const ptlist& convex_node::get_world_verts() const
{
    if (world_verts_dirty)
    {
        world_verts.clear();
        world_verts.resize(verts.size());
        transform(verts.begin(), verts.end(), world_verts.begin(), get_world_trans());
        world_verts_dirty = false;
    }
    return world_verts;
}

void convex_node::get_shape_sgel(std::string& s) const
{
    std::stringstream ss;
    ss << "v ";
    for (size_t i = 0; i < verts.size(); ++i)
    {
        ss << verts[i](0) << " " << verts[i](1) << " " << verts[i](2) << " ";
    }
    s = ss.str();
}

/*
 This is a naive implementation. Should be able to get complexity to sublinear.
*/
void convex_node::gjk_local_support(const vec3& dir, vec3& support) const
{
    double dp, best = 0.0;
    long long best_i = -1;

    for (size_t i = 0; i < verts.size(); ++i)
    {
        dp = dir.dot(verts[i]);
        if (best_i == -1 || dp > best)
        {
            best = dp;
            best_i = i;
        }
    }
    support = verts[static_cast<size_t>(best_i)];
}

void convex_node::proxy_use_sub(const std::vector<std::string>& args, std::ostream& os)
{
    sgnode::proxy_use_sub(args, os);

    table_printer t;
    for (size_t i = 0, iend = verts.size(); i < iend; ++i)
    {
        t.add_row() << verts[i](0) << verts[i](1) << verts[i](2);
    }

    os << std::endl << "vertices" << std::endl;
    t.print(os);
}

double convex_node::max_project_on_axis(const vec3& axis) const
{
    double max = get_centroid().dot(axis);
    const ptlist& world_verts = get_world_verts();
    for (size_t i = 0, iend = world_verts.size(); i < iend; ++i)
    {
        double vert_proj = world_verts[i].dot(axis);

        if(vert_proj > max)
        {
            max = vert_proj;
        }
    }
    return max;
}

double convex_node::min_project_on_axis(const vec3& axis) const
{
    double min = get_centroid().dot(axis);
    const ptlist& world_verts = get_world_verts();
    for (size_t i = 0, iend = world_verts.size(); i < iend; ++i)
    {
        double vert_proj = world_verts[i].dot(axis);
        if(vert_proj < min)
        {
            min = vert_proj;
        }
    }
    return min;
}

ball_node::ball_node(const std::string& id, double radius)
    : geometry_node(id), radius(radius)
{}

void ball_node::get_shape_sgel(std::string& s) const
{
    std::stringstream ss;
    ss << "b " << radius;
    s = ss.str();
}

sgnode* ball_node::clone_sub() const
{
    return new ball_node(get_id(), radius);
}

/*
 This will overestimate the bounding box right now.
*/
void ball_node::update_shape()
{
    transform3 t = get_world_trans();
    bbox bb(t(vec3(-radius, -radius, -radius)));
    bb.include(t(vec3(-radius, -radius, radius)));
    bb.include(t(vec3(-radius, radius, -radius)));
    bb.include(t(vec3(-radius, radius, radius)));
    bb.include(t(vec3(radius, -radius, -radius)));
    bb.include(t(vec3(radius, -radius, radius)));
    bb.include(t(vec3(radius, radius, -radius)));
    bb.include(t(vec3(radius, radius, radius)));
    set_bounds(bb);
}

void ball_node::set_radius(double r)
{
    radius = r;
    set_shape_dirty();
}

void ball_node::gjk_local_support(const vec3& dir, vec3& support) const
{
    support = radius * dir.normalized();
}

void ball_node::proxy_use_sub(const std::vector<std::string>& args, std::ostream& os)
{
    sgnode::proxy_use_sub(args, os);

    os << std::endl << "radius: " << radius << std::endl;
}

double ball_node::max_project_on_axis(const vec3& axis) const
{
    double world_radius = get_world_trans()(vec3(radius, 0.0, 0.0)).norm();
    return get_centroid().dot(axis) + world_radius;
}

double ball_node::min_project_on_axis(const vec3& axis) const
{
    double world_radius = get_world_trans()(vec3(radius, 0.0, 0.0)).norm();
    return get_centroid().dot(axis) - world_radius;
}

const tag_map& sgnode::get_all_tags() const
{
    return tags;
}

bool sgnode::get_tag(const std::string& tag_name, std::string& tag_value) const
{
    return map_get(tags, tag_name, tag_value);
}

void sgnode::set_tag(const std::string& tag_name, const std::string& tag_value)
{
    tags[tag_name] = tag_value;
    send_update(sgnode::TAG_CHANGED, tag_name);
}

void sgnode::delete_tag(const std::string& tag_name)
{
    tag_map::iterator i = tags.find(tag_name);
    if (i != tags.end())
    {
        tags.erase(i);
        send_update(sgnode::TAG_DELETED, tag_name);
    }
}

