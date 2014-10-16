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

double centroid_distance(const sgnode* a, const sgnode* b)
{
    vec3 ca = a->get_centroid();
    vec3 cb = b->get_centroid();
    return (cb - ca).norm();
}

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


double bbox_volume(const sgnode* a)
{
    bbox boxa = a->get_bounds();
    return boxa.get_volume();
}

bool convex_intersects(const sgnode* a, const sgnode* b)
{
    if (a->get_bounds().intersects(b->get_bounds()))
    {
        return convex_distance(a, b) < INTERSECT_THRESH;
    }
    return false;
}

bool bbox_intersects(const sgnode* a, const sgnode* b)
{
    bbox boxa = a->get_bounds();
    bbox boxb = b->get_bounds();
    return boxa.intersects(boxb);
}

bool bbox_contains(const sgnode* a, const sgnode* b)
{
    bbox boxa = a->get_bounds();
    bbox boxb = b->get_bounds();
    return boxa.contains(boxb);
}

/*  overlap(sgnode* n1, sgnode* n2)
 * This will estimate the percentage of node 1 that is contained within node 2
 * Using random sampling 
 * Result = # Samples that are contained in n1 and n2 / # Samples contained in n1
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
