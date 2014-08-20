/**********************************************************
 *
 * File: commands/set_transform.cpp
 * Contains:
 *  class set_transform_command
 *  
 *  Soar Command to change the transform of a node (pos/rot/scale)
 *  Parameters:
 *     ^id <string> - name of the node to change
 *     ^position <vec3> [Optional] - desired position
 *     ^rotation <vec3> [Optional] - desired rotation
 *     ^scale <vec3> [Optional] - desired scale
 *      (Can set any or all of these)
 **********************************************************/

#include <iostream>
#include <string>
#include <map>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"

using namespace std;

class set_transform_command : public command
{
    public:
        set_transform_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), first(true)
        {
            si = state->get_svs()->get_soar_interface();
            scn = state->get_scene();
        }
        
        ~set_transform_command()
        {
        }
        
        string description()
        {
            return string("transform");
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

            map<char, vec3>::iterator pi;
            for(pi = props.begin(); pi != props.end(); pi++){
              n->set_trans(pi->first, pi->second);
            }
            
            set_status("success");
            
            return true;
        }
        
        bool early()
        {
            return false;
        }
        
        bool parse()
        {
            wme* idwme, *propwme;
            
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

            vec3 vec;
            if(si->get_vec3(root, "position", vec)){
              props['p'] = vec;
            }
            if(si->get_vec3(root, "rotation", vec)){
              props['r'] = vec;
            }
            if(si->get_vec3(root, "scale", vec)){
              props['s'] = vec;
            }

            return true;
        }
        
    private:
        Symbol*         root;
        scene*          scn;
        soar_interface* si;
        bool            first;
        string          id;
        map<char, vec3> props;

};

command* _make_set_transform_command_(svs_state* state, Symbol* root)
{
    return new set_transform_command(state, root);
}
