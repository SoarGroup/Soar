#include <string>
#include "command.h"
#include "scene.h"
#include "filter.h"
#include "svs.h"

using namespace std;

class del_node_command : public command
{
    public:
        del_node_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), scn(state->get_scene()), first(true)
        {
            si = state->get_svs()->get_soar_interface();
        }
        
        string description()
        {
            return string("del-node");
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
            if (!si->get_val(si->get_wme_val(idWme), nodeId))
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

command* _make_del_node_command_(svs_state* state, Symbol* root)
{
    return new del_node_command(state, root);
}
