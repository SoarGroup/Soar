#include "filter.h"
#include "filter_table.h"

using namespace std;

class vec3_filter : public typed_map_filter<vec3>
{
    public:
        vec3_filter(Symbol* root, soar_interface* si, filter_input* input)
            : typed_map_filter<vec3>(root, si, input)
        {}
        
        bool compute(const filter_params* params, bool adding, vec3& res, bool& changed)
        {
            vec3 newres;
            if (!get_filter_param(this, params, "x", newres[0]) ||
                    !get_filter_param(this, params, "y", newres[1]) ||
                    !get_filter_param(this, params, "z", newres[2]))
            {
                return false;
            }
            changed = (res != newres);
            res = newres;
            return true;
        }
};

filter* _make_vec3_filter_(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new vec3_filter(root, si, input);
}

filter_table_entry* vec3_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "vec3";
    e->create = &_make_vec3_filter_;
    return e;
}
