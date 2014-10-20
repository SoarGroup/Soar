#ifndef SGNODE_ALGS_H
#define SGNODE_ALGS_H

#include <vector>

class sgnode;
class convex_node;
class scene;

double convex_distance(const sgnode* a, const sgnode* b);

double centroid_distance(const sgnode* a, const sgnode* b);

double axis_distance(const sgnode* a, const sgnode* b, int axis);

double bbox_volume(const sgnode* a);

bool convex_intersects(const sgnode* a, const sgnode* b);

bool bbox_intersects(const sgnode* a, const sgnode* b);

bool bbox_contains(const sgnode* a, const sgnode* b);

double convex_overlap(const sgnode* a, const sgnode* b, int nsamples);

typedef std::pair<convex_node*, bool> view_line;
void calc_view_lines(const sgnode* target, const sgnode* eye, std::vector<view_line>& view_lines);

double convex_occlusion(const sgnode* target, const sgnode* eye, const std::vector<const sgnode*>& occluders);

double convex_occlusion(std::vector<view_line>& view_lines, const std::vector<const sgnode*>& occluders);

void adjust_sgnode_size(sgnode* n, std::vector<const sgnode*> targets);

void adjust_sgnode_size(sgnode* n, scene* scn);

#endif
