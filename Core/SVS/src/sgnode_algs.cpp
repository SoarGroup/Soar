#include "sgnode_algs.h"

#include <assert.h>
#include <list>
#include <vector>
#include <iterator>
#include <algorithm>
#include "sgnode.h"
#include "ccd/ccd.h"
#include "params.h"

#include <iostream>
using namespace std;

typedef std::vector<const sgnode*> c_sgnode_list;
typedef std::set<const sgnode*> c_sgnode_set;
typedef std::vector<const geometry_node*> c_geom_node_list;
typedef std::vector<view_line> view_line_list;

/****** Support functions for CCD *******/

void point_ccd_support(const void* obj, const ccd_vec3_t* dir, ccd_vec3_t* v)
{
    const vec3* point = static_cast<const vec3*>(obj);
    for (int i = 0; i < 3; ++i)
    {
        v->v[i] = (*point)(i);
    }
}

void geom_ccd_support(const void* obj, const ccd_vec3_t* dir, ccd_vec3_t* v)
{
    vec3 d, support;
    const geometry_node* n = static_cast<const geometry_node*>(obj);
    
    for (int i = 0; i < 3; ++i)
    {
        d(i) = dir->v[i];
    }
    n->gjk_support(d, support);
    for (int i = 0; i < 3; ++i)
    {
        v->v[i] = support(i);
    }
}

// Gets convex_distance between two geometry nodes //
double geom_convex_dist(const geometry_node* a, const geometry_node* b)
{
    ccd_t ccd;
    double dist;
    
    CCD_INIT(&ccd);
    ccd.support1       = geom_ccd_support;
    ccd.support2       = geom_ccd_support;
    ccd.max_iterations = 100;
    ccd.dist_tolerance = INTERSECT_THRESH;
    
    dist = ccdGJKDist(a, b, &ccd);
    return dist > 0.0 ? dist : 0.0;
}

// Gets convex distance between a point and a geometry node //
double point_geom_convex_dist(const vec3& p, const geometry_node* g)
{
    ccd_t ccd;
    double dist;
    
    CCD_INIT(&ccd);
    ccd.support1       = point_ccd_support;
    ccd.support2       = geom_ccd_support;
    ccd.max_iterations = 100;
    ccd.dist_tolerance = INTERSECT_THRESH;
    
    dist = ccdGJKDist(&p, g, &ccd);
    return dist > 0.0 ? dist : 0.0;
}

// Gets the convex distance between two sgnodes //
double convex_distance(const sgnode* a, const sgnode* b)
{
    vector<const geometry_node*> g1, g2;
    vector<double> dists;
    vec3 c;
    
    if (a == b || a->has_descendent(b) || b->has_descendent(a))
    {
        return 0.0;
    }
    
    a->walk_geoms(g1);
    b->walk_geoms(g2);
    
    if (g1.empty() && g2.empty())
    {
        return (a->get_centroid() - b->get_centroid()).norm();
    }
    
    if (g1.empty())
    {
        dists.reserve(g2.size());
        c = a->get_centroid();
        for (int i = 0, iend = g2.size(); i < iend; ++i)
        {
            dists.push_back(point_geom_convex_dist(c, g2[i]));
        }
    }
    else if (g2.empty())
    {
        dists.reserve(g1.size());
        c = b->get_centroid();
        for (int i = 0, iend = g1.size(); i < iend; ++i)
        {
            dists.push_back(point_geom_convex_dist(c, g1[i]));
        }
    }
    else
    {
        dists.reserve(g1.size() * g2.size());
        for (int i = 0, iend = g1.size(); i < iend; ++i)
        {
            for (int j = 0, jend = g2.size(); j < jend; ++j)
            {
                dists.push_back(geom_convex_dist(g1[i], g2[j]));
            }
        }
    }
    return *min_element(dists.begin(), dists.end());
}

// Returns the euclidean distance between the centroids of two nodes //
double centroid_distance(const sgnode* a, const sgnode* b)
{
    vec3 ca = a->get_centroid();
    vec3 cb = b->get_centroid();
    return (cb - ca).norm();
}

/*
 * Returns the distance between two nodes along a given axis
 *   using their bounding boxes
 *
 * Returns a negative distance if node a is higher than node b
 * Returns a positive distance if node a is lower than node b
 * Returns 0 if node a and node b overlap on the axis
 *
 * Axis should be 0 for x-axis, 1 for y-axis, 2 for z-axis 
 */
