#ifndef BULLET_SUPPORT_H
#define BULLET_SUPPORT_H

#include <fstream>
#include <btBulletCollisionCommon.h>
#include "common.h"
#include "mat.h"

/* Draws using vis */
class bullet_debug_drawer : public btIDebugDraw {
public:
	bullet_debug_drawer(const char *path);
	
	void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color);
	void drawContactPoint(const btVector3 &pt, const btVector3 &norm, btScalar dist, int time, const btVector3 &color);
	void reportErrorWarning(const char *msg);
	void draw3dText(const btVector3 &p, const char *text);
	void setDebugMode(int mode);
	int getDebugMode() const;
	void reset();
	
private:
	void outputBoxVerts(const btVector3 &min, const btVector3 &max);
	
	std::ofstream fifo;
	int counter;
};

inline btVector3 to_btvec(const vec3 &v) {
	return btVector3(v[0], v[1], v[2]);
}

inline vec3 from_btvec(const btVector3 &v) {
	return vec3(v.x(), v.y(), v.z());
}

btConvexHullShape *ptlist_to_hullshape(const ptlist &pts);

/*
 Calculate distance between closest points on convex hulls. Distance
 will be negative if hulls intersect and represents penetration depth.
*/
double hull_distance(btConvexHullShape *ahull, btConvexHullShape *bhull);

/*
 Convenience method that does hull conversion internally
*/
double hull_distance(const ptlist &a, const ptlist &b);

std::ostream &operator<<(std::ostream &os, const btVector3 &v);

#endif

