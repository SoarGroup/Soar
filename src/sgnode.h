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
	
	sgnode(std::string name);
	virtual ~sgnode();
	
	/* copied node doesn't inherit listeners */
	virtual sgnode* copy() const = 0;

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

	group_node *as_group();
	const group_node *as_group() const;
	
	void set_trans(char type, const vec3 &t);
	void set_trans(const vec3 &p, const vec3 &r, const vec3 &s);
	vec3 get_trans(char type) const;
	void get_trans(vec3 &p, vec3 &r, vec3 &s) const;
	void copy_trans(const sgnode *n);
	
	void set_shape_dirty();
	void listen(sgnode_listener *o);
	void unlisten(sgnode_listener *o);
	const bbox &get_bounds() const;
	const vec3 &get_centroid() const;

	virtual bool is_group() const = 0;
	virtual void get_shape_sgel(std::string &s) const = 0;
	virtual void walk(std::vector<sgnode*> &result) = 0;
	
protected:
	const transform3 &get_world_trans() const;
	void set_bounds(const bbox &b);
	virtual void update_shape() = 0;
	virtual void set_transform_dirty_derived() {}
	
private:
	void set_transform_dirty();
	void update_transform();
	void send_update(change_type t, int added=-1);
	
	std::string name;
	group_node* parent;
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
	group_node(std::string name) : sgnode(name) {}
	~group_node();
	
	sgnode* copy() const;
	sgnode* get_child(int i);
	const sgnode *get_child(int i) const;
	bool attach_child(sgnode *c);
	void detach_child(sgnode *c);
	void walk(std::vector<sgnode*> &result);

	int num_children() const {
		return children.size();
	}

	bool is_group() const { return true; }
	
	// group nodes have no shape
	void get_shape_sgel(std::string &s) const {}
	
private:
	void update_shape();
	void set_transform_dirty_derived();
	
	std::vector<sgnode*> children;
};

class convex_node : public sgnode {
public:
	convex_node(const std::string &name, const ptlist &points);
	
	const ptlist &get_local_points() const;
	const ptlist &get_world_points() const;
	void set_local_points(const ptlist &pts);
	void get_shape_sgel(std::string &s) const;
	void walk(std::vector<sgnode*> &result);
	sgnode *copy() const;
	
	bool is_group() const { return false; }
	
private:
	void update_shape();
	
	ptlist points;
	ptlist world_points;
	bool   dirty;

};

class sgnode_listener {
public:
	virtual void node_update(sgnode *n, sgnode::change_type t, int added_child) = 0;
};

#endif
