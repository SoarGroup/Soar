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
        for (size_t i = 0, iend = g2.size(); i < iend; ++i)
        {
            dists.push_back(point_geom_convex_dist(c, g2[i]));
        }
    }
    else if (g2.empty())
    {
        dists.reserve(g1.size());
        c = b->get_centroid();
        for (size_t i = 0, iend = g1.size(); i < iend; ++i)
        {
            dists.push_back(point_geom_convex_dist(c, g1[i]));
        }
    }
    else
    {
        dists.reserve(g1.size() * g2.size());
        for (size_t i = 0, iend = g1.size(); i < iend; ++i)
        {
            for (size_t j = 0, jend = g2.size(); j < jend; ++j)
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

