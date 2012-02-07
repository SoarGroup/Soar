/*
 Collision detection using Bullet physics. Currently I'm ignoring the
 broadphase pruning, so this is not as efficient as it can be.
*/
#include <iostream>
#include <map>
#include "linalg.h"
#include "filter.h"
#include "filter_table.h"
#include "common.h"
#include "bullet_support.h"

using namespace std;

const bool do_debug_draw = false;

struct node_info {
	sgnode *node;
	ptlist vertices;
	btCollisionObject *object;
};

struct result_info {
	bool oldval;
	bool newval;
	filter_val *fval;
};

typedef map<filter_val*, node_info> input_table_t;
typedef pair<filter_val*, filter_val*> fval_pair;
typedef map<fval_pair, result_info> result_table_t;

void update_transforms(node_info &info) {
	vec3 rpy = info.node->get_trans('r');
	btQuaternion q;
	q.setEuler(rpy[0], rpy[1], rpy[2]);
	info.object->getWorldTransform().setOrigin(to_btvec(info.node->get_trans('p')));
	info.object->getWorldTransform().setRotation(q);
	info.object->getCollisionShape()->setLocalScaling(to_btvec(info.node->get_trans('s')));
}

class intersect_filter : public filter {
public:
	intersect_filter(filter_input *input) 
	: filter(input), drawer(NULL)
	{
		btVector3 worldAabbMin(-1000,-1000,-1000);
		btVector3 worldAabbMax(1000,1000,1000);
		
		config = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(config);
		broadphase = new btSimpleBroadphase();
		//broadphase = new btAxisSweep3(worldAabbMin, worldAabbMax);
		cworld = new btCollisionWorld(dispatcher, broadphase, config);
		
		if (do_debug_draw) {
			drawer = new bullet_debug_drawer("/tmp/dispfifo");
			cworld->setDebugDrawer(drawer);
		}
	}
	
	~intersect_filter() {
		delete cworld;
		delete dispatcher;
		delete broadphase;
		delete config;
		delete drawer;
	}
	
	bool update_results() {
		const filter_input *input = get_input();
		result_table_t::iterator j;
		filter_val *av, *bv;
		
		for (int i = input->first_added(); i < input->num_current(); ++i) {
			const filter_param_set *params = input->get_current(i);
			if (!map_get<string, filter_val*>(*params, "a", av) ||
			    !map_get<string, filter_val*>(*params, "b", bv) )
			{
				set_error("missing parameter(s)");
				return false;
			}
			result_info &rp = results[make_pair(av, bv)];
			rp.oldval = false;
			rp.newval = false;
			rp.fval = new filter_val_c<bool>(false);
			add_result(rp.fval, params);
			add_node(av);
			add_node(bv);
		}
		for (int i = 0; i < input->num_removed(); ++i) {
			const filter_param_set *params = input->get_removed(i);
			if (!map_get<string, filter_val*>(*params, "a", av) ||
			    !map_get<string, filter_val*>(*params, "b", bv))
			{
				set_error("missing parameter(s)");
				return false;
			}
			result_info rp;
			if (!map_pop(results, make_pair(av, bv), rp)) {
				assert(false);
			}
			remove_result(rp.fval);
			del_node(av);
			del_node(bv);
		}
		for (int i = 0; i < input->num_changed(); ++i) {
			const filter_param_set *params = input->get_changed(i);
			if (!map_get<string, filter_val*>(*params, "a", av) ||
			    !map_get<string, filter_val*>(*params, "b", bv))
			{
				set_error("missing parameter(s)");
				return false;
			}
			change_node(av);
			change_node(bv);
		}
		
		/*
		for (j = results.begin(); j != results.end(); ++j) {
			btCollisionObject *o1 = input_table[j->first.first].object;
			btCollisionObject *o2 = input_table[j->first.second].object;
			j->second.oldval = j->second.newval;
			j->second.newval = false;
			cworld->contactPairTest(o1, o2, callback);
		}
		*/
		
		for (j = results.begin(); j != results.end(); ++j) {
			j->second.oldval = j->second.newval;
			j->second.newval = false;
		}
		
		cworld->performDiscreteCollisionDetection();
		int num_manifolds = dispatcher->getNumManifolds();
		for (int k = 0; k < num_manifolds; ++k) {
			btPersistentManifold *m = dispatcher->getManifoldByIndexInternal(k);
			int numcontacts = m->getNumContacts();
			if (numcontacts == 0) {
				continue;
			}
			btCollisionObject *a = static_cast<btCollisionObject*>(m->getBody0());
			btCollisionObject *b = static_cast<btCollisionObject*>(m->getBody1());
			filter_val *af = static_cast<filter_val*>(a->getUserPointer());
			filter_val *bf = static_cast<filter_val*>(b->getUserPointer());
			add_collision(af, bf);
		}
		
		for (j = results.begin(); j != results.end(); ++j) {
			result_info &r = j->second;
			if (r.oldval != r.newval) {
				set_filter_val(r.fval, r.newval);
				change_result(r.fval);
			}
		}
		
		if (drawer) {
			drawer->reset();
			cworld->debugDrawWorld();
		}
		return true;
	}
	
	void add_collision(filter_val *a, filter_val *b) {
		fval_pair p1 = make_pair(a, b), p2 = make_pair(b, a);
		result_table_t::iterator i;
		
		/* have to check both orderings */
		if ((i = results.find(p1)) == results.end()) {
			i = results.find(p2);
		}
		
		if (i != results.end()) {
			i->second.newval = true;
		}
	}
	
private:

