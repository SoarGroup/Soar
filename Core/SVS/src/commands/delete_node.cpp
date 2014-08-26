/**********************************************************
 *
 * File: commands/delete_node.cpp
 * Contains:
 *  class delete_node_command
 *  
 *  Soar Command to remove a node from the world
 *  Parameters:
 *     ^id <string> - name of the node to delete
 **********************************************************/
#include <string>
#include "command.h"
#include "scene.h"
#include "filter.h"
#include "svs.h"
#include "soar_interface.h"
#include "symtab.h"

using namespace std;

class delete_node_command : public command
{
    public:
        delete_node_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), scn(state->get_scene()), first(true)
        {
            si = state->get_svs()->get_soar_interface();
        }
        
        string description()
        {
            return string("delete_node");
        }
        
        bool early()
        {
            return false;
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
            
            if (scn->del_node(nodeId))
            {
                set_status("success");
                return true;
            }
            else
            {
                set_status("Could not find the given node");
                return false;
            }
            
            return true;
        }
        
    private:
        bool parse()
        {
            // Get the ^node-id wme
            wme* idWme;
            if (!si->find_child_wme(root, "id", idWme))
            {
                set_status("^id must be specified");
                return false;
            }
            
            // Get the value of the ^source-id wme
            if (!get_symbol_value(si->get_wme_val(idWme), nodeId))
            {
                set_status("^id must be a string");
                return false;
            }
            
            return true;
        }
        
        scene*             scn;
        Symbol*            root;
        soar_interface*    si;
        bool first;
        string                      nodeId;
};

command* _make_delete_node_command_(svs_state* state, Symbol* root)
{
    return new delete_node_command(state, root);
}

command_table_entry* delete_node_command_entry(){
  command_table_entry* e = new command_table_entry();
  e->name = "delete_node";
  e->description = "Deletes the node from the scene";
  e->parameters["id"] = "Id of the node to delete";
  e->create = &_make_delete_node_command_;
  return e;
}
