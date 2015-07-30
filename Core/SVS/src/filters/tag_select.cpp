/***********************************************************************
 *
 * file filters/tag_select.h
 *
 *  tag_select
 *    Parameters:
 *      sgnode a
 *      string tag_name
 *      string tag_value
 *    Returns:
 *      sgnode a if a has the given tag with the value
 *
 ******************************************************************************/

#include "filter.h"
#include "scene.h"
#include "filter_table.h"

#include <string>

using namespace std;

class tag_select_filter : public select_filter<sgnode*>
{
    public:
        tag_select_filter(Symbol* root, soar_interface* si,
                          filter_input* input)
            : select_filter<sgnode * >(root, si, input)
        {}
        
        bool compute(const filter_params* p, sgnode*& out, bool& select)
        {
            sgnode* a;
            if (!get_filter_param(this, p, "a", a))
            {
                set_status("Needs node a as input");
                return false;
            }
            
            string tag_name;
            if (!get_filter_param(this, p, "tag_name", tag_name))
            {
                set_status("Needs tag_name as input");
                return false;
            }
            
            string desired_value;
            if (!get_filter_param(this, p, "tag_value", desired_value))
            {
                set_status("Needs tag_value as input");
                return false;
            }
            
            string tag_value;
            if (a->get_tag(tag_name, tag_value) && desired_value == tag_value)
            {
                select = true;
            }
            else
            {
                select = false;
            }
            
            out = a;
            return true;
        }
};

filter* make_tag_select_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new tag_select_filter(root, si, input);
}

filter_table_entry* tag_select_filter_entry()
{
    filter_table_entry* e = new filter_table_entry();
    e->name = "tag_select";
    e->description = "Outputs all nodes in a which have the given tag";
    e->parameters["a"] = "Sgnode a";
    e->parameters["tag_name"] = "Name of the tag to select";
    e->parameters["tag_value"] = "Value of the tag to select";
    e->create = &make_tag_select_filter;
    return e;
}