	/*
	class collision_callback : public btCollisionWorld::ContactResultCallback {
	public:
		collision_callback(intersect_filter *f) : f(f) {}
		
		virtual	btScalar addSingleResult(btManifoldPoint& cp, 
		                                 const btCollisionObject* obj1,
		                                 int partId1,
		                                 int index1,
		                                 const btCollisionObject* obj2,
		                                 int partId2,
		                                 int index2)
		{
			filter_val *v1, *v2;
			v1 = static_cast<filter_val*>(obj1->getUserPointer());
			v2 = static_cast<filter_val*>(obj2->getUserPointer());
			f->add_collision(v1, v2);
			return 0.f;
		}
		
	private:
		intersect_filter *f;
	};
	*/
	
	void add_object(filter_val *v, node_info &info) {
		info.object = new btCollisionObject();
		info.object->setUserPointer(static_cast<void*>(v));
		info.node->get_local_points(info.vertices);
		info.object->setCollisionShape(ptlist_to_hullshape(info.vertices));
		update_transforms(info);
		cworld->addCollisionObject(info.object);
	}
	
	void remove_object(node_info &info) {
		cworld->removeCollisionObject(info.object);
		delete info.object->getCollisionShape();
		delete info.object;
	}
	
	bool add_node(filter_val *v) {
		sgnode *n;
		
		if (input_table.find(v) != input_table.end()) {
			return true;
		}
		
		if (!get_filter_val(v, n)) {
			return false;
		}
		
		node_info &info = input_table[v];
		info.node = n;
		add_object(v, info);
		return true;
	}
	
	void del_node(filter_val *v) {
		input_table_t::iterator i = input_table.find(v);
		assert(i != input_table.end());
		
		node_info &info = i->second;
		remove_object(info);
		input_table.erase(v);
	}
	
	bool change_node(filter_val *v) {
		sgnode *newnode;
		
		if (!get_filter_val(v, newnode)) {
			return false;
		}
		
		input_table_t::iterator i = input_table.find(v);
		assert(i != input_table.end());
		node_info &info = i->second;
		if (info.node != newnode) {
			info.node = newnode;
			remove_object(info);
			add_object(v, info);
		} else {
			ptlist newverts;
			info.node->get_local_points(newverts);
			if (info.vertices != newverts) {
				cout << "SETTING NEW VERTICES FOR " << info.node->get_name() << endl;
				copy(newverts.begin(), newverts.end(), ostream_iterator<vec3>(cout, " / "));
				cout << endl;
				
				assert(newverts.size() == info.vertices.size());
				info.vertices = newverts;
				delete info.object->getCollisionShape();
				info.object->setCollisionShape(ptlist_to_hullshape(info.vertices));
			}
			update_transforms(info);
		}
		return true;
	}
	
	btCollisionConfiguration *config;
	btCollisionDispatcher    *dispatcher;
	btBroadphaseInterface    *broadphase;
	btCollisionWorld         *cworld;
	bullet_debug_drawer      *drawer;
	
	input_table_t      input_table;
	result_table_t     results;
};

filter *make_intersect_filter(scene *scn, filter_input *input) {
	return new intersect_filter(input);
}

bool standalone_intersect(scene *scn, const vector<string> &args) {
	bool collide = false;
	
	btDefaultCollisionConfiguration config;
	btCollisionDispatcher dispatcher(&config);
	btSimpleBroadphase broadphase;
	btCollisionWorld world(&dispatcher, &broadphase, &config);
	
	vector<btCollisionObject*> objs;
	for (int i = 0; i < args.size(); ++i) {
		sgnode *n = scn->get_node(args[i]);
		btCollisionObject *obj = new btCollisionObject();
		
		ptlist verts;
		n->get_local_points(verts);
		btCollisionShape *shape = ptlist_to_hullshape(verts);
		shape->setLocalScaling(to_btvec(n->get_trans('s')));
		obj->setCollisionShape(shape);
		
		obj->getWorldTransform().setOrigin(to_btvec(n->get_trans('p')));
		
		vec3 rpy = n->get_trans('r');
		btQuaternion q;
		q.setEuler(rpy[0], rpy[1], rpy[2]);
		obj->getWorldTransform().setRotation(q);
		
		world.addCollisionObject(obj);
		objs.push_back(obj);
	}
	
	world.performDiscreteCollisionDetection();
	int num_manifolds = dispatcher.getNumManifolds();
	for (int k = 0; k < num_manifolds; ++k) {
		btPersistentManifold *m = dispatcher.getManifoldByIndexInternal(k);
		if (m->getNumContacts() > 0) {
			collide = true;
			break;
		}
	}
	
	vector<btCollisionObject*>::iterator j;
	for (j = objs.begin(); j != objs.end(); ++j) {
		delete (**j).getCollisionShape();
		delete *j;
	}
	
	return collide;
}

filter_table_entry intersect_fill_entry() {
	filter_table_entry e;
	e.name = "intersect";
	e.parameters.push_back("a");
	e.parameters.push_back("b");
	e.create = &make_intersect_filter;
	e.calc = &standalone_intersect;
	e.possible_args = &all_node_pairs_unordered_no_repeat;
	return e;
}
