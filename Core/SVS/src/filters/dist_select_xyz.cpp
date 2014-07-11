#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

class dist_select_xyz_filter : public select_filter
{
    public:
        dist_select_xyz_filter(Symbol* root, soar_interface* si, filter_input* input)
            : select_filter(root, si, input) {}
            
        bool compute(const filter_params* params, filter_val*& out, bool& changed)
        {
            float newres;
            const sgnode* a, *b;
            int axis = -1;
            double top, bot;
            vec3 amin, amax, bmin, bmax, ac, bc;
            double dist;
            
            
            if (!get_filter_param(this, params, "a", a) ||
                    !get_filter_param(this, params, "b", b) ||
                    !get_filter_param(this, params, "top", top) ||
                    !get_filter_param(this, params, "bot", bot) ||
                    !get_filter_param(this, params, "axis", axis))
            {
                return false;
            }
            
            ac = a->get_centroid();
            bc = b->get_centroid();
            bbox ba = a->get_bounds();
            bbox bb = b->get_bounds();
            ba.get_vals(amin, amax);
            bb.get_vals(bmin, bmax);
            
            if (amax[axis] <= bmin[axis])
            {
                dist = abs(amax[axis] - bmin[axis]);
            }
            else if (bmax[axis] <= amin[axis])
            {
                dist = abs(bmax[axis] - amin[axis]);
            }
            else if ((amax[axis] < bmax[axis] && amax[axis] > bmin[axis]) ||
                     (bmax[axis] < amax[axis] && bmax[axis] > amin[axis]) ||
                     (amax[axis] == bmax[axis]) || (bmin[axis] == amin[axis]))
            {
                dist = 0.0;
            }
            else
            {
                std::cout << "Error: Object locations/axes info inconsistent"
                          << std::endl;
                dist = 0.0;
            }
            
            newres = dist;
            
            bool select = ((newres >= bot) && (newres <= top));
            if ((bot == 0) && (top == 100))
            {
                select = true;
            }
            changed = true;
            filter_val* a_val = new filter_val_c<const sgnode*>(b);
            
            if (select && out == NULL)
            {
                // Create a new filter val
                out = new filter_val_c<const sgnode*>(b);
            }
            else if (select && a_val != out)
            {
                // The value has changed
                set_filter_val(out, b);
            }
            else if (!select && out != NULL)
            {
                // We no longer are selecting the value, make it null
                out = NULL;
            }
            else
            {
                // the value didn't actually changed
                changed = false;
            }
            
            delete a_val;
            
            return true;
        }
};

filter* make_dist_select_xyz_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new dist_select_xyz_filter(root, si, input);
}

filter_table_entry* dist_select_xyz_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "dist_select_xyz";
    e->parameters.push_back("a");
    e->parameters.push_back("b");
    e->parameters.push_back("top");
    e->parameters.push_back("bot");
    e->parameters.push_back("axis");
    e->ordered = false;
    e->allow_repeat = false;
    e->create = &make_dist_select_xyz_filter;
    return e;
}
