#ifndef SGNODE_H
#define SGNODE_H

/* native scene graph implementation */

#include <vector>
#include <list>
#include "common.h"
#include "linalg.h"

class sgnode_listener;
class group_node;

class sgnode {
	friend class group_node;
	
public:
	enum change_type {
		CHILD_ADDED,
		DELETED,        // sent from destructor
		TRANSFORM_CHANGED,
		SHAPE_CHANGED
	};
	
	sgnode(std::string name, bool group);
	virtual ~sgnode();
	
	/* copied node doesn't inherit listeners */
	virtual sgnode* clone() const;

	const std::string &get_name() const {
		return name;
	}
	
	void set_name(const std::string &n) {
		name = n;
	}

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
	
	void set_shape_dirty();
	void listen(sgnode_listener *o);
	void unlisten(sgnode_listener *o);
	const bbox &get_bounds() const;
	vec3 get_centroid() const;

	virtual void get_shape_sgel(std::string &s) const = 0;
	virtual void walk(std::vector<sgnode*> &result) = 0;
	
protected:
	const transform3 &get_world_trans() const;
	void set_bounds(const bbox &b);
	virtual void update_shape() = 0;
	virtual sgnode *clone_sub() const = 0;
	virtual void set_transform_dirty_sub() {}
	
private:
	void set_transform_dirty();
	void update_transform();
	void send_update(change_type t, int added=-1);
	
	std::string name;
	group_node* parent;
	bool        group;
	vec3        pos;
	vec3        rot;
	vec3        scale;
	transform3  wtransform;
	transform3  ltransform;
	bool        shape_dirty;
	bool        trans_dirty;
	bbox        bounds;
	vec3        centroid;
	
	std::list<sgnode_listener*> listeners;
};

class group_node : public sgnode {
public:
	group_node(std::string name) : sgnode(name, true) {}
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
	
private:
	void update_shape();
	void set_transform_dirty_sub();
	sgnode* clone_sub() const;
	
	std::vector<sgnode*> children;
};

class geometry_node : public sgnode {
public:
	geometry_node(const std::string &name) : sgnode(name, false) {}
	virtual ~geometry_node() {}
	void walk(std::vector<sgnode*> &result) { result.push_back(this); }
};

class convex_node : public geometry_node {
public:
	convex_node(const std::string &name, const ptlist &points);
	
	const ptlist &get_local_points() const;
	const ptlist &get_world_points() const;
	void set_local_points(const ptlist &pts);
	void get_shape_sgel(std::string &s) const;
	void set_transform_dirty_sub();
	
private:
	void update_shape();
	sgnode *clone_sub() const;
	
	ptlist points;
	ptlist world_points;
	bool   dirty;

};

class ball_node : public geometry_node {
public:
	ball_node(const std::string &name, double radius);
	void get_shape_sgel(std::string &s) const;
	
	double get_radius() const {
		return radius;
	}
	
	void set_radius(double r);
	
private:
	void update_shape();
	sgnode *clone_sub() const;
	
	double radius;
};

class sgnode_listener {
public:
	virtual void node_update(sgnode *n, sgnode::change_type t, int added_child) = 0;
};

#endif
