#include <BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h>
#include <BulletCollision/NarrowPhaseCollision/btPointCollector.h>
#include <BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btConvexPenetrationDepthSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h>
#include "bullet_support.h"
#include "linalg.h"
#include "common.h"

using namespace std;

bullet_debug_drawer::bullet_debug_drawer(const char *path)
: fifo(path), counter(0) 
{
	reset();
}

void bullet_debug_drawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
	fifo << "bullet n " << counter++ << " c " << color << " v " << from << " " << to << endl;
	fifo.flush();
}

void bullet_debug_drawer::drawContactPoint(const btVector3 &pt, const btVector3 &norm, btScalar dist, int time, const btVector3 &color) {
	fifo << "bullet n " << counter++ << " c " << color << " v " << pt << endl;
	fifo.flush();
}

void bullet_debug_drawer::reportErrorWarning(const char *msg) {
	fifo << "bullet l " << msg << endl;
	fifo.flush();
}

void bullet_debug_drawer::draw3dText(const btVector3 &p, const char *text) {
	fifo << "bullet t " << counter++ << " " << p << " " << text << endl;
	fifo.flush();
}

void bullet_debug_drawer::outputBoxVerts(const btVector3 &min, const btVector3 &max) {
	float pt[3];
	for (int i = 0; i < 8; i++) {
		pt[0] = min.x() + ((max.x() - min.x()) * (i & 1));
		pt[1] = min.y() + ((max.y() - min.y()) * ((i >> 1) & 1));
		pt[2] = min.z() + ((max.z() - min.z()) * ((i >> 2) & 1));
		fifo << " ";
		copy(pt, pt + 3, ostream_iterator<float>(fifo, " "));
	}
}

void bullet_debug_drawer::setDebugMode(int mode) {
}

int bullet_debug_drawer::getDebugMode() const {
	return DBG_DrawWireframe | DBG_DrawContactPoints | DBG_DrawText | DBG_DrawFeaturesText; // | DBG_DrawAabb | DBG_DrawConstraints | DBG_DrawConstraintLimits;
}

void bullet_debug_drawer::reset() {
	fifo << "bullet r" << endl;
	fifo.flush();
	counter = 0;
}

btConvexHullShape *ptlist_to_hullshape(const ptlist &pts) {
	btConvexHullShape *s = new btConvexHullShape(reinterpret_cast<const btScalar*>(&pts[0]), pts.size(), sizeof(vec3));
	s->setMargin(0.0001);
	return s;
}

float hull_distance(const ptlist &a, const ptlist &b) {
	btConvexHullShape *ahull = ptlist_to_hullshape(a);
	btConvexHullShape *bhull = ptlist_to_hullshape(b);
	float d = hull_distance(ahull, bhull);
	delete ahull;
	delete bhull;
	return d;
}

float hull_distance(btConvexHullShape *ahull, btConvexHullShape *bhull) {
	btVoronoiSimplexSolver ssolver;
	btGjkEpaPenetrationDepthSolver pdsolver;
	btPointCollector out;
	
	btGjkPairDetector convexConvex(ahull, bhull, &ssolver, &pdsolver); 
	btGjkPairDetector::ClosestPointInput input;
	input.m_transformA.setIdentity();
	input.m_transformB.setIdentity();
	convexConvex.getClosestPoints(input, out, 0);
	
	assert(out.m_hasResult);
 	return out.m_distance;
}

ostream &operator<<(ostream &os, const btVector3 &v) {
	os << v.x() << " " << v.y() << " " << v.z();
	return os;
}
