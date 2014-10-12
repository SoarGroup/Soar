/**********************************************************
 *
 * File: commands/set_tag.cpp
 * Contains:
 *  class set_tag_command
 *
 *  Soar Command to set a tag on a node (key/val pair)
 *    If the tag already exists, this replaces the existing value
 *  Parameters:
 *     ^id <string> - id of the node to set the tag on
 *     ^tag_name <string> - name of the tag to set
 *     ^tag_value <string> - value of the tag to set
 **********************************************************/
#include <iostream>
#include <string>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"
#include "symtab.h"
#include "command_table.h"

using namespace std;

class set_tag_command : public command
{
    public:
        set_tag_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), first(true)
        {
            si = state->get_svs()->get_soar_interface();
            scn = state->get_scene();
        }
        
        ~set_tag_command() {}
        
        string description()
        {
            return string("set_tag");
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
            }
            else
            {
                return true;
            }
            
            sgnode* n = scn->get_node(id);
            if (!n)
            {
                set_status(string("Couldn't find node ") + id);
                return false;
            }
            
            n->set_tag(tag_name, tag_value);
            set_status("success");
            
            return true;
        }
        
        bool early()
        {
            return false;
        }
        
        bool parse()
        {
            wme* idwme, *tagwme, *valwme;
            
            // id - the id of the node to set the tag of
            if (!si->find_child_wme(root, "id", idwme))
            {
                set_status("no object id specified");
                return false;
            }
            if (!get_symbol_value(si->get_wme_val(idwme), id))
            {
                set_status("object id must be a string");
                return false;
            }
            
            // tag_name - the name of the tag to set
            if (!si->find_child_wme(root, "tag_name", tagwme))
            {
                set_status("no tag_name specified");
                return false;
            }
            if (!get_symbol_value(si->get_wme_val(tagwme), tag_name))
            {
                set_status("tag_name must be a string");
                return false;
            }
            
            // tag_value - the value of the tag to set
            if (!si->find_child_wme(root, "tag_value", valwme))
            {
                set_status("no value specified");
                return false;
            }
            if (!get_symbol_value(si->get_wme_val(valwme), tag_value))
            {
                set_status("tag_value must be a string");
                return false;
            }
            
            return true;
        }
        
    private:
        Symbol*         root;
        scene*          scn;
        soar_interface* si;
        bool            first;
        string          id;
        string          tag_name;
        string          tag_value;
};

command* _make_set_tag_command_(svs_state* state, Symbol* root)
{
    return new set_tag_command(state, root);
}

command_table_entry* set_tag_command_entry()
{
    command_table_entry* e = new command_table_entry();
    e->name = "set_tag";
    e->description = "Sets a tag on a given node (replaces existing)";
    e->parameters["id"] = "Id of the node to tag";
    e->parameters["tag_name"] = "Name of the tag to set";
    e->parameters["tag_value"] = "Value of the tag to set";
    e->create = &_make_set_tag_command_;
    return e;
}
