/**********************************************************
 *
 * File: commands/set_transform.cpp
 * Contains:
 *  class set_transform_command
 *
 *  Soar Command to change the transform of a node (pos/rot/scale)
 *  Parameters:
 *     ^id <string> - id of the node to change
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
#include "symbol.h"
#include "command_table.h"

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

        std::string description()
        {
            return std::string("transform");
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
                set_status(std::string("Couldn't find node ") + id);
                return false;
            }

//						cout << "setting " << props.size() << " properties" << std::endl;
            std::map<char, vec3>::iterator pi;
            for (pi = props.begin(); pi != props.end(); pi++)
            {
//								cout << "setting " << pi->first << " for " << n->get_id() << " to " << pi->second << std::endl;
                n->set_trans(pi->first, pi->second);
            }

            set_status("success");

            return true;
        }

        int command_type()
        {
            return SVS_WRITE_COMMAND;
        }

        bool parse()
        {
            wme* idwme;

            if (!si->find_child_wme(root, "id", idwme))
            {
                set_status("no object id specified");
                return false;
            }
            if (!get_symbol_value(si->get_wme_val(idwme), id))
            {
                set_status("object id must be a std::string");
                return false;
            }

            vec3 vec;
            if (si->get_vec3(root, "position", vec))
            {
                props['p'] = vec;
            }
            if (si->get_vec3(root, "rotation", vec))
            {
                props['r'] = vec;
            }
            if (si->get_vec3(root, "scale", vec))
            {
                props['s'] = vec;
            }

            return true;
        }

    private:
        Symbol*         root;
        scene*          scn;
        soar_interface* si;
        bool            first;
        std::string          id;
        std::map<char, vec3> props;

};

command* _make_set_transform_command_(svs_state* state, Symbol* root)
{
    return new set_transform_command(state, root);
}

command_table_entry* set_transform_command_entry()
{
    command_table_entry* e = new command_table_entry();
    e->name = "set_transform";
    e->description = "Sets the transforms for a given node";
    e->parameters["id"] = "Id of the node to change";
    e->parameters["position"] = "[Optional] - node position {^x ^y ^z}";
    e->parameters["rotation"] = "[Optional] - node rotation {^x ^y ^z}";
    e->parameters["scale"] = "[Optional] - node scale {^x ^y ^z}";
    e->create = &_make_set_transform_command_;
    return e;
}
