#ifndef SGNODE_H
#define SGNODE_H

/* native scene graph implementation */

#include <vector>
#include <list>
#include "linalg.h"

class sgnode_listener;

class sgnode {
public:
	enum change_type {
		CHILD_ADDED,
		DELETED,        // sent from destructor
		TRANSFORM_CHANGED,
		POINTS_CHANGED
	};
	
	sgnode(std::string name);
	sgnode(std::string name, const ptlist &points);
	~sgnode();
	
	/* copied node doesn't inherit listeners */
	sgnode*     copy() const;

	std::string get_name() const;
	void        set_name(std::string nm);
	bool        is_group() const;

	sgnode*     get_parent();
	int         num_children() const;
	sgnode*     get_child(int i);
	void        walk(std::list<sgnode*> &result);
	bool        attach_child(sgnode *c);
	
	void        set_trans(char type, const vec3 &t);
	void        set_trans(const vec3 &p, const vec3 &r, const vec3 &s);
	vec3        get_trans(char type) const;
	void        get_trans(vec3 &p, vec3 &r, vec3 &s) const;
	
	/*
	 get_local_points and get_world_points intuitively should be
	 const, but are not because they might have to run some lazily
	 deferred updates.
	*/
	void        get_local_points(ptlist &result);
	void        set_local_points(const ptlist &pts);
	void        get_world_points(ptlist &result);
	
	void        listen(sgnode_listener *o);
	void        unlisten(sgnode_listener *o);
	
private:
	void detach_child(sgnode *c);
	void set_transform_dirty();
	void update_transform();
	void set_points_dirty();
	void update_points();
	void send_update(change_type t, int added=-1);
	
	std::string          name;
	sgnode*              parent;
	ptlist               points;
	std::vector<sgnode*> children;
	bool                 isgroup;
	vec3                 pos;
	vec3                 rot;
	vec3                 scale;
	transform3           wtransform;
	transform3           ltransform;
	
	bool                 tdirty;       // transforms dirty
	bool                 pdirty;       // convex hull dirty
	
	std::list<sgnode_listener*> listeners;
};

class sgnode_listener {
public:
	virtual void node_update(sgnode *n, sgnode::change_type t, int added_child) = 0;
};

#endif
