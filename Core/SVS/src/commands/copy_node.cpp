#include <iostream>
#include <string>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"
#include "soar_interface.h"
#include "symtab.h"

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
                return parse();
            }
            return true;
        }
        
        
        bool early()
        {
            return false;
        }
        
        bool parse()
        {
            // Get the ^source-id wme
            wme* srcWme;
            if (!si->find_child_wme(root, "source-id", srcWme))
            {
                set_status("^source-id must be specified");
                return false;
            }
            
            // Get the value of the ^source-id wme
            string sourceId;
            if (!get_symbol_value(si->get_wme_val(srcWme), sourceId))
            {
                set_status("^source-id must be a string");
                return false;
            }
            
            // Find the source node with the given name
            const sgnode* sourceNode = scn->get_node(sourceId);
            if (!sourceNode)
            {
                set_status("Could not find the given source node");
                return false;
            }
            
            // Get the ^dest-id wme
            wme* destWme;
            if (!si->find_child_wme(root, "dest-id", destWme))
            {
                set_status("^dest-id must be specified");
                return false;
            }
            
            // Get the value of the ^source-id wme
            string destId;
            if (!get_symbol_value(si->get_wme_val(destWme), destId))
            {
                set_status("^dest-id must be a string");
                return false;
            }
            
            //cout << "COPYING " << sourceId << " to " << destId << endl;
            
            wme* adjustWme;
            string adjust;
            if (!si->find_child_wme(root, "adjust", adjustWme))
            {
                adjust = "false";
            }
            else if (!get_symbol_value(si->get_wme_val(adjustWme), adjust))
            {
                adjust = "false";
            }
            else if (adjust != "true" && adjust != "false")
            {
                adjust = "false";
            }
            
            // Make sure the given destination doesn't already exist
            sgnode* destNode = scn->get_node(destId);
            if (!destNode)
            {
                const ball_node* sourceBall = dynamic_cast<const ball_node*>(sourceNode);
                const convex_node* sourceConvex = dynamic_cast<const convex_node*>(sourceNode);
                if (sourceBall)
                {
                    double radius = sourceBall->get_radius();
                    destNode = new ball_node(destId, "object", radius);
                }
                else if (sourceConvex)
                {
                    ptlist points(sourceConvex->get_verts());
                    destNode = new convex_node(destId, "object", points);
                }
                else
                {
                    set_status("Error: Source node must be either a convex or ball node");
                    return false;
                }
                
                sgnode* rootNode = scn->get_root();
                rootNode->as_group()->attach_child(destNode);
            }
            
            vec3 pos, rot, scale;
            sourceNode->get_trans(pos, rot, scale);
            destNode->set_trans(pos, rot, scale);
            
            //cout << "Adjust = " << adjust << endl;
            if (adjust == "true")
            {
                //cout << "adjust = true" << endl;
                std::vector<const sgnode*> targets;
                std::vector<const sgnode*> all;
                scn->get_all_nodes(all);
                for (std::vector<const sgnode*>::const_iterator i = all.begin(); i != all.end(); i++)
                {
                    if (*i == destNode)
                    {
                        continue;
                    }
                    std::string propName = "object-source";
                    std::string src;
                    if ((*i)->get_property(propName, src))
                    {
                        if (src == "belief")
                        {
                            targets.push_back(*i);
                        }
                    }
                }
                //  cout << "Found Targets: " << endl;
                for (std::vector<const sgnode*>::iterator i = targets.begin(); i != targets.end(); i++)
                {
                    //cout << "  " << (*i)->get_name() << endl;
                }
                destNode->adjust_size(targets);
            }
            
            set_status("success");
            return true;
        }
        
    private:
        Symbol*         root;
        scene*          scn;
        soar_interface* si;
        bool first;
};

command* _make_copy_node_command_(svs_state* state, Symbol* root)
{
    return new copy_node_command(state, root);
}
