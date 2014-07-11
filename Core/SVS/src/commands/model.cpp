#include <string>
#include "svs.h"
#include "command.h"
#include "model.h"
#include "decide.h"

using namespace std;

void read_list(soar_interface* si, Symbol* id, vector<string>& words)
{
    slot* s;
    wme* w;
    
    if (!si->is_identifier(id))
    {
        return;
    }
    
    for (s = id->id->slots; s != NULL; s = s->next)
    {
        for (w = s->wmes; w != NULL; w = w->next)
        {
            if (si->is_string(w->attr))
            {
                words.push_back(w->attr->sc->name);
                read_list(si, w->value, words);
            }
        }
    }
}

model* parse_model_struct(soar_interface* si, Symbol* cmd, svs* owner)
{
    wme* type_wme, *name_wme;
    string name, type;
    
    if (!si->find_child_wme(cmd, "type", type_wme) ||
            !si->get_val(si->get_wme_val(type_wme), type))
    {
        return NULL;
    }
    
    if (!si->find_child_wme(cmd, "name", name_wme) ||
            !si->get_val(si->get_wme_val(name_wme), name))
    {
        return NULL;
    }
    
    return make_model(owner, name, type);
}

class create_model_command : public command
{
    public:
        create_model_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), svsp(state->get_svs()), broken(false)
        {
            si = svsp->get_soar_interface();
        }
        
        string description()
        {
            return string("create model");
        }
        
        bool update_sub()
        {
            string name;
            
            if (!changed())
            {
                return !broken;
            }
            
            model* m = parse_model_struct(si, root, get_state()->get_svs());
            if (!m)
            {
                set_status("invalid syntax");
                broken = true;
                return false;
            }
            if (!svsp->add_model(m->get_name(), m))
            {
                set_status("nonunique name");
                delete m;
                broken = true;
                return false;
            }
            set_status("success");
            broken = false;
            return true;
        }
        
        bool early()
        {
            return true;
        }
        
        
    private:
        svs*            svsp;
        soar_interface* si;
        Symbol*         root;
        bool            broken;
};


class assign_model_command : public command
{
    public:
        assign_model_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), mmdl(state->get_model()), broken(false), scn(state->get_scene())
        {
            si = state->get_svs()->get_soar_interface();
        }
        
        ~assign_model_command()
        {
            mmdl->unassign_model(name);
        }
        
        string description()
        {
            return string("activate model");
        }
        
        bool early()
        {
            return true;
        }
        
        bool update_sub()
        {
            int i;
            wme* w;
            vector<string> inputs, outputs;
            bool all_inputs = false;
            
            if (!changed() && !broken)
            {
                return true;
            }
            
            if (!si->find_child_wme(root, "name", w) ||
                    !si->get_val(si->get_wme_val(w), name))
            {
                set_status("need model name");
                broken = true;
                return false;
            }
            
            if (si->find_child_wme(root, "inputs", w))
            {
                string v;
                if (si->get_val(si->get_wme_val(w), v) && v == "all")
                {
                    all_inputs = true;
                }
                else
                {
                    read_list(si, si->get_wme_val(w), inputs);
                }
            }
            if (si->find_child_wme(root, "outputs", w))
            {
                read_list(si, si->get_wme_val(w), outputs);
            }
            
            string error = mmdl->assign_model(name, inputs, all_inputs, outputs);
            if (error != "")
            {
                set_status(error);
                broken = true;
                return false;
            }
            
            set_status("success");
            broken = false;
            return true;
        }
        
    private:
        soar_interface* si;
        Symbol*         root;
        multi_model*    mmdl;
        string          name;
        bool            broken;
        scene*          scn;
};

command* _make_create_model_command_(svs_state* state, Symbol* root)
{
    return new create_model_command(state, root);
}

command* _make_assign_model_command_(svs_state* state, Symbol* root)
{
    return new assign_model_command(state, root);
}
