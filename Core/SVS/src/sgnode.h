#ifndef SGNODE_H
#define SGNODE_H

/* native scene graph implementation */

#include <vector>
#include <list>
#include <string>
#include "common.h"
#include "mat.h"
#include "cliproxy.h"

class sgnode_listener;
class group_node;
class geometry_node;


typedef std::map<std::string, std::string> string_properties_map;
typedef std::map<std::string, double> numeric_properties_map;

class sgnode : public cliproxy {
	friend class group_node;

public:
	enum change_type {
		CHILD_ADDED,
		DELETED,        // sent from destructor
		TRANSFORM_CHANGED,
		SHAPE_CHANGED,
		PROPERTY_CHANGED,
		PROPERTY_DELETED
	};

	sgnode(const std::string &name, const std::string &type, bool group);
	virtual ~sgnode();

	/* copied node doesn't inherit listeners */
	virtual sgnode* clone() const;

	int get_id() const { return id; }
	const std::string &get_name() const { return name; }
	void set_name(const std::string &n) { name = n; }

	const std::string &get_type() const { return type; }

	group_node* get_parent() {
		return parent;
	}

	const group_node* get_parent() const {
		return parent;
	}

	bool is_group() const {
		return group;
	}

	group_node *as_group();
	const group_node *as_group() const;

	void set_trans(char type, const vec3 &t);
	void set_trans(const vec3 &p, const vec3 &r, const vec3 &s);
	vec3 get_trans(char type) const;
	void get_trans(vec3 &p, vec3 &r, vec3 &s) const;
	const transform3 &get_world_trans() const;

	void set_shape_dirty();
	void listen(sgnode_listener *o);
	void unlisten(sgnode_listener *o);
	void get_listeners(std::list<sgnode_listener*> &l) const { l = listeners; }

	const bbox &get_bounds() const;
	vec3 get_centroid() const;
	bool has_descendent(const sgnode *n) const;

	void proxy_use_sub(const std::vector<std::string> &args, std::ostream &os);

	virtual void get_shape_sgel(std::string &s) const = 0;
	virtual void walk(std::vector<sgnode*> &result) = 0;

	virtual void walk_geoms(std::vector<geometry_node*> &g) = 0;
	virtual void walk_geoms(std::vector<const geometry_node*> &g) const = 0;

	// AM: accessors/mutators for node properties
	const string_properties_map  &get_string_properties() const  { return string_props; }
	const numeric_properties_map &get_numeric_properties() const { return numeric_props; }
	void set_property(const std::string& propertyName, const std::string& value);
	void set_property(const std::string& propertyName, double value);
	bool get_property(const std::string& propertyName, std::string& value) const;
	bool get_property(const std::string& propertyName, double& value) const;
	void delete_property(const std::string& propertyName);
	void set_native_property(char type, int dim, double value);

	void adjust_size(std::vector<const sgnode*> targets);

protected:
	void set_bounds(const bbox &b);
	virtual void update_shape() = 0;
	virtual sgnode *clone_sub() const = 0;
	virtual void set_transform_dirty_sub() {}

private:
	void set_transform_dirty();
	void update_transform() const;
	void send_update(change_type t, const std::string& update_info);
	void send_update(change_type t){
		std::string s = "";
		send_update(t, s);
	}

	int         id;
	std::string name;
	std::string type;
	group_node* parent;
	bool        group;
	vec3        pos;
	vec3        rot;
	vec3        scale;
	vec3        centroid;

	mutable bool shape_dirty;

	mutable bbox bounds;
	mutable bool bounds_dirty;

	mutable transform3 wtransform;
	mutable transform3 ltransform;
	mutable bool       trans_dirty;

	std::list<sgnode_listener*> listeners;

	// AM: Maps that hold both numeric and string properties
	string_properties_map string_props;
	numeric_properties_map numeric_props;

};

class group_node : public sgnode {
public:
	group_node(const std::string &name, const std::string &type) : sgnode(name, type, true) {}
	~group_node();

	sgnode* get_child(int i);
	const sgnode *get_child(int i) const;
	bool attach_child(sgnode *c);
	void detach_child(sgnode *c);
	void walk(std::vector<sgnode*> &result);

	int num_children() const {
		return children.size();
	}

	// group nodes have no shape
	void get_shape_sgel(std::string &s) const {}

	void walk_geoms(std::vector<geometry_node*> &g);
	void walk_geoms(std::vector<const geometry_node*> &g) const;

	void proxy_get_children(std::map<std::string, cliproxy*> &c);

private:
	void update_shape();
	void set_transform_dirty_sub();
	sgnode* clone_sub() const;

	std::vector<sgnode*> children;
};

class geometry_node : public sgnode {
public:
	geometry_node(const std::string &name, const std::string &type) : sgnode(name, type, false) {}
	virtual ~geometry_node() {}
	void gjk_support(const vec3 &dir, vec3 &support) const;

	void walk(std::vector<sgnode*> &result) { result.push_back(this); }

	void walk_geoms(std::vector<geometry_node*> &g);
	void walk_geoms(std::vector<const geometry_node*> &g) const;

private:
	virtual void gjk_local_support(const vec3 &dir, vec3 &support) const = 0;
};

class convex_node : public geometry_node {
public:
	convex_node(const std::string &name, const std::string &type, const ptlist &v);

	const ptlist &get_verts() const { return verts; }
	const ptlist &get_world_verts() const;
	void set_verts(const ptlist &v);
	void get_shape_sgel(std::string &s) const;
	void gjk_local_support(const vec3 &dir, vec3 &support) const;

	void proxy_use_sub(const std::vector<std::string> &args, std::ostream &os);

private:
	void set_transform_dirty_sub();
	void update_shape();
	sgnode *clone_sub() const;

	ptlist verts;
	mutable ptlist world_verts;
	mutable bool world_verts_dirty;
};

class ball_node : public geometry_node {
public:
	ball_node(const std::string &name, const std::string &type, double radius);
	void get_shape_sgel(std::string &s) const;

	double get_radius() const {
		return radius;
	}

	void set_radius(double r);
	void gjk_local_support(const vec3 &dir, vec3 &support) const;

	void proxy_use_sub(const std::vector<std::string> &args, std::ostream &os);

private:
	void update_shape();
	sgnode *clone_sub() const;

	double radius;
};

class sgnode_listener {
public:
	virtual void node_update(sgnode *n, sgnode::change_type t, const std::string& update_info) = 0;
};

double convex_distance(const sgnode *n1, const sgnode *n2);
bool intersects(const sgnode *n1, const sgnode *n2);
double overlap(const sgnode* n1, const sgnode* n2);

#endif
