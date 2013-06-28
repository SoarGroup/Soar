#include <iostream>
#include <assert.h>
#include <string>
#include <map>
#include "filter.h"
#include "sgnode.h"
#include "scene.h"
#include "filter_table.h"

using namespace std;

typedef std::vector<const sgnode*> c_sgnode_list;
typedef std::vector<const geometry_node*> c_geom_node_list;
typedef std::pair<convex_node*, bool> view_line;
typedef std::vector<view_line> view_line_list;

view_line create_view_line(const string& name, const vec3& p1, const vec3& p2){
	vec3 dPosOver2 = (p2 - p1)/2;	// Vector from eye to vertex
	vec3 center = p1 + dPosOver2;	// Point halfway between eye and vertex

	ptlist linePoints;
	linePoints.push_back(dPosOver2);
	linePoints.push_back(-dPosOver2);

	convex_node* line = new convex_node(name, "object", linePoints);
	line->set_trans('p', center);

	return view_line(line, false);
}

double calc_occlusion(const scene* scn, const sgnode* a){
	// Get the eye position from the world
	const sgnode* eye = scn->get_node("eye");
	const sgnode* world = scn->get_node("world");
	if(!eye){
		return 0;
	}

	vec3 eyePos = eye->get_centroid();
	std::cout << "EYE: " << eyePos[0] << ", " << eyePos[1] << ", " << eyePos[2] << std::endl;

	// Build up a list of view lines that go from the eye point to each vertex in the target object
	// view_line.first is a convex_node that actually represents the line
	// view_line.second is a bool which is T if that view line is occluded by another object
	view_line_list view_lines;

	c_geom_node_list geom_nodes;
	a->walk_geoms(geom_nodes);

	for(c_geom_node_list::const_iterator i = geom_nodes.begin(); i != geom_nodes.end(); i++){
		const convex_node* c_node = dynamic_cast<const convex_node*>(*i);
		if(c_node){
			std::string name = "_centroid_line_";
			view_lines.push_back(create_view_line(name, eyePos, c_node->get_centroid()));

			const ptlist& verts = c_node->get_world_verts();
			for(ptlist::const_iterator i = verts.begin(); i != verts.end(); i++){
				//std::cout << "Point: " << (*i)[0] << ", " << (*i)[1] << ", " << (*i)[2] << endl;
				std::string name = "_temp_line_" + tostring(view_lines.size());
				view_lines.push_back(create_view_line(name, eyePos, *i));
			}
		}
	}

	// Go through every other object in the scene and see if it occludes any view lines
	c_sgnode_list all_nodes;
	scn->get_all_nodes(all_nodes);

	int num_occluded = 0;
	bool centroid_occluded = false;

	for(c_sgnode_list::const_iterator i = all_nodes.begin(); i != all_nodes.end(); i++){
		const sgnode* n = *i;
		if(n == eye || n == a || n == world){
			// Don't test the target or eye objects
			continue;
		}
		for(view_line_list::iterator j = view_lines.begin(); j != view_lines.end(); j++){
			view_line& view_line = *j;
			if(view_line.second){
				// Already occluded, don't bother checking again
				continue;
			}
			double dist = convex_distance(n, view_line.first);
			if(dist <= 0){
				view_line.second = true;
				num_occluded++;
				if(view_line.first->get_name() == std::string("_centroid_line_")){
					centroid_occluded = true;
				}
			}
		}
	}

	for(view_line_list::iterator i = view_lines.begin(); i != view_lines.end(); i++){
		// No memory leaks here!
		delete i->first;
	}
	if(centroid_occluded){
		return 1;
	} else {
		// Count the number of view lines occluded and return the fraction
		return ((float)num_occluded)/view_lines.size();
	}
}


/*
This filter takes 2 objects and determines if the one occludes the other with respect
to the eye position
*/
class occlusion_filter : public typed_map_filter<double> {
public:
	occlusion_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input)
	: typed_map_filter<double>(root, si, input), scn(scn)
	{}

	bool compute(const filter_params *p, bool adding, double &res, bool &changed) {
		double newres;
		const sgnode *a;

		if (!get_filter_param(this, p, "a", a)) {
			set_status("expecting parameter a");
			return false;
		}
		newres = calc_occlusion(scn, a);
		changed = (res != newres);
		res = newres;
		return true;
	}

private:

	scene *scn;
};

filter *make_occlusion_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new occlusion_filter(root, si, scn, input);
}

filter_table_entry *occlusion_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "occlusion";
	e->parameters.push_back("a");
	e->ordered = false;
	e->allow_repeat = false;
	e->create = &make_occlusion_filter;
	return e;
}