double axis_distance(const sgnode* a, const sgnode* b, int axis)
{
    if (axis < 0 || axis > 2)
    {
        return 0;
    }
    double mina = a->get_bounds().get_min()[axis];
    double maxa = a->get_bounds().get_max()[axis];
    double minb = b->get_bounds().get_min()[axis];
    double maxb = b->get_bounds().get_max()[axis];
    
    if (minb > maxa)
    {
        return (minb - maxa);
    }
    else if (maxb < mina)
    {
        return (maxb - mina);
    }
    else
    {
        return 0;
    }
}

// Returns the volume of the node's bounding box //
double bbox_volume(const sgnode* a)
{
    bbox boxa = a->get_bounds();
    return boxa.get_volume();
}

// Returns true if the convex hull of node a 
//   intersects the convex hull of node b
bool convex_intersects(const sgnode* a, const sgnode* b)
{
    if (a->get_bounds().intersects(b->get_bounds()))
    {
        return convex_distance(a, b) < INTERSECT_THRESH;
    }
    return false;
}

bool convex_intersects(const sgnode* n, const std::vector<const sgnode*>& intersectors){
	std::vector<const sgnode*>::const_iterator i;
	for(i = intersectors.begin(); i != intersectors.end(); i++){
		if(convex_intersects(n, *i)){
			return true;
		}
	}
	return false;
}

// Returns true if the bounding box of node a
//   intersects the bounding box of node b
bool bbox_intersects(const sgnode* a, const sgnode* b)
{
    bbox boxa = a->get_bounds();
    bbox boxb = b->get_bounds();
    return boxa.intersects(boxb);
}

// Returns true if the bounding box of node a 
//   contains the bounding box of node b
bool bbox_contains(const sgnode* a, const sgnode* b)
{
    bbox boxa = a->get_bounds();
    bbox boxb = b->get_bounds();
    return boxa.contains(boxb);
}

/*  
 * Returns the estimated percentage of node n1 
 *   that is contained within node n2
 *   using random sampling
 *
 * Result = # Samples that are contained in n1 and n2 / # Samples contained in n1
 *   (where it tries to get the requested number of samples)
 */
double convex_overlap(const sgnode* n1, const sgnode* n2, int nsamples){
	if(n1 == n2 || n1->has_descendent(n2) || n2->has_descendent(n1)){
		// Something is weird, the given nodes are part of the same object
		return 0;
	}

	vector<const geometry_node*> g1, g2;
	n1->walk_geoms(g1);
	n2->walk_geoms(g2);

	if(g2.empty()){
		// Node 2 is a point, so return 0
		return 0;
	}

	if(g1.empty()){
		// Node 1 is a point
		vec3 pt = n1->get_centroid();
		for(int i = 0, iend = g2.size(); i < iend; i++){
			double dist = point_geom_convex_dist(pt, g2[i]);
			//std::cout << "  pt-geom dist: " << dist << std::endl;
			if(dist <= 0){
				// Node 1's point is contained within node 2
				return 1;
			}
		}
		// Node 1's point is not contained within node 2
		return 0;
	}

	// Early exit, first check if they intersect at all
	double dist = convex_distance(n1, n2);
	if(dist > 0){
		// No intersection
		return 0;
	}

	ccd_t ccd;

	CCD_INIT(&ccd);
	ccd.support1 = point_ccd_support;
	ccd.support2 = geom_ccd_support;
	ccd.max_iterations = 100;
	ccd.dist_tolerance = INTERSECT_THRESH;

	bbox bounds = n1->get_bounds();

	int numSamples = 0;
	int numIntersections = 0;
	int numIters = 0;

	// Generate random points within node 1, and test if within node 2
	while(numSamples < nsamples && numIters < 100000){
		numIters++;
		vec3 randPt = bounds.get_random_point();

		bool inNode1 = false;
		for(int i = 0, iend = g1.size(); i < iend; i++){
			double dist = ccdGJKDist(&randPt, g1[i], &ccd);
			if(dist <= 0){
				inNode1 = true;
				break;
			}
		}
		if(!inNode1){
			continue;
		}

		numSamples++;

		bool inNode2 = false;
		for(int j = 0, jend = g2.size(); j < jend; j++){
			double dist = ccdGJKDist(&randPt, g2[j], &ccd);
			if(dist <= 0){
				inNode2 = true;
				break;
			}
		}
		if(!inNode2){
			continue;
		}

		numIntersections++;
	}

	if(numSamples == 0){
		return 0;
	} else {
		return numIntersections / (double)numSamples;
	}
}


