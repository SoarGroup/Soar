/***************************************************
 *
 * File: filters/distance_on_axis.cpp
 *
 * Distance Filters
 *  double compare_distance_on_axis(sgnode* a, sgnode* b, fp* p)
 *    Returns the distance between nodes a and b on the given axis
 *
 * Filter distance_on_axis : node_comparison_filter
 *   Parameters:
 *    sgnode a
 *    sgnode b
 *    axis << x y z >>
 *   Returns:
 *    double - distance_on_axis between a and b
 *
 * Filter distance_on_axis_select : node_comparison_select_filter
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

#include <string>

using namespace std;

double compare_distance_on_axis(sgnode* a, sgnode* b, const filter_params* p){
  string axisName;
  if(!get_filter_param(0, p, "axis", axisName)){
    return 0;
  }
  int axis = tolower(axisName[0]) - 'x';
  return distance_on_axis(a, b, axis);
}

///// filter distance_on_axis //////
filter* make_distance_on_axis_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input){
  return new node_comparison_filter(root, si, input, &compare_distance_on_axis);
}

filter_table_entry* distance_on_axis_filter_entry(){
  filter_table_entry* e = new filter_table_entry();
  e->name = "distance_on_axis";
  e->create = &make_distance_on_axis_filter;
  return e;
}

///// filter distance_on_axis_select //////
filter* make_distance_on_axis_select_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input){
  return new node_comparison_select_filter(root, si, input, &compare_distance_on_axis);
}

filter_table_entry* distance_on_axis_select_filter_entry(){
  filter_table_entry* e = new filter_table_entry();
  e->name = "distance_on_axis_select";
  e->create = &make_distance_on_axis_select_filter;
  return e;
}

