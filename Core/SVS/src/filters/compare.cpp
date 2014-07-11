#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

class compare_nodes_filter : public typed_map_filter<double>
{
    public:
        compare_nodes_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
            : typed_map_filter<double>(root, si, input)
        {}
        
        bool compute(const filter_params* params, bool adding, double& res, bool& changed)
        {
            const sgnode* a;
            const sgnode* b;
            string comp;
            double newres;
            
            if (!get_filter_param(this, params, "a", a))
            {
                set_status("Expecting parameter a of type node");
                return false;
            }
            if (!get_filter_param(this, params, "b", b))
            {
                set_status("Expecting parameter b of type node");
                return false;
            }
            if (!get_filter_param(this, params, "compare", comp))
            {
                set_status("Expecting parameter compare of type string");
                return false;
            }
            if (comp == "volume")
            {
                vec3 aScale = a->get_trans('s');
                vec3 bScale = b->get_trans('s');
                double aVol = aScale[0] * aScale[1] * aScale[2];
                double bVol = bScale[0] * bScale[1] * bScale[2];
                if (bVol < .000000001)
                {
                    newres = 1;
                }
                else
                {
                    newres = aVol / bVol;
                }
            }
            else if (comp == "position")
            {
                vec3 aPos = a->get_trans('p');
                vec3 bPos = b->get_trans('p');
                vec3 diff = a->get_trans('p') - b->get_trans('p');
                newres = sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
            }
            else
            {
                set_status("Unknown comparison type");
                return false;
            }
            changed = (newres != res);
            res = newres;
            return true;
        }
        
};

filter* make_compare_nodes_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new compare_nodes_filter(root, si, scn, input);
}

filter_table_entry* compare_nodes_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "compare-nodes";
    e->parameters.push_back("a");
    e->parameters.push_back("b");
    e->parameters.push_back("compare");
    e->create = &make_compare_nodes_filter;
    return e;
}


class compare_filter : public typed_map_filter<bool>
{
    public:
        compare_filter(Symbol* root, soar_interface* si, filter_input* input)
            : typed_map_filter<bool>(root, si, input)
        {}
        
        bool compute(const filter_params* params, bool adding, bool& res, bool& changed)
        {
            double a, b;
            string comp;
            bool newres;
            
            if (!get_filter_param(this, params, "a", a) ||
                    !get_filter_param(this, params, "b", b) ||
                    !get_filter_param(this, params, "compare", comp))
            {
                return false;
            }
            if (comp == "gt")
            {
                newres = (a > b);
            }
            else if (comp == "gte")
            {
                newres = (a >= b);
            }
            else if (comp == "lt")
            {
                newres = (a < b);
            }
            else if (comp == "lte")
            {
                newres = (a <= b);
            }
            else if (comp == "eq")
            {
                newres = (a == b);
            }
            else
            {
                return false;
            }
            
            changed = (newres != res);
            res = newres;
            return true;
        }
        
};

filter* make_compare_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new compare_filter(root, si, input);
}

filter_table_entry* compare_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "compare";
    e->parameters.push_back("compare");
    e->create = &make_compare_filter;
    return e;
}
