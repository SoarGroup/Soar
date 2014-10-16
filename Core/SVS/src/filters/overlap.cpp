/***************************************************
 *
 * File: filters/overlap.cpp
 *
 * Overlap Filters
 *  double compare_overlap(sgnode* a, sgnode* b, fp* p)
 *  	Returns the percentage of a overlapped by b
 *
 * Filter overlap : node_comparison_filter
 *   Parameters:
 *    sgnode a
 *    sgnode b
 *   Returns:
 *    double - percentage of a overlapped by b
 *
 * Filter overlap_select : node_comparison_select_filter
 *   Parameters:
 *    sgnode a
 *    sgnode b
 *    double min [optional - default = -INF]
 *    double max [optional - default = +INF]
 *   Returns:
 *    sgnode b if min <= overlap(a, b) <= max
 *
 *********************************************************/
#include <iostream>
#include <map>
#include "filter.h"
#include "filter_table.h"
#include "common.h"
#include "scene.h"

using namespace std;

double compare_overlap(sgnode* a, sgnode* b, const filter_params* p){
		if (a == b)
		{
				return 1;
		}
		return convex_overlap(a, b, 200);
}

///// filter overlap //////
filter* make_overlap_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_comparison_filter(root, si, input, &compare_overlap);
}

filter_table_entry* overlap_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "overlap";
    e->description = "Returns the percentage of a overlapped by b";
    e->parameters["a"] = "Sgnode a";
    e->parameters["b"] = "Sgnode b";
    e->create = &make_overlap_filter;
    return e;
}

///// filter overlap_select //////
filter* make_overlap_select_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_comparison_select_filter(root, si, input, &compare_overlap);
}

filter_table_entry* overlap_select_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "overlap_select";
    e->description = "Selects b if min <= overlap(a, b) <= max";
    e->parameters["a"] = "Sgnode a";
    e->parameters["b"] = "Sgnode b";
    e->parameters["min"] = "minimum overlap to select";
    e->parameters["max"] = "maximum overlap to select";
    e->create = &make_overlap_select_filter;
    return e;
}


