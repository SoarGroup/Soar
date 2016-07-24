/**********************************************************
 *
 * File: commands/add_node.cpp
 * Contains:
 *  class add_node_command
 *
 *  Soar Command to add a node to the world
 *  Parameters:
 *     ^id <string> - id to give the node, must not already exist
 *     ^parent <string> [Optional] - parent to add the node to
 *     ^geometry << box point sphere group >> - geometry of new node
 *     ^position <vec3> [Optional] - position of the new node
 *     ^rotation <vec3> [Optional] - rotation of the new node
 *     ^scale <vec3> [Optional] - scale of the new node
 *     ^tags <tags> [Optional] - any att/val pairs underneath added as tags
 **********************************************************/
#include <string>
#include <map>
#include "command.h"
#include "scene.h"
#include "command_table.h"
#include "svs.h"
#include "symbol.h"

using namespace std;

enum GeometryType
{
    BOX_GEOM, SPHERE_GEOM, NONE_GEOM, POINT_GEOM
};

class add_node_command : public command
{
    public:
        add_node_command(svs_state* state, Symbol root)
            : command(state, root), root(root), scn(state->get_scene()), parent(NULL), first(true)
        {
            si = state->get_svs()->get_soar_interface();
        }
        
        ~add_node_command()
        {
        }
        
        string description()
        {
            return string("add_node");
        }
        
        bool update_sub()
        {
            if (first)
            {
                first = false;
                if (!parse())
                {
                    return false;
                }
                return add_node();
            }
            return true;
        }
        
        
        bool early()
        {
            return false;
        }
        
        
    private:
        bool parse()
        {
            // ^parent <id>
            // The id of the parent to attach the node to
            // Default is the root
            string parent_id;
            if (!si->get_const_attr(root, "parent", parent_id))
            {
                parent = scn->get_root();
            }
            else
            {
                parent = scn->get_group(parent_id);
                if (parent == NULL)
                {
                    set_status("no parent group node found");
                    return false;
                }
            }
            
            // ^id <id>
            // The id to give the node
            if (!si->get_const_attr(root, "id", node_id))
            {
                set_status("no id specified");
                return false;
            }
            if (scn->get_node(node_id))
            {
                set_status("id already exists");
                return false;
            }
            
            // ^position <vec3>
            // ^rotation <vec3>
            // ^scale <vec3>
            // All optional - specify transforms on the node
            vec3 trans;
            if (si->get_vec3(root, "position", trans))
            {
                transforms['p'] = trans;
            }
            if (si->get_vec3(root, "rotation", trans))
            {
                transforms['r'] = trans;
            }
            if (si->get_vec3(root, "scale", trans))
            {
                transforms['s'] = trans;
            }
            
            // ^geometry << box point sphere group >>
            // Optional - default is group
            // The geometry of the new node
            string geom;
            if (!si->get_const_attr(root, "geometry", geom))
            {
                geom = "group";
            }
            if (geom == "box")
            {
                geom_type = BOX_GEOM;
            }
            else if (geom == "point")
            {
                geom_type = POINT_GEOM;
            }
            else if (geom == "sphere")
            {
                geom_type = SPHERE_GEOM;
            }
            else
            {
                geom_type = NONE_GEOM;
            }

            // Find any att/val pairs underneath tags
            wme* tags_wme;
            if(si->find_child_wme(root, "tags", tags_wme))
            {
                Symbol tags_root = si->get_wme_val(tags_wme);
                vector<wme*> tag_wmes;
                if(si->get_child_wmes(tags_root, tag_wmes))
                {
                    for(vector<wme*>::const_iterator tag_it = tag_wmes.begin(); tag_it != tag_wmes.end(); tag_it++)
                    {
                        Symbol tag_attr = si->get_wme_attr(*tag_it);
                        Symbol tag_value = si->get_wme_val(*tag_it);
                        string tag_attr_str;
                        string tag_value_str;
                        if(get_symbol_value(tag_attr, tag_attr_str) && get_symbol_value(tag_value, tag_value_str))
                        {
                            tags[tag_attr_str] = tag_value_str;
                        }
                    }
                }
            }

            return true;
        }
        
        bool add_node()
        {
            sgnode* n;
            ptlist verts;
            
            switch (geom_type)
            {
                case NONE_GEOM:
                    n = new group_node(node_id);
                    break;
                case SPHERE_GEOM:
                    n = new ball_node(node_id, 1.0);
                    break;
                case POINT_GEOM:
                    verts.push_back(vec3(0, 0, 0));
                    n = new convex_node(node_id, verts);
                    break;
                case BOX_GEOM:
                    verts = bbox_vertices();
                    n = new convex_node(node_id, verts);
                    break;
                default:
                    n   = NULL;
                    break;
            }
            
            for (std::map<char, vec3>::iterator i = transforms.begin(); i != transforms.end(); i++)
            {
                n->set_trans(i->first, i->second);
            }
            
            if (!scn->add_node(parent->get_id(), n))
            {
                set_status("error adding node to scene");
                return false;
            }

            for (tag_map::const_iterator tag_it = tags.begin(); tag_it != tags.end(); tag_it++)
            {
                n->set_tag(tag_it->first, tag_it->second);
            }
            
            set_status("success");
            return true;
        }
        
        ptlist bbox_vertices()
        {
            ptlist pts;
            for (double x = -.5; x <= .5; x += 1.0)
            {
                for (double y = -.5; y <= .5; y += 1.0)
                {
                    for (double z = -.5; z <= .5; z += 1.0)
                    {
                        pts.push_back(vec3(x, y, z));
                    }
                }
            }
            return pts;
        }
        
        scene*             scn;
        Symbol            root;
        soar_interface*    si;
        
        bool first;
        
        GeometryType geom_type;
        map<char, vec3> transforms;
        group_node* parent;
        string node_id;
        tag_map tags;
        
};

command* _make_add_node_command_(svs_state* state, Symbol root)
{
    return new add_node_command(state, root);
}

command_table_entry* add_node_command_entry()
{
    command_table_entry* e = new command_table_entry();
    e->name = "add_node";
    e->description = "Create a new node and adds it to the scene";
    e->parameters["id"] = "Id of the new node";
    e->parameters["parent"] = "Id of the parent node to attach to";
    e->parameters["geometry"] = "Either box, point, sphere, or group";
    e->parameters["position"] = "[Optional] - node position {^x ^y ^z}";
    e->parameters["rotation"] = "[Optional] - node rotation {^x ^y ^z}";
    e->parameters["scale"] = "[Optional] - node scale {^x ^y ^z}";
    e->parameters["tags"] = "[Optional] - any att/val pairs underneath added as tags";
    e->create = &_make_add_node_command_;
    return e;
}