// Creates a view_line consisting of a convex node with the given name
//   which represents a line segment between the two given points
view_line create_view_line(const string& name, const vec3& p1, const vec3& p2){
	vec3 dPosOver2 = (p2 - p1)/2;	// Vector from eye to vertex
	vec3 center = p1 + dPosOver2;	// Point halfway between eye and vertex

	ptlist linePoints;
	linePoints.push_back(dPosOver2);
	linePoints.push_back(-dPosOver2);

	convex_node* line = new convex_node(name, linePoints);
	line->set_trans('p', center);

	return view_line(line, false);
}

// Build up a list of view lines that go from the eye point to each vertex in the target object
// view_line.first is a convex_node that actually represents the line
// view_line.second is a bool which is T if that view line is occluded by another object
void calc_view_lines(const sgnode* target, const sgnode* eye, view_line_list& view_lines){
	vec3 eyePos = eye->get_centroid();
//	std::cout << "EYE: " << eyePos[0] << ", " << eyePos[1] << ", " << eyePos[2] << std::endl;

	// Create a view_line for the centroid
	//std::string name = "_centroid_line_";
	//view_lines.push_back(create_view_line(name, eyePos, target->get_centroid()));

	c_geom_node_list geom_nodes;
	target->walk_geoms(geom_nodes);

	// Go through each vertex in the node and create a view_line to that vertex
	for(c_geom_node_list::const_iterator i = geom_nodes.begin(); i != geom_nodes.end(); i++){
		const convex_node* c_node = dynamic_cast<const convex_node*>(*i);
		if(c_node){
			const ptlist& verts = c_node->get_world_verts();
			for(ptlist::const_iterator i = verts.begin(); i != verts.end(); i++){
				//std::cout << "Point " << view_lines.size() << ": " << (*i)[0] << ", " << (*i)[1] << ", " << (*i)[2] << endl;
				std::string name = "_temp_line_" + tostring(view_lines.size());
				view_lines.push_back(create_view_line(name, eyePos, *i));
			}
		}
	}
}


// Returns the percentage of given view lines are intersected by an object in occludingNodes
double convex_occlusion(view_line_list& view_lines, const c_sgnode_list& occluders){
	if(view_lines.size() == 0){
		return 0;
	}
	if(occluders.size() == 0){
		return 0;
	}

	// Go through every other object in the given set and see if it occludes any view lines
	int num_occluded = 0;

	for(view_line_list::iterator i = view_lines.begin(); i != view_lines.end(); i++){
		i->second = false;
	}

	for(c_sgnode_list::const_iterator i = occluders.begin(); i != occluders.end(); i++){
		const sgnode* n = *i;
		//std::cout << "Testing Object " << n->get_id() << std::endl;
		for(view_line_list::iterator j = view_lines.begin(); j != view_lines.end(); j++){
			view_line& view_line = *j;
			if(view_line.second){
				// Already occluded, don't bother checking again
				continue;
			}
			double dist = convex_distance(n, view_line.first);
			if(dist <= 0){
				if(n->get_id() == "arm"){
					//std::cout << "ARM OCCLUSION!" << std::endl;
				}
				//std::cout << "Occlusion detected" << std::endl;
				//std::cout << "  " << j->first->get_id() << std::endl;
				//std::cout << "  " << n->get_id() << std::endl; 

				view_line.second = true;
				num_occluded++;
			}
		}
	}

	// Count the number of view lines occluded and return the fraction
	return ((double)num_occluded)/view_lines.size();
}

double convex_occlusion(const sgnode* a, const sgnode* eye, const std::vector<const sgnode*>& occluders){
	view_line_list view_lines;
	calc_view_lines(a, eye, view_lines);
	return convex_occlusion(view_lines, occluders);
}

///////////////// adjusting nodes ///////////////
// Used to make sure bounding boxes dont overlap
vec3 adjust_on_dims(sgnode* n, std::vector<const sgnode*> targets, int d1, int d2, int d3){
	vec3 scale = n->get_trans('s');
	vec3 tempScale = scale;
	//cout << "Adjusting on dims: " << d1 << ", " << d2 << ", " << d3 << endl;

	// Simple binary search, finds it within 1%
	double min = 0.001, max = 1.0;
	for(int i = 0; i < 8; i++){
		double s = (max + min)/2;
		tempScale[d1] = scale[d1] * s;
		tempScale[d2] = scale[d2] * s;
		tempScale[d3] = scale[d3] * s;
		n->set_trans('s', tempScale);
		//cout << "  Test " << s << ": ";
		if(convex_intersects(n, targets)){
			//cout << "I" << endl;
			max = s;
		} else {
			//cout << "N" << endl;
			min = s;
		}
	}

	tempScale[d1] = scale[d1] * min * .98;
	tempScale[d2] = scale[d2] * min * .98;
	tempScale[d3] = scale[d3] * min * .98;
	//cout << "Final Result: " << min << endl;
	return tempScale;
}

