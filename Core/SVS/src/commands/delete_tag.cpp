/**********************************************************
 *
 * File: commands/delete_tag
 * Contains:
 *  class delete_tag
 *  
 *  Soar Command to delete a tag on a node
 *  Parameters:
 *     ^id <string> - name of the node to delete the tag on
 *     ^tag_name <string> - name of the tag to delete
 *
 **********************************************************/
#include <iostream>
#include <string>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"

using namespace std;

class delete_tag_command : public command
{
    public:
        delete_tag_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), first(true)
        {
            si = state->get_svs()->get_soar_interface();
            scn = state->get_scene();
        }
        
        ~delete_tag_command() {}
        
        string description() {
            return string("delete_tag");
        }
        
        bool update_sub(){
            if (first) {
                first = false;
                if (!parse()) {
                    return false;
                }
            } else {
                return true;
            }
            
            sgnode* n = scn->get_node(id);
            if (!n) {
                set_status(string("Couldn't find node ") + id);
                return false;
            }

            n->delete_tag(tag_name);
            set_status("success");
            
            return true;
        }
        
        bool early() {
            return false;
        }
        
        bool parse() {
            wme* idwme, *tagwme, *valwme;
            
            // id - the id of the node to delete the tag from
            if (!si->find_child_wme(root, "id", idwme))
            {
                set_status("no object id specified");
                return false;
            }
            if (!get_symbol_value(si->get_wme_val(idwme), id)){
                set_status("object id must be a string");
                return false;
            }
            
            // tag_name - the name of the tag to delete
            if (!si->find_child_wme(root, "tag_name", tagwme)){
                set_status("no tag_name specified");
                return false;
            }
            if (!get_symbol_value(si->get_wme_val(tagwme), tag_name)){
                set_status("tag_name must be a string");
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
};

command* _make_delete_tag_command_(svs_state* state, Symbol* root)
{
  return new delete_tag_command(state, root);
}
