/**********************************************************
 *
 * File: commands/copy_node.cpp
 * Contains:
 *  class add_copy_command
 *
 *  Soar Command to create a new node by copying an old one
 *  Parameters:
 *     ^id <string> - id to give the node, must not already exist
 *     ^source <string> - id of the node to copy from
 *     ^parent <string> [Optional] - parent to add the node to
 *
 *     ^position <vec3> [Optional] - position of the new node
 *     ^rotation <vec3> [Optional] - rotation of the new node
 *     ^scale <vec3> [Optional] - scale of the new node
 *       (These transforms default to that of the source node)
 *     ^copy_tags <bool> [Optional] - whether to copy tags from source
 *     ^tags <tags> [Optional] - any string att/val pairs underneath added as tags
 **********************************************************/
#include <iostream>
#include <string>
#include <map>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"
#include "sgnode_algs.h"
#include "soar_interface.h"
#include "symtab.h"
#include "command_table.h"

using namespace std;

class copy_node_command : public command
{
    public:
        copy_node_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), first(true)
        {
            si = state->get_svs()->get_soar_interface();
            scn = state->get_scene();
        }
        
        string description()
        {
            return string("copy-node");
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
                return copy_node();
            }
            return true;
        }
        
        
        bool early()
        {
            return false;
        }
        
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
                    set_status("no group node parent");
                    return false;
                }
            }
            
            // source <id>
            // The id of the node to copy
            string source_id;
            if (!si->get_const_attr(root, "source", source_id))
            {
                set_status("must specify a source");
                return false;
            }
            
            source_node = scn->get_node(source_id);
            if (!source_node)
            {
                set_status("Could not find the given source node");
                return false;
            }
            
            // id <id>
            // the id of the node to create
            if (!si->get_const_attr(root, "id", node_id))
            {
                set_status("^id must be specified");
                return false;
            }
            if (scn->get_node(node_id))
            {
                set_status("Node already exists");
                return false;
            }
            
            // ^position <vec3>
            // ^rotation <vec3>
            // ^scale <vec3>
            // All optional - specify transforms on the node
            //   Default to those of the source node
            transforms['p'] = source_node->get_trans('p');
            transforms['r'] = source_node->get_trans('r');
            transforms['s'] = source_node->get_trans('s');
            
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
            
            // ^copy_tags << true false >>
            copy_tags = false;
            string copy_str;
            if (si->get_const_attr(root, "copy_tags", copy_str) &&
                    copy_str == "true")
            {
                copy_tags = true;
            }

            // Find any att/val pairs underneath tags
            wme* tags_wme;
            if(si->find_child_wme(root, "tags", tags_wme))
            {
                Symbol* tags_root = si->get_wme_val(tags_wme);
                wme_list tag_wmes;
                if(si->get_child_wmes(tags_root, tag_wmes))
                {
                    for(wme_list::const_iterator tag_it = tag_wmes.begin(); tag_it != tag_wmes.end(); tag_it++)
                    {
                        Symbol* tag_attr = si->get_wme_attr(*tag_it);
                        Symbol* tag_value = si->get_wme_val(*tag_it);
                        string tag_attr_str;
                        string tag_value_str;
                        if(get_symbol_value(tag_attr, tag_attr_str) && get_symbol_value(tag_value, tag_value_str))
                        {
                            tags[tag_attr_str] = tag_value_str;
                        }
                    }
                }
            }

						// adjust << true false >>
						adjust = false;
						string adjust_str;
						if(si->get_const_attr(root, "adjust", adjust_str) &&
								adjust_str == "true"){
							adjust = true;
						}
            
            return true;
        }
        
        bool copy_node()
        {
            sgnode* dest_node;
            
            const ball_node* sourceBall = dynamic_cast<const ball_node*>(source_node);
            const convex_node* sourceConvex = dynamic_cast<const convex_node*>(source_node);
            if (sourceBall)
            {
                double radius = sourceBall->get_radius();
                dest_node = new ball_node(node_id, radius);
            }
            else if (sourceConvex)
            {
                ptlist points(sourceConvex->get_verts());
                dest_node = new convex_node(node_id, points);
            }
            else
            {
                dest_node = new group_node(node_id);
            }
            
            parent->attach_child(dest_node);
            
            dest_node->set_trans('p', transforms['p']);
            dest_node->set_trans('r', transforms['r']);
            dest_node->set_trans('s', transforms['s']);
            
            if (copy_tags)
            {
                const tag_map& src_tags = source_node->get_all_tags();
                for (tag_map::const_iterator tag_it = src_tags.begin(); tag_it != src_tags.end(); tag_it++)
                {
                    dest_node->set_tag(tag_it->first, tag_it->second);
                }
            }

            for (tag_map::const_iterator tag_it = tags.begin(); tag_it != tags.end(); tag_it++)
            {
                dest_node->set_tag(tag_it->first, tag_it->second);
            }

						if(adjust){
							adjust_sgnode_size(dest_node, scn);
						}
            
            set_status("success");
            return true;
        }
        
    private:
        Symbol*         root;
        scene*          scn;
        soar_interface* si;
        bool first;
        
        const sgnode* source_node;
        group_node* parent;
        string node_id;
        map<char, vec3> transforms;
        tag_map tags;
        bool copy_tags;
				bool adjust;
};

command* _make_copy_node_command_(svs_state* state, Symbol* root)
{
    return new copy_node_command(state, root);
}

command_table_entry* copy_node_command_entry()
{
    command_table_entry* e = new command_table_entry();
    e->name = "copy_node";
    e->description = "Creates a copy of the given source node";
    e->parameters["id"] = "Id of the new node";
    e->parameters["source"] = "Id of the node to copy from";
    e->parameters["parent"] = "[Optional] - Id of the parent node to attach to";
    e->parameters["position"] = "[Optional] - node position {^x ^y ^z}";
    e->parameters["rotation"] = "[Optional] - node rotation {^x ^y ^z}";
    e->parameters["scale"] = "[Optional] - node scale {^x ^y ^z}";
    e->parameters["copy_tags"] = "[Optional] - true/false to copy tags from source node";
    e->parameters["tags"] = "[Optional] - any att/val pairs underneath added as tags";
    e->create = &_make_copy_node_command_;
    return e;
}
