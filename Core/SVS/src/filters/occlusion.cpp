/***************************************************
 *
 * File: filters/occlusion.cpp
 *
 * Filter occlusion : typed_filter<double>
 *   Parameters:
 *    sgnode a - The object to check the occlusion for
 *    sgnode b - A set of potential occluders to check against
 *    sgnode eye [Optional] The node to use as the view origin
 *      Defaults to looking for a node named 'eye'
 *   Returns:
 *    double - approximate percentage of node a occluded by objects in set b
 *
 *  Purpose: The occlusion filter estimates how much a given 
 *     sgnode is occluded from the eye's point of view and returns
 *     a number indicating a rough percentage of occlusion.
 *     Occlusion is based on the fraction of vertices of the node
 *       That are blocked by another node from the eye perspective
 *
 *  !!!! NOTE !!!!
 *  This filter does not work well for targets that are sphers
 *    (degrades to testing if the center is occluded Y/N)
 *  Also, this has not been tested with ongoing extraction 
 *    (designed for extract_once)
 *    It may work for moving occluders, but will not recalculate the view lines
 *    if object a moves
 ****************************************************************/

#include <iostream>
#include <assert.h>
#include <string>
#include <map>
#include "filter.h"
#include "sgnode.h"
#include "scene.h"
#include "filter_table.h"

using namespace std;

typedef map<const filter_params*, sgnode*> element_map;


class occlusion_filter : public typed_filter<double> {
public:
	occlusion_filter(Symbol *root, soar_interface *si, scene* scn, filter_input *input)
	: typed_filter<double>(root, si, input), a(0), eye(0), scn(scn)
	{}

	~occlusion_filter(){
		for(vector<view_line>::iterator i = view_lines.begin(); i != view_lines.end(); i++){
			delete i->first;
		}
	}

private:
	// Gets the eye and target (a) nodes
	bool initialize(const filter_params* params){
		if(!get_filter_param(this, params, "a", a)){
			set_status("expecting parameter a");
			return false;
		}
		if(!get_filter_param(this, params, "eye", eye)){
			eye = scn->get_node("eye");
			if(eye == NULL){
				set_status("expecting parameter eye");
				return false;
			}
		}
		calc_view_lines(a, eye, view_lines);
		return true;
	}

	virtual bool update_outputs()
	{
		const filter_input* input = filter::get_input();
		const filter_params* params;

		bool changed = false;

		// Added inputs
		for (int i = input->first_added(); i < input->num_current(); ++i)
		{
			params = input->get_current(i);
			if(!initialize(params)){
				return false;
			}
			sgnode* b;
			if(!get_filter_param(this, params, "b", b)){
				set_status("expecting parameter b");
				return false;
			}
			nodes[params] = b;
			changed = true;
		}
		
		// Changed inputs
		for (int i = 0; i < input->num_changed(); ++i)
		{
			params = input->get_changed(i);
			sgnode* b;
			if(!get_filter_param(this, params, "b", b)){
				set_status("Error getting parameter b");
				return false;
			}
			nodes[params] = b;
			changed = true;
		}
		
		// Removed inputs
		for (int i = 0; i < input->num_removed(); ++i)
		{
			params = input->get_removed(i);
			nodes.erase(params);
			changed = true;
		}

		if(changed){
			vector<const sgnode*> occluders;
			for(element_map::const_iterator i = nodes.begin(); i != nodes.end(); i++){
				occluders.push_back(i->second);
			}
			double res = convex_occlusion(view_lines, occluders);
			set_output(NULL, res);
		}

		return true;
	}

	scene* scn;

	sgnode* a;
	sgnode* eye;
	vector<view_line> view_lines; // Set of lines from eye to vertices of a
	element_map nodes;  // Set of nodes to check as occluders
};

filter *make_occlusion_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new occlusion_filter(root, si, scn, input);
}

filter_table_entry *occlusion_filter_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "occlusion";
	e->description = "Returns rough percentage of a occluded by nodes in set b from viewpoint of eye";
	e->parameters["a"] = "Node to check the occlusion for";
	e->parameters["b"] = "Set of nodes to check as occluders";
	e->parameters["eye"] = "Node to act as the viewpoint to check for occlusion from";
	e->create = &make_occlusion_filter;
	return e;
}

