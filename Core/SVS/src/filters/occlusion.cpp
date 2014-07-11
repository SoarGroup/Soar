/***************************************************************
*  File: occlusion.cpp
*  Author: Aaron Mininger
*  Date: 7/2/13
*  Purpose: The occlusion filter estimates how much a given
*     sgnode is occluded from the eye's point of view and returns
*     a number indicating a rough percentage of occlusion.
*     Occlusion is based on the fraction of vertices of the node
*       That are blocked by another node from the eye perspective
*     This requires an object in the scene with the name 'eye'
*     Note: This filter does not work well with targets that are spheres
*        It will pretty much just determine whether the center is occluded
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

typedef std::vector<const sgnode*> c_sgnode_list;
typedef std::set<const sgnode*> c_sgnode_set;
typedef std::vector<const geometry_node*> c_geom_node_list;
typedef std::pair<convex_node*, bool> view_line;
typedef std::vector<view_line> view_line_list;
typedef std::map<const filter_params*, const sgnode*> element_map;


// Creates a view_line consisting of a convex node with the given name
//   which represents a line segment between the two given points
view_line create_view_line(const string& name, const vec3& p1, const vec3& p2)
{
    vec3 dPosOver2 = (p2 - p1) / 2; // Vector from eye to vertex
    vec3 center = p1 + dPosOver2;   // Point halfway between eye and vertex
    
    ptlist linePoints;
    linePoints.push_back(dPosOver2);
    linePoints.push_back(-dPosOver2);
    
    convex_node* line = new convex_node(name, "object", linePoints);
    line->set_trans('p', center);
    
    return view_line(line, false);
}

// Build up a list of view lines that go from the eye point to each vertex in the target object
// view_line.first is a convex_node that actually represents the line
// view_line.second is a bool which is T if that view line is occluded by another object
void calc_view_lines(const sgnode* target, const sgnode* eye, view_line_list& view_lines)
{
    vec3 eyePos = eye->get_centroid();
//  std::cout << "EYE: " << eyePos[0] << ", " << eyePos[1] << ", " << eyePos[2] << std::endl;

    // Create a view_line for the centroid
    //std::string name = "_centroid_line_";
    //view_lines.push_back(create_view_line(name, eyePos, target->get_centroid()));
    
    c_geom_node_list geom_nodes;
    target->walk_geoms(geom_nodes);
    
    // Go through each vertex in the node and create a view_line to that vertex
    for (c_geom_node_list::const_iterator i = geom_nodes.begin(); i != geom_nodes.end(); i++)
    {
        const convex_node* c_node = dynamic_cast<const convex_node*>(*i);
        if (c_node)
        {
            const ptlist& verts = c_node->get_world_verts();
            for (ptlist::const_iterator i = verts.begin(); i != verts.end(); i++)
            {
                //std::cout << "Point " << view_lines.size() << ": " << (*i)[0] << ", " << (*i)[1] << ", " << (*i)[2] << endl;
                std::string name = "_temp_line_" + tostring(view_lines.size());
                view_lines.push_back(create_view_line(name, eyePos, *i));
            }
        }
    }
}


// Returns the percentage of given view lines are intersected by an object in occludingNodes
double calc_occlusion(view_line_list& view_lines, const c_sgnode_set& occludingNodes)
{
    // Go through every other object in the given set and see if it occludes any view lines
    int num_occluded = 0;
    
    for (view_line_list::iterator i = view_lines.begin(); i != view_lines.end(); i++)
    {
        i->second = false;
    }
    
    for (c_sgnode_set::const_iterator i = occludingNodes.begin(); i != occludingNodes.end(); i++)
    {
        const sgnode* n = *i;
        //std::cout << "Testing Object " << n->get_name() << std::endl;
        for (view_line_list::iterator j = view_lines.begin(); j != view_lines.end(); j++)
        {
            view_line& view_line = *j;
            if (view_line.second)
            {
                // Already occluded, don't bother checking again
                continue;
            }
            double dist = convex_distance(n, view_line.first);
            if (dist <= 0)
            {
                if (n->get_name() == "arm")
                {
                    //std::cout << "ARM OCCLUSION!" << std::endl;
                }
                //std::cout << "Occlusion detected" << std::endl;
                //std::cout << "  " << j->first->get_name() << std::endl;
                //std::cout << "  " << n->get_name() << std::endl;
                
                view_line.second = true;
                num_occluded++;
            }
        }
    }
    
    // Count the number of view lines occluded and return the fraction
    return ((float)num_occluded) / view_lines.size();
}

// Estimates the percentage of the given target that is occluded from the given eye's perspective
// It does this by shooting view_lines from the eye to each verted in the node and returning
//   the fraction that are occluded by other nodes
double calc_occlusion(const sgnode* target, const sgnode* eye, const c_sgnode_set& occludingNodes)
{
    view_line_list view_lines;
    calc_view_lines(eye, target, view_lines);
    double result = calc_occlusion(view_lines, occludingNodes);
    for (view_line_list::iterator i = view_lines.begin(); i != view_lines.end(); i++)
    {
        delete i->first;
    }
    return result;
}


class occlusion_filter : public reduce_filter<double>
{
    public:
        occlusion_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
            : reduce_filter<double>(root, si, input), a(0), eye(0)
        {}
        
        ~occlusion_filter()
        {
            for (view_line_list::iterator i = view_lines.begin(); i != view_lines.end(); i++)
            {
                delete i->first;
            }
        }
        
    private:
        bool input_added(const filter_params* params)
        {
            if (a == 0)
            {
                if (!get_filter_param(this, params, "a", a))
                {
                    set_status("expecting parameter a");
                    return false;
                }
            }
            if (eye == 0)
            {
                if (!get_filter_param(this, params, "eye", eye))
                {
                    set_status("expecting parameter eye");
                    return false;
                }
                calc_view_lines(a, eye, view_lines);
            }
            
            
            const sgnode* b;
            if (!get_filter_param(this, params, "b", b))
            {
                set_status("expecting parameter b");
                return false;
            }
            
            elements[params] = b;
            
            return true;
        }
        
        bool input_changed(const filter_params* params)
        {
            return true;
        }
        
        bool input_removed(const filter_params* params)
        {
            element_map::iterator i = elements.find(params);
            if (i != elements.end())
            {
                elements.erase(i);
            }
            
            return true;
        }
        
        bool calculate_value(double& res)
        {
            c_sgnode_set node_set;
            for (element_map::const_iterator i = elements.begin(); i != elements.end(); i++)
            {
                node_set.insert(i->second);
            }
            res = calc_occlusion(view_lines, node_set);
            return true;
        }
        
        const sgnode* a;
        const sgnode* eye;
        view_line_list view_lines;
        element_map elements;
};

filter* make_occlusion_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new occlusion_filter(root, si, scn, input);
}

filter_table_entry* occlusion_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "occlusion";
    e->parameters.push_back("a");
    e->ordered = false;
    e->allow_repeat = false;
    e->create = &make_occlusion_filter;
    return e;
}
