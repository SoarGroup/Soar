/***************************************************
 *
 * File: filters/intersect.cpp
 *
 * Intersect Filters
 *  bool intersect_test(sgnode* a, sgnode* b, fp* p)
 *    Returns true if a intersects b
 *    intersect_type << bbox hull >>
 *      what kind of intersection to test for, defaults to bbox
 *
 *  Filter intersect : node_test_filter
 *    Parameters:
 *      sgnode a
 *      sgnode b
 *    Returns
 *      bool - true if a intersects b
 *
 *  Filter intersect_select : node_test_select_filter
 *    Parameters:
 *      sgnode a
 *      sgnode b
 *    Returns:
 *      sgnode b - if a intersects b
 *
 *********************************************************/
#include "sgnode_algs.h"
#include "filters/base_node_filters.h"
#include "scene.h"
#include "filter_table.h"

#include <string>

using namespace std;

bool intersect_test(sgnode* a, sgnode* b, const filter_params* p)
{
    if (a == b)
    {
        return true;
    }
    string int_type = "bbox";
    get_filter_param(0, p, "intersect_type", int_type);
    if (int_type == "hull")
    {
        return convex_intersects(a, b);
    }
    else
    {
        return bbox_intersects(a, b);
    }
}

////// filter intersect //////
filter* make_intersect_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_test_filter(root, si, input, &intersect_test);
}

filter_table_entry* intersect_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "intersect";
    e->description = "Returns true if a intersects b";
    e->parameters["a"] = "Sgnode a";
    e->parameters["b"] = "Sgnode b";
    e->create = &make_intersect_filter;
    return e;
}

////// filter intersect_select //////
filter* make_intersect_select_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_test_select_filter(root, si, input, &intersect_test);
}

filter_table_entry* intersect_select_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "intersect_select";
    e->description = "Selects b if a intersects b";
    e->parameters["a"] = "Sgnode a";
    e->parameters["b"] = "Sgnode b";
    e->create = &make_intersect_select_filter;
    return e;
}

