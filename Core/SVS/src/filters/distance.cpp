/***************************************************
 *
 * File: filters/distance.cpp
 *
 * Distance Filters
 *  double compare_distance(sgnode* a, sgnode* b, fp* p)
 *    Returns the distance between nodes a and b
 *    Optional parameter distance_type << centroid hull >>
 *      Default is centroid
 *
 * Filter distance : node_comparison_filter
 *   Parameters:
 *    sgnode a
 *    sgnode b
 *   Returns:
 *    double - distance between a and b
 *
 * Filter distance_select : node_comparison_select_filter
 *   Parameters:
 *    sgnode a
 *    sgnode b
 *    double min [optional - default = -INF]
 *    double max [optional - default = +INF]
 *   Returns:
 *    sgnode b if min <= dist(a, b) <= max
 *
 *  Filter closest : node_comparison_rank_filter
 *    Parameters:
 *      set<sgnode> a
 *      set<sgnode> b
 *    Returns:
 *      The closest pair of nodes from a and b
 *
 *  Filter farthest : node_comparison_rank_filter
 *    Parameters:
 *      set<sgnode> a
 *      set<sgnode> b
 *    Returns:
 *      The farthest pair of nodes from a and b
 *
 *********************************************************/
#include "sgnode_algs.h"
#include "filters/base_node_filters.h"
#include "scene.h"
#include "filter_table.h"

#include <string>

using namespace std;

double compare_distance(sgnode* a, sgnode* b, const filter_params* p)
{
    if (a == b)
    {
        return 0;
    }
    string dist_type = "centroid";
    get_filter_param(0, p, "distance_type", dist_type);
    if (dist_type == "hull")
    {
        return convex_distance(a, b);
    }
    else
    {
        return centroid_distance(a, b);
    }
}

///// filter distance //////
filter* make_distance_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_comparison_filter(root, si, input, &compare_distance);
}

filter_table_entry* distance_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "distance";
    e->description = "Returns distance between a and b";
    e->parameters["a"] = "Sgnode a";
    e->parameters["b"] = "Sgnode b";
    e->parameters["distance_type"] = "Either centroid or hull";
    e->create = &make_distance_filter;
    return e;
}

///// filter distance_select //////
filter* make_distance_select_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_comparison_select_filter(root, si, input, &compare_distance);
}

filter_table_entry* distance_select_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "distance_select";
    e->description = "Selects b if min <= dist(a, b) <= max";
    e->parameters["a"] = "Sgnode a";
    e->parameters["b"] = "Sgnode b";
    e->parameters["distance_type"] = "Either centroid or hull";
    e->parameters["min"] = "minimum distance to select";
    e->parameters["max"] = "maximum distance to select";
    e->create = &make_distance_select_filter;
    return e;
}

////// filter closest //////
filter* make_closest_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    node_comparison_rank_filter* f = new node_comparison_rank_filter(root, si, input, &compare_distance);
    f->set_select_highest(false);
    return f;
}

filter_table_entry* closest_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "closest";
    e->description = "Output node b closest to node a";
    e->parameters["a"] = "Sgnode a";
    e->parameters["b"] = "Sgnode b";
    e->parameters["distance_type"] = "Either centroid or hull";
    e->create = &make_closest_filter;
    return e;
}

////// filter farthest //////
filter* make_farthest_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_comparison_rank_filter(root, si, input, &compare_distance);
}

filter_table_entry* farthest_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "farthest";
    e->description = "Output node b farthest from node a";
    e->parameters["a"] = "Sgnode a";
    e->parameters["b"] = "Sgnode b";
    e->parameters["distance_type"] = "Either centroid or hull";
    e->create = &make_farthest_filter;
    return e;
}
