#ifndef SGNODE_ALGS_H
#define SGNODE_ALGS_H

#include <vector>

class sgnode;

double convex_distance(const sgnode* n1, const sgnode* n2);
bool intersects(const sgnode* n1, const sgnode* n2);
bool intersects(const sgnode* n, std::vector<const sgnode*> targets);
double overlap(const sgnode* n1, const sgnode* n2);

#endif
