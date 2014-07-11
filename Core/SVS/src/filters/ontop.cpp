#include "filter.h"
#include "filter_table.h"
#include "common.h"
#include "scene.h"
#include "mat.h"

using namespace std;

bool ontop(const sgnode* tn, const sgnode* bn)
{
    vec3 tmin, tmax, bmin, bmax;
    
    const bbox& tb = tn->get_bounds();
    const bbox& bb = bn->get_bounds();
    double h1 = tmax[2] - tmin[2], h2 = bmax[2] - bmin[2];
    double margin = min(h1, h2) * .05;
    
    tb.get_vals(tmin, tmax);
    bb.get_vals(bmin, bmax);
    return tb.intersects(bb) && tmin[2] >= bmax[2] - margin;
}

bool standalone(const scene* scn, const vector<const sgnode*>& args)
{
    assert(args.size() == 2);
    return ontop(args[0], args[1]);
}

class ontop_filter : public typed_map_filter<bool>
{
    public:
        ontop_filter(Symbol* root, soar_interface* si, filter_input* input)
            : typed_map_filter<bool>(root, si, input)
        {}
        
        bool compute(const filter_params* params, bool adding, bool& res, bool& changed)
        {
            const sgnode* tn, *bn;
            if (!get_filter_param(this, params, "top", tn) ||
                    !get_filter_param(this, params, "bottom", bn))
            {
                return false;
            }
            bool newres = ontop(tn, bn);
            changed = (newres != res);
            res = newres;
            return true;
        }
};

filter* make_ontop_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new ontop_filter(root, si, input);
}

filter_table_entry* ontop_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "on-top";
    e->parameters.push_back("top");
    e->parameters.push_back("bottom");
    e->ordered = true;
    e->allow_repeat = false;
    e->create = &make_ontop_filter;
    e->calc = &standalone;
    return e;
}
