#ifndef COLLISION_H
#define COLLISION_H

#include <map>
#include <utility>
#include "bullet_support.h"

class sgnode;

class collision_detector {
public:
	collision_detector();
	~collision_detector();
	void add_node(sgnode *n);
	void del_node(sgnode *n);
	void update_transform(sgnode *n);
	void update_points(sgnode *n);
	void update(std::vector<std::pair<sgnode*, sgnode*> > &collisions);
	
	const timer_set &get_timers() const { return timers; }
	
private:
	void init();
	
	btCollisionConfiguration *config;
	btCollisionDispatcher    *dispatcher;
	btBroadphaseInterface    *broadphase;
	btCollisionWorld         *cworld;
	bullet_debug_drawer      *drawer;
	
	std::map<sgnode*, btCollisionObject*> object_map;
	
	enum Timers {ADD_NODE_T, DEL_NODE_T, UPDATE_TRANSFORM_T, UPDATE_POINTS_T, UPDATE_T, COLLISION_T};
	timer_set timers;
};

#endif
