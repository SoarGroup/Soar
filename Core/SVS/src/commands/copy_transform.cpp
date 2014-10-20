/**********************************************************
 *
 * File: commands/copy_transform.cpp
 * Contains:
 *  class copy_transform_command
 *
 *  Soar Command to copy transforms from one node to another
 *  Parameters:
 *     ^source <string> - id of the node to copy the transform from
 *     ^destination <string> - id of the node to copy the transform to
 *
 *     ^position << yes no >> [Optional] - whether to copy the position transform
 *     ^rotation << yes no >> [Optional] - whether to copy the rotation transform
 *     ^scale << yes no >> [Optional] - whether to copy the scale transform
 *     (These all default to no)
 **********************************************************/
#include <iostream>
#include <string>
#include <map>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"
#include "soar_interface.h"
#include "symtab.h"
#include "command_table.h"
#include "sgnode_algs.h"

using namespace std;

class copy_transform_command : public command
{
    public:
        copy_transform_command(svs_state* state, Symbol* root)
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
                return copy_transform();
            }
            return true;
        }
        
        
        bool early()
        {
            return false;
        }
        
        bool parse()
        {
            // source <id>
            // The id of the node to copy the transforms from
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

            // destination <id>
            // The id of the node to copy the transforms to
            string dest_id;
            if (!si->get_const_attr(root, "destination", dest_id))
            {
                set_status("must specify a destination");
                return false;
            }
            
            dest_node = scn->get_node(dest_id);
            if (!dest_node)
            {
                set_status("Could not find the given destination node");
                return false;
            }

						// position << yes no >>
						// Whether to copy the position transform
						string pos_str;
						if(si->get_const_attr(root, "position", pos_str) && 
								(pos_str == "yes" || pos_str == "true")){
							copy_pos = true;
						} 
						else 
						{
							copy_pos = false;
						}

						// rotation << yes no >>
						// Whether to copy the rotation transform
						string rot_str;
						if(si->get_const_attr(root, "rotation", rot_str) && 
								(rot_str == "yes" || rot_str == "true")){
							copy_rot = true;
						} 
						else 
						{
							copy_rot = false;
						}
            
						// scale << yes no >>
						// Whether to copy the scale transform
						string scl_str;
						if(si->get_const_attr(root, "scale", scl_str) && 
								(scl_str == "yes" || scl_str == "true"))
						{
							copy_scl = true;
						}
						else 
						{
							copy_scl = false;
						}

						// adjust << true false >>
						adjust = false;
						string adjust_str;
						if(si->get_const_attr(root, "adjust", adjust_str) &&
								(adjust_str == "yes" || adjust_str == "true")){
							adjust = true;
						}
            
            return true;
        }
        
        bool copy_transform()
        {
						if(copy_pos)
						{
							dest_node->set_trans('p', source_node->get_trans('p'));
						}
						if(copy_rot)
						{
							dest_node->set_trans('r', source_node->get_trans('r'));
						}
						if(copy_scl)
						{
							dest_node->set_trans('s', source_node->get_trans('s'));
						}
						if(adjust)
						{
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
        
        sgnode* source_node;
				sgnode* dest_node;
				bool copy_pos;
				bool copy_rot;
				bool copy_scl;
				bool adjust;
};

command* _make_copy_transform_command_(svs_state* state, Symbol* root)
{
    return new copy_transform_command(state, root);
}

command_table_entry* copy_transform_command_entry()
{
    command_table_entry* e = new command_table_entry();
    e->name = "copy_transform";
    e->description = "Sets transforms on the destination node to those on the source";
    e->parameters["source"] = "Id of the node to copy the transforms from";
    e->parameters["destination"] = "Id of the node to copy the transforms to";
    e->parameters["position"] = "[Optional] - yes/no to copy position transform";
    e->parameters["rotation"] = "[Optional] - yes/no to copy rotation transform";
    e->parameters["scale"] = "[Optional] - yes/no to copy scale transform";
    e->create = &_make_copy_transform_command_;
    return e;
}