vec3 adjust_single_dim(sgnode* n, std::vector<const sgnode*> targets, int dim){
	return adjust_on_dims(n, targets, dim, dim, dim);
}

vec3 adjust_two_dims(sgnode* n, std::vector<const sgnode*> targets, int dim){
	return adjust_on_dims(n, targets, (dim+1)%3, (dim+2)%3, (dim+2)%3);
}

vec3 adjust_all_dims(sgnode* n, std::vector<const sgnode*> targets){
	return adjust_on_dims(n, targets, 0, 1, 2);
}

void adjust_sgnode_size(sgnode* n, std::vector<const sgnode*> targets){
	std::vector<const sgnode*> intersectors;
	std::vector<const sgnode*>::iterator i;
	// Find all the nodes that actually intersect the original sized object
	//cout << "Intersectors: " << endl;
	for(i = targets.begin(); i != targets.end(); i++){
		if(*i == n){
			continue;
		}
		if(!convex_intersects(n, *i)){
			continue;
		}
		//cout << "  " << (*i)->get_name() << endl;
		intersectors.push_back(*i);
	}
	if(intersectors.size() == 0){
		//cout << "No Intersectors" << endl;
		// Don't need to adjust at all
		return;
	}
	//cout << "Generating centroid" << endl;

	// Check to see if the centroid is already inside another object
	ptlist centroid_pt;
	centroid_pt.push_back(n->get_centroid());
	convex_node* centroid = new convex_node("temp-centroid", centroid_pt);
	if(convex_intersects(centroid, intersectors)){
		//cout << "Centroid is intersected" << endl;
		// Something is very wrong, the centroid is inside another object, just quit
		delete centroid;
		return;
	}
	delete centroid;

	//cout << "Copying points" << endl;

	// Now do the actual adjustment, on a copy of the original node
	// Copy all the points from this node
	sgnode* copied_node = n->clone();

	vec3 scale = n->get_trans('s');
	vec3 tempScale = scale;
	vec3 newScale = scale;

	//cout << "Old Scale: " << scale[0] << ", " << scale[1] << ", " << scale[2] << endl;

	int freeDim = -1;
	// Test each dimension to see if just 1 needs to be adjusted
	for(int d = 0; d < 3; d++){
		tempScale[d] = scale[d] * .001;
		copied_node->set_trans('s', tempScale);
		if(!convex_intersects(copied_node, intersectors)){
			if(freeDim == -1){
				freeDim = d;
			} else {
				freeDim = -1;
				break;
			}
		}
		tempScale = scale;
	}
	tempScale = scale;
	copied_node->set_trans('s', scale);
	//cout << "Free Dim: " << freeDim << endl;
	if(freeDim != -1){
		newScale = adjust_single_dim(copied_node, intersectors, freeDim);
	} else {
		for(int d = 0; d < 3; d++){
			int d1 = (d+1)%3;
			int d2 = (d+2)%3;
			tempScale[d1] = scale[d1] * .001;
			tempScale[d2] = scale[d2] * .001;
			copied_node->set_trans('s', tempScale);
			if(!convex_intersects(copied_node, intersectors)){
				if(freeDim == -1){
					freeDim = d;
				} else {
					freeDim = -1;
					break;
				}
			}
			tempScale = scale;
		}
		tempScale = scale;
		copied_node->set_trans('s', scale);
		//cout << "Free Dim: " << freeDim << endl;
		if(freeDim != -1){
			newScale = adjust_two_dims(copied_node, intersectors, freeDim);
		} else {
			newScale = adjust_all_dims(copied_node, intersectors);
		}
	}
	//cout << "Old Scale: " << scale[0] << ", " << scale[1] << ", " << scale[2] << endl;
	//cout << "New Scale: " << newScale[0] << ", " << newScale[1] << ", " << newScale[2] << endl;
	n->set_trans('s', newScale);
	delete copied_node;
}

void adjust_sgnode_size(sgnode* n, scene* scn){
	std::vector<const sgnode*> targets;
	std::vector<const sgnode*> all;
	scn->get_all_nodes(all);
	for(std::vector<const sgnode*>::const_iterator i = all.begin(); i != all.end(); i++){
		if(*i == n){
			continue;
		}
		std::string tag_name = "object-source";
		std::string tag_val;
		if((*i)->get_tag(tag_name, tag_val) && tag_val == "belief"){
			targets.push_back(*i);
		}
	}
	adjust_sgnode_size(n, targets);
}
