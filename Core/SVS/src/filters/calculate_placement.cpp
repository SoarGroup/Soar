/***************************************************
 *
 * File: filters/calculate_placement.cpp
 *
 * Calculate Placement Filter
 * 	given two nodes, find a position for the first relative to the second
 * 	that satisfies axis separation constraints, 
 * 	taking bounding boxes into consideration
 * 	E.g. calculate a position so that an object is on top of another, 
 *
 * Filter calculate_placement : map_filter<vec3>
 *   Parameters:
 *    sgnode a - the node being placed
 *    sgnode b - the reference node
 *    x_axis_separation FLOAT [Optional] - Default is 0 (aligned)
 *    y_axis_separation FLOAT [Optional] - Default is 0 (aligned)
 *    z_axis_separation FLOAT [Optional] - Default is 0 (aligned)
 *   Returns:
 *    vec3 - a position at which to place node a such that the given 
 *    	axis separation constraints are satisfied
 *   Notes on separations:
 *   	The separation is a constraint on what the distance between 
 *   	the two nodes should be along the given axis (using bounding boxes)
 *   		A positive distance indicates node a is higher up on the axis, 
 *   		A negative distance indicates node a is lower down on the axis, 
 *   		A distance of 0 indicates they are aligned on that axis (share the same value)
 *
 *********************************************************/
#include "sgnode_algs.h"
#include "filters/base_node_filters.h"
#include "scene.h"
#include "filter_table.h"

#include <string>

using namespace std;

vec3 calculate_placement(const sgnode* a, const sgnode* b, double xsep, double ysep, double zsep){
	vec3 sep(xsep, ysep, zsep);
	vec3 pa = a->get_centroid();
	vec3 pb = b->get_centroid();
	vec3 mina = a->get_bounds().get_min();
	vec3 minb = b->get_bounds().get_min();
	vec3 maxa = a->get_bounds().get_max();
	vec3 maxb = b->get_bounds().get_max();
	vec3 pos;
	for(int dim = 0; dim < 3; dim++){
		// For each dimension/axis
		if(sep[dim] > 0){
			// Pos of a = Max bound of b + separation distance + distance from min bound to centroid of a
			pos[dim] = maxb[dim] + sep[dim] + (pa[dim] - mina[dim]);
		} else if(sep[dim] == 0){
			// Pos of a = pos of b (aligned)
			pos[dim] = pb[dim];
		} else {
			// Pos of a = min bound of b - separation distance - distance from centroid to max bound of a
			// (note separation distance is negative, so we add it)
			pos[dim] = minb[dim] + sep[dim] - (maxa[dim] - pa[dim]);
		}
	}
	return pos;
}

class calculate_placement_filter : public map_filter<vec3>
{
    public:
      calculate_placement_filter(Symbol* root, soar_interface* si, filter_input* input)
            : map_filter<vec3>(root, si, input)
        {}
        
        bool compute(const filter_params* params, vec3& out)
        {
            sgnode* a;
            if (!get_filter_param(this, params, "a", a))
            {
							set_status("expecting sgnode parameter 'a'");
                return false;
            }

            sgnode* b;
            if (!get_filter_param(this, params, "b", b))
            {
							set_status("expecting sgnode parameter 'b'");
                return false;
            }

						double xsep;
						if(!get_filter_param(this, params, "x_axis_separation", xsep)){
							xsep = 0;
						}
						double ysep;
						if(!get_filter_param(this, params, "y_axis_separation", ysep)){
							ysep = 0;
						}
						double zsep;
						if(!get_filter_param(this, params, "z_axis_separation", zsep)){
							zsep = 0;
						}

						out = calculate_placement(a, b, xsep, ysep, zsep);
            return true;
        }
};

///// filter calculate_placement //////
filter* make_calculate_placement_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new calculate_placement_filter(root, si, input);
}

filter_table_entry* calculate_placement_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "calculate_placement";
    e->description = "Outputs a position for node a that satisfies the axis separation constraints relative to node b";
    e->parameters["a"] = "The node being placed";
    e->parameters["b"] = "The node acting as a reference to place node a";
    e->parameters["x_axis_separation"] = "Desired separation between a and b along the x axis (default is 0)";
    e->parameters["y_axis_separation"] = "Desired separation between a and b along the y axis (default is 0)";
    e->parameters["z_axis_separation"] = "Desired separation between a and b along the z axis (default is 0)";
    e->create = &make_calculate_placement_filter;
    return e;
}

