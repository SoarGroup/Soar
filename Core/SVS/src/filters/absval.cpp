/*********************************************************
 * file: filters/absval.cpp
 *
 * Filter: absval_filter
 * Base: map_filter<double>
 * Type: absval
 *
 * filter_params:
 *  number [double]
 *
 * output:
 *  For every double input this outputs the absolute value of that number
 *
 * *******************************************************/

#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

class absval_filter : public map_filter<double>
{
    public:
        absval_filter(Symbol* root, soar_interface* si, filter_input* input)
            : map_filter<double>(root, si, input)
        {}
        
        bool compute(const filter_params* params, double& res)
        {
            double inval;
            if (!get_filter_param(this, params, "number", inval)){
              return false;
            }
            res = fabs(inval);
            return true;
        }
};

filter* make_absval_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new absval_filter(root, si, input);
}

filter_table_entry* absval_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "absval";
    e->parameters.push_back("number");
    e->create = &make_absval_filter;
    return e;
}
