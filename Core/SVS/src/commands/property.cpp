#include <iostream>
#include <string>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"
#include "soar_interface.h"
#include "symtab.h"

using namespace std;

class property_command : public command
{
    public:
        property_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), first(true)
        {
            si = state->get_svs()->get_soar_interface();
            scn = state->get_scene();
        }
        
        ~property_command()
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
            
            //std::cout << "Property " << prop << " of node " << id << " set to " << val << std::endl;
            
            n->set_property(prop, val);
            set_status("success");
            
            return true;
        }
        
        bool early()
        {
            return false;
        }
        
        bool parse()
        {
            wme* idwme, *propwme, *valwme;
            
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
            
            if (!si->find_child_wme(root, "property", propwme))
            {
                set_status("no property specified");
                return false;
            }
            if (!get_symbol_value(si->get_wme_val(propwme), prop))
            {
                set_status("property name must be a string");
                return false;
            }
            
            if (!si->find_child_wme(root, "value", valwme))
            {
                set_status("no value specified");
                return false;
            }
            double dv;
            if (!get_symbol_value(si->get_wme_val(valwme), val))
            {
                if (!get_symbol_value(si->get_wme_val(valwme), dv))
                {
                    set_status("unknown value type");
                    return false;
                }
                val = tostring(dv);
            }
            return true;
        }
        
    private:
        Symbol*         root;
        scene*          scn;
        soar_interface* si;
        bool                        first;
        string          id;
        string          prop;
        string                  val;
};

command* _make_property_command_(svs_state* state, Symbol* root)
{
    return new property_command(state, root);
}
