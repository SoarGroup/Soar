#ifndef COLLISION_H
#define COLLISION_H

#include <map>
#include <set>
#include <utility>
#define dDOUBLE
#include <ode/ode.h>
#include "timer.h"

class sgnode;
class geometry_node;

typedef std::set<std::pair<const sgnode*, const sgnode*> > collision_table;

bool intersects(const sgnode *n1, const sgnode *n2);
double convex_distance(const geometry_node *n1, const geometry_node *n2);

class collision_detector {
public:
	collision_detector();
	~collision_detector();
	void add_node(const sgnode *n);
	void del_node(const sgnode *n);
	void update_transform(const sgnode *n);
	void update_shape(const sgnode *n);
	const collision_table &get_collisions();
	
	const timer_set &get_timers() const { return timers; }
	
private:
	dSpaceID space;
	std::map<const sgnode*, dGeomID> object_map;
	bool dirty;
	collision_table results;
	
	timer_set timers;
};

#endif
