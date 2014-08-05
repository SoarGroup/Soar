#ifndef SGNODE_ALGS_H
#define SGNODE_ALGS_H

#include <vector>

class sgnode;

double convex_distance(const sgnode* a, const sgnode* b);

double centroid_distance(const sgnode* a, const sgnode* b);

double distance_on_axis(const sgnode* a, const sgnode* b, int axis);

double bbox_volume(const sgnode* a);

bool convex_intersect(const sgnode* a, const sgnode* b);

bool bbox_intersect(const sgnode* a, const sgnode* b);

bool bbox_contain(const sgnode* a, const sgnode* b);

#endif
