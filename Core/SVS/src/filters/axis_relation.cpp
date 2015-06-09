/***************************************************
 *
 * File: filters/axis_relation.cpp
 *
 * Filter axis_relation_select : select_filter<sgnode*>
 *   Parameters:
 *    sgnode a
 *    sgnode b
 *    axis << x y z >>
 *    bottom DOUBLE
 *      The minimum axis distance for the relation to hold
 *    top DOUBLE
 *      The maximum axis distance for the relation to hold
 *    less << true false >>
 *    	If true, tests if the axis distance is between -bottom and -top
 *    aligned << true false >>
 *    	If true, tests if the two nodes are aligned
 *    greater << true false >>
 *      If true, tests if the axis distance is between bottom and top
 *    base << a b >>
 *      If base == a, calculates the distanec for b relative to a
 *      If base == b, calculates the distance for a relative to b
 *
 *   Returns:
 *    sgnode b if the axis_distance satisfies the specified relation
 *
 *********************************************************/
#include "sgnode_algs.h"
#include "scene.h"
#include "filters/base_node_filters.h"
#include "filter_table.h"

#include <string>

using namespace std;

bool test_axis_relation(const sgnode* a, const sgnode* b, int axis, double top, double bot, 
		bool less, bool aligned, bool greater)
{
	double dist = axis_distance(a, b, axis);
	if(aligned && dist == 0.0){
		return true;
	}
	if(less && -top < dist && dist < -bot){
		return true;
	}
	if(less && aligned && -top < dist && dist <= 0.0){
		return true;
	}
	if(greater && bot < dist && dist < top){
		return true;
	}
	if(greater && aligned && 0.0 <= dist && dist < top){
		return true;
	}
	return false;
}

class axis_relation_select_filter : public select_filter<sgnode*>
{
    public:
        axis_relation_select_filter(Symbol* root, soar_interface* si, filter_input* input)
            : select_filter<sgnode*>(root, si, input)
        {
        }
        
        bool compute(const filter_params* params, sgnode*& out, bool& select)
        {
					sgnode* a;
					if(!get_filter_param(this, params, "a", a)){
						set_status("Need node a as input");
						return false;
					}

					sgnode* b;
					if(!get_filter_param(this, params, "b", b)){
						set_status("Need node b as input");
						return false;
					}

					string axisName;
					if(!get_filter_param(this, params, "axis", axisName)){
						set_status("Need axis x, y, or z specified");
						return false;
					}
					int axis = tolower(axisName[0]) - 'x';

					string greater_str;
					if(!get_filter_param(this, params, "greater", greater_str)){
						greater_str = "false";
					}
					bool greater = (greater_str == "true");

					string aligned_str;
					if(!get_filter_param(this, params, "aligned", aligned_str)){
						aligned_str = "false";
					}
					bool aligned = (aligned_str == "true");

					string less_str;
					if(!get_filter_param(this, params, "less", less_str)){
						less_str = "false";
					}
					bool less = (less_str == "true");

					double bottom;
					if(!get_filter_param(this, params, "bottom", bottom)){
						bottom = 0.0;
					}

					double top;
					if(!get_filter_param(this, params, "top", top)){
						top = 0.0;
					}

					string base;
					if(!get_filter_param(this, params, "base", base)){
						base = "b";
					}

					out = b;
					if(base == "a" || base == "A"){
						select = test_axis_relation(a, b, axis, top, bottom, less, aligned, greater);
					} else {
						select = test_axis_relation(b, a, axis, top, bottom, less, aligned, greater);
					}
					return true;
        }
};


filter* make_axis_relation_select_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new axis_relation_select_filter(root, si, input);
}


filter_table_entry* axis_relation_select_filter_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "axis_relation_select";
    e->description = "Selects node b if the specified relation holds between a and b";
    e->parameters["a"] = "Sgnode a";
		e->parameters["b"] = "Sgnode b";
		e->parameters["axis"] = "<< x y z >> - axis to test on";
		e->parameters["top"] = "Greatest distance allowed in ranges";
		e->parameters["bottom"] = "Smallest distance allowed in ranges";
		e->parameters["less"] = "bool - whether the relation holds when the axis_distnace is negative";
		e->parameters["aligned"] = "bool - whether the relation holds when the nodes are aligned";
		e->parameters["greater"] = "bool - whether the relation holds when the axis_distance is positive";
		e->parameters["base"] = "<< a b >> - whether the relation is b relative to a, or vice-versa";
    e->create = &make_axis_relation_select_filter;
    return e;
}
