#include <iostream>
#include <string>
#include <map>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"

using namespace std;

class set_property_command : public command
{
    public:
        set_property_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), first(true)
        {
            si = state->get_svs()->get_soar_interface();
            scn = state->get_scene();
        }
        
        ~set_property_command()
        {
        }
        
        string description()
        {
            return string("property");
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

            map<string, double>::iterator pi;
            for(pi = props.begin(); pi != props.end(); pi++){
              char p = tolower(pi->first[0]);
              int dim = tolower(pi->first[1]) - 'x';
              n->set_native_property(p, dim, pi->second);
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

            const char* native_props[] = { "px", "py", "pz", "rx", "ry", "rz", "sx", "sy", "sz" };
            for(int i = 0; i < 9; i++){
              double value;
              if(si->find_child_wme(root, native_props[i], propwme) &&
                  get_symbol_value(si->get_wme_val(propwme), value)){
                props[native_props[i]] = value;
              }
            }

            return true;
        }
        
    private:
        Symbol*         root;
        scene*          scn;
        soar_interface* si;
        bool            first;
        string          id;
        map<string, double> props;

};

command* _make_set_property_command_(svs_state* state, Symbol* root)
{
    return new set_property_command(state, root);
}
