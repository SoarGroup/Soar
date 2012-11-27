#include <iostream>
#include "collision.h"
#include "sgnode.h"

using namespace std;

static const int VERTEX_STRIDE = sizeof(vec3);
static const int TRI_STRIDE = 3 * sizeof(int);

dGeomID get_node_geom(dSpaceID space, const sgnode *n) {
	const convex_node *cn = dynamic_cast<const convex_node*>(n);
	if (cn) {
		const ptlist &verts = cn->get_verts();
		const vector<int> &inds = cn->get_triangles();
		
		dTriMeshDataID mesh = dGeomTriMeshDataCreate();	// need to clean this up somehow
		dGeomTriMeshDataBuildDouble(mesh, &verts[0], VERTEX_STRIDE, verts.size(), &inds[0], inds.size(), TRI_STRIDE);
		return dCreateTriMesh(space, mesh, NULL, NULL, NULL);
	}
	const ball_node *bn = dynamic_cast<const ball_node*>(n);
	if (bn) {
		return dCreateSphere(space, bn->get_radius());
	}
	/*
	 If this is a group node, should make a compound shape in the
	 future.
	*/
	assert(false);
	return NULL;
}

collision_detector::collision_detector()
: dirty(true)
{
	dInitODE();
	space = dSimpleSpaceCreate(NULL);
	dirty = true;
}

collision_detector::~collision_detector() {
	dSpaceDestroy(space);
}

void collision_detector::add_node(const sgnode *n) {
	function_timer t(timers.get_or_add("add-node"));
	assert(object_map.find(n) == object_map.end());
	update_shape(n);
	update_transform(n);
}

void collision_detector::del_node(const sgnode *n) {
	function_timer t(timers.get_or_add("del-node"));
	
	dGeomID geom;
	if(!map_pop(object_map, n, geom)) {
		assert(false);
	}
	dGeomDestroy(geom);
	dirty = true;
}

void collision_detector::update_transform(const sgnode *n) {
	function_timer t(timers.get_or_add("update-transform"));
	
	dGeomID geom;
	if (!map_get(object_map, n, geom)) {
		assert(false);
	}
	
	assert(n->get_trans('s') == vec3(1.0, 1.0, 1.0));
	vec3 p = n->get_trans('p');
	vec4 q1 = n->get_quaternion();
	dQuaternion q2;
	q2[0] = q1(3);
	q2[1] = q1(0);
	q2[2] = q1(1);
	q2[3] = q1(2);
	
	dGeomSetPosition(geom, p(0), p(1), p(2));
	dGeomSetQuaternion(geom, q2);
	dirty = true;
}

void collision_detector::update_shape(const sgnode *n) {
	dGeomID geom;
	if (map_get(object_map, n, geom)) {
		dGeomDestroy(geom);
	}
	geom = get_node_geom(space, n);
	dGeomSetData(geom, const_cast<sgnode*>(n));
	object_map[n] = geom;
	dirty = true;
}

void near_callback(void *data, dGeomID g1, dGeomID g2) {
	static dContactGeom contacts[1];
	
	if (dCollide(g1, g2, 1, contacts, sizeof(dContactGeom)) > 0) {
		const sgnode *n1 = static_cast<const sgnode*>(dGeomGetData(g1));
		const sgnode *n2 = static_cast<const sgnode*>(dGeomGetData(g2));
		collision_table *t = static_cast<collision_table*>(data);
		(*t).insert(make_pair(n1, n2));
	}
}

const collision_table &collision_detector::get_collisions() {
	function_timer t(timers.get_or_add("update"));
	
	if (dirty) {
		results.clear();
		timer &ct = timers.get_or_add("collision");
		dSpaceCollide(space, static_cast<void*>(&results), near_callback);
		ct.stop();
		dirty = false;
	}
	return results;
}

bool intersects(const sgnode *n1, const sgnode *n2) {
	static dContactGeom contacts[1];
	dGeomID g1 = get_node_geom(NULL, n1), g2 = get_node_geom(NULL, n2);
	int num_contacts = dCollide(g1, g2, 1, contacts, sizeof(dContactGeom));
	dGeomDestroy(g1);
	dGeomDestroy(g2);
	return num_contacts > 0;
}
