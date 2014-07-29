#include <iostream>
#include <list>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"
#include "filter_table.h"
#include "symtab.h"

using namespace std;

class project_command : public command
{
    public:
        project_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), state(state), a(NULL), b(NULL), res_root(NULL), first(true)
        {
            avg[0] = -1;
            avg[1] = -1;
            avg[2] = -1;
            si = state->get_svs()->get_soar_interface();
        }
        
        ~project_command()
        {
            if (!fltrs.empty())
            {
                fltrs.clear();
            }
        }
        
        string description()
        {
            return string("projection");
        }
        
        bool update_sub()
        {
        
            if (first)
            {
                if (!fltrs.empty())
                {
                    fltrs.clear();
                }
                
                std::list<string> tlist;
                if (!get_filter_types(state->get_svs()->get_soar_interface(), root,
                                      tlist))
                {
                    set_status("incorrect filter syntax");
                    return false;
                }
                std::list<string>::iterator i = tlist.begin();
                while (i != tlist.end())
                {
                    filter* f = parse_project_filter_spec(
                                    state->get_svs()->get_soar_interface(), root,
                                    state->get_scene(), *i, "");
                    if (f == NULL)
                    {
                        set_status("incorrect filter syntax");
                        return false;
                    }
                    
                    fltrs.push_back(f);
                    i++;
                }
                
            }
            if (a == NULL || b == NULL)
            {
                std::cout << "Error: CANNOT FIND A OR B" << std::endl;
                set_status("a,b incorrect filter syntax");
            }
            if (avg[0] < 0 || avg[1] < 0 || avg[2] < 0)
            {
                std::cout << "Error: CANNOT FIND avg axes" << std::endl;
                set_status("z,y,z avg incorrect filter syntax");
            }
            if (first)
            {
                first = false;
                update_position_result();
            }
            
            return true;
        }
        
        bool early()
        {
            return false;
        }
        
        void update_position_result()
        {
            std::list<filter*>::iterator i;
            int axis, comp;
            int axes[3][3] = {{ -10, -10, -10},
                { -10, -10, -10},
                { -10, -10, -10}
            };
            
            srand(time(NULL));
            for (i = fltrs.begin(); i != fltrs.end(); ++i)
            {
                filter* f = (filter*) *i;
                axis = f->getAxis();
                comp = f->getComp();
                if (axis >= 0 && comp > -3)
                {
                    if (axes[axis][2] > -10)
                    {
                        std::cout << "Error: TOO MANY RELATIONS" << std::endl;
                    }
                    if (axes[axis][1] > -10)
                    {
                        axes[axis][2] = comp;
                    }
                    else if (axes[axis][0] > -10)
                    {
                        axes[axis][1] = comp;
                    }
                    else
                    {
                        axes[axis][0] = comp;
                    }
                }
            }
            
            vec3 amin, amax, bmin, bmax, ac, bc;
            ptlist pa, pb;
            
            ac = a->get_centroid();
            bc = b->get_centroid();
            bbox ba = a->get_bounds();
            bbox bb = b->get_bounds();
            ba.get_vals(amin, amax);
            bb.get_vals(bmin, bmax);
            int direction[3];
            double pos[3];
            int top[3] = {0};
            double dist;
            for (int i = 0; i < 3; i++)
            {
                //int direction;
                //int top = 0;
                for (int j = 0; j < 3; j++)
                {
                    if (axes[i][j] > -10)
                    {
                        top[i]++;
                    }
                }
                if (top[i] == 0)
                {
                    std::cout << "Error: No filters for AXIS " << i << std::endl;
                }
                direction[i] = axes[i][rand() % top[i]];
                dist = avg[i] * (double) direction[i];
                
                pos[i] = bc[i] + dist +
                         (bmax[i] - bc[i] + amax[i] - ac[i]) * (double) direction[i];
            }
            while (!all_aligned && direction[0] == 0
                    && direction[1] == 0 && direction[2] == 0)
            {
                if (top[0] < 2 && top[1] < 2 && top[2] < 2)
                {
                    //ERROR all-aligned false, but only have aligned filters
                    std::cout << "Error: Alignment misrepresented in projection"
                              << std::endl;
                    break;
                }
                int i = rand() % 3;
                direction[i] = axes[i][rand() % top[i]];
                dist = avg[i] * (double) direction[i];
                
                pos[i] = bc[i] + dist +
                         (bmax[i] - bc[i] + amax[i] - ac[i]) * (double) direction[i];
            }
            
            if (!res_root)
            {
                res_root = si->get_wme_val(si->make_id_wme(root, "result"));
            }
            si->make_wme(res_root, "x", pos[0]);
            si->make_wme(res_root, "y", pos[1]);
            si->make_wme(res_root, "z", pos[2]);
        }
        
        filter* parse_project_filter_spec(soar_interface* si, Symbol* root,
                                          scene* scn, string ftype,
                                          string parent_pname)
        {
            wme_list children, params;
            wme_list::iterator i;
            Symbol* cval;
            string strval, pname, itype;
            long intval;
            double floatval;
            filter_input* input;
            bool fail;
            filter* f;
            
            fail = false;
            si->get_child_wmes(root, children);
            for (i = children.begin(); i != children.end(); ++i)
            {
                if (!get_symbol_value(si->get_wme_attr(*i), pname))
                {
                    continue;
                }
                cval = si->get_wme_val(*i);
                if (pname == "type")
                {
                
                }
                else if (pname == "input-type")
                {
                    if (!get_symbol_value(cval, itype))
                    {
                        return NULL;
                    }
                }
                else if (pname != "status" && pname != "result")
                {
                    params.push_back(*i);
                }
            }
            
            if (itype == "concat")
            {
                input = new concat_filter_input();
            }
            else if (params.size() == 0)
            {
                input = new null_filter_input();
            }
            else
            {
                input = new product_filter_input();
            }
            
            for (i = params.begin(); i != params.end(); ++i)
            {
                if (!get_symbol_value(si->get_wme_attr(*i), pname))
                {
                    continue;
                }
                cval = si->get_wme_val(*i);
                if (get_symbol_value(cval, strval))
                {
                    input->add_param(pname, new const_filter<string>(strval));
                    if ((parent_pname.compare("a") == 0) &&
                            a == NULL && (pname.compare("name") == 0))
                    {
                        a = scn->get_node(strval);
                    }
                    if ((parent_pname.compare("b") == 0) &&
                            b == NULL && (pname.compare("name") == 0))
                    {
                        b = scn->get_node(strval);
                    }
                }
                else if (get_symbol_value(cval, intval))
                {
                    input->add_param(pname, new const_filter<int>(intval));
                }
                else if (get_symbol_value(cval, floatval))
                {
                    input->add_param(pname, new const_filter<float>(floatval));
                }
                else
                {
                    filter* cf;
                    // must be identifier
                    if ((cf = parse_project_filter_spec(si, cval, scn, ftype, pname))
                            == NULL)
                    {
                        fail = true;
                        break;
                    }
                    input->add_param(pname, cf);
                }
                
            }
            
            if (!fail)
            {
                //f = get_filter_table().make_filter(ftype, scn, input);
                f = get_filter_table().make_filter(ftype, root, si, scn, input);
            }
            
            if (fail || ftype == "" || f == NULL)
            {
                delete input;
                return NULL;
            }
            return f;
        }
        
        bool get_filter_types(soar_interface* si, Symbol* root,
                              std::list<string>& tlist)
        {
            wme_list::iterator i;
            bool pass = false;
            wme_list children, children2;
            wme_list::iterator j;
            string pname, pname2, ftype;
            Symbol* cval;
            Symbol* cval2;
            long intval;
            string sval;
            si->get_child_wmes(root, children);
            for (i = children.begin(); i != children.end(); ++i)
            {
                if (!get_symbol_value(si->get_wme_attr(*i), pname))
                {
                    continue;
                }
                int axis;
                double dval, average;
                if ((pname.compare("aligned") == 0))
                {
                    cval = si->get_wme_val(*i);
                    if (!get_symbol_value(cval, sval))
                    {
                        continue;
                    }
                    if (sval.compare("true") == 0)
                    {
                        all_aligned = true;
                    }
                    else
                    {
                        all_aligned = false;
                    }
                }
                else if ((pname.compare("rel") == 0))
                {
                    cval = si->get_wme_val(*i);
                    si->get_child_wmes(cval, children2);
                    for (j = children2.begin(); j != children2.end(); ++j)
                    {
                        if (!get_symbol_value(si->get_wme_attr(*j), pname2))
                        {
                            continue;
                        }
                        cval2 = si->get_wme_val(*j);
                        if (pname2.compare("axis") == 0)
                        {
                            if (!get_symbol_value(cval2, intval))
                            {
                                continue;
                            }
                            axis = (int) intval;
                        }
                        else if (pname2.compare("avg") == 0)
                        {
                            if (!get_symbol_value(cval2, dval))
                            {
                                continue;
                            }
                            average = dval;
                        }
                        else if (pname2.compare("type") == 0)
                        {
                            if (!get_symbol_value(cval2, ftype))
                            {
                                continue;
                            }
                            pass = true;
                            tlist.push_back(ftype);
                        }
                    }
                    avg[axis] = average;
                }
                
            }
            return pass;
        }
        
        void clear_results()
        {
            fltrs.clear();
        }
        
    private:
        Symbol*         root;
        Symbol*         res_root;
        svs_state*      state;
        soar_interface* si;
        std::list<filter*>   fltrs;
        bool   first;
        sgnode* a;
        sgnode* b;
        bool all_aligned;
        double avg[3];
};

command* _make_project_command_(svs_state* state, Symbol* root)
{
    return new project_command(state, root);
}
