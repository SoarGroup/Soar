/***************************************************
 *
 * File: filters/contain.cpp
 *
 * Intersect Filters
 *  bool contain_test(sgnode* a, sgnode* b, fp* p)
 *    Returns true if a contains b (bbox)
 *
 *  Filter contain : node_test_filter
 *    Parameters:
 *      sgnode a
 *      sgnode b
 *    Returns
 *      bool - true if a contains b
 *
 *  Filter contain_select : node_test_select_filter
 *    Parameters:
 *      sgnode a
 *      sgnode b
 *    Returns:
 *      sgnode b - if a contains b
 *
 *********************************************************/
#include "sgnode_algs.h"
#include "filters/base_node_filters.h"
#include "scene.h"
#include "filter_table.h"

#include <string>

using namespace std;

bool contain_test(sgnode* a, sgnode* b, const filter_params* p){
  return bbox_contains(a, b);
}

////// filter contain //////
filter* make_contain_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input){
  return new node_test_filter(root, si, input, &contain_test);
}

filter_table_entry* contain_filter_entry(){
  filter_table_entry* e = new filter_table_entry();
  e->name = "contain";
  e->description = "Returns true if bbox(a) contains bbox(b)";
  e->parameters["a"] = "Sgnode a";
  e->parameters["b"] = "Sgnode b";
  e->create = &make_contain_filter;
  return e;
}

////// filter contain_select //////
filter* make_contain_select_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input){
  return new node_test_select_filter(root, si, input, &contain_test);
}

filter_table_entry* contain_select_filter_entry(){
  filter_table_entry* e = new filter_table_entry();
  e->name = "contain_select";
  e->description = "Output b if bbox(a) contains bbox(b)";
  e->parameters["a"] = "Sgnode a";
  e->parameters["b"] = "Sgnode b";
  e->create = &make_contain_select_filter;
  return e;
}

