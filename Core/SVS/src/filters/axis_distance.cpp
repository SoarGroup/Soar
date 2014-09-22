/***************************************************
 *
 * File: filters/axis_distance.cpp
 *
 * Distance Filters
 *  double compare_axis_distance(sgnode* a, sgnode* b, fp* p)
 *    Returns the distance between nodes a and b on the given axis
 *
 * Filter axis_distance : node_comparison_filter
 *   Parameters:
 *    sgnode a
 *    sgnode b
 *    axis << x y z >>
 *   Returns:
 *    double - axis_distance between a and b
 *
 * Filter axis_distance_select : node_comparison_select_filter
 *   Parameters:
 *    sgnode a
 *    sgnode b
 *    axis << x y z >>
 *    double min [optional - default = -INF]
 *    double max [optional - default = +INF]
 *   Returns:
 *    sgnode b if min <= dist(a, b) <= max
 *
 *********************************************************/
#include "sgnode_algs.h"
#include "filters/base_node_filters.h"
#include "scene.h"
#include "filter_table.h"

#include <string>

using namespace std;

double compare_axis_distance(sgnode* a, sgnode* b, const filter_params* p)
{
    string axisName;
    if (!get_filter_param(0, p, "axis", axisName))
    {
        return 0;
    }
    int axis = tolower(axisName[0]) - 'x';
    return axis_distance(a, b, axis);
}

///// filter axis_distance //////
filter* make_axis_distance_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_comparison_filter(root, si, input, &compare_axis_distance);
}

filter_table_entry* axis_distance_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "axis_distance";
    e->description = "Output distance between a and b on given axis";
    e->parameters["a"] = "Sgnode a";
    e->parameters["b"] = "Sgnode b";
    e->parameters["axis"] = "Axis to measure distance on (xyz)";
    e->create = &make_axis_distance_filter;
    return e;
}

///// filter axis_distance_select //////
filter* make_axis_distance_select_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_comparison_select_filter(root, si, input, &compare_axis_distance);
}

filter_table_entry* axis_distance_select_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "axis_distance_select";
    e->description = "Select b if min <= dist(a, b, axis) <= max";
    e->parameters["a"] = "Sgnode a";
    e->parameters["b"] = "Sgnode b";
    e->parameters["axis"] = "Axis to measure distance on (xyz)";
    e->parameters["min"] = "minimum distance to select";
    e->parameters["max"] = "maximum distance to select";
    e->create = &make_axis_distance_select_filter;
    return e;
}

