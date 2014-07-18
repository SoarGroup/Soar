/*********************************************************
 * file: filters/bbox.cpp
 *
 * Filter: bbox_filter
 *  Base: map_filter<bbox>
 *  Type: bbox
 *
 *  filter_params:
 *   node [sgnode]
 *
 *  output:
 *   For every node input this outputs the AA bounding box of that node
 *
 *
 * Filter: bbox_binary_filter
 *  Base: map_filter<bool>
 *  Type: bbox_contains
 *
 *  filter_params:
 *   a [bbox]
 *   b [bbox]
 *
 *  output:
 *   Returns true if bbox a contains bbox b, (b is entirely inside a)
 *   Returns false otherwise
 *
 *
 *  Filter: bbox_binary_filter
 *   Base: map_filter<bool>
 *   Type: bbox_intersects
 *
 *   filter_params:
 *    a [bbox]
 *    b [bbox]
 *
 *   output:
 *    Returns true if bbox a intersects bbox b
 *    Returns false otherwise
 *
 * *******************************************************/

#include "filter.h"
#include "filter_table.h"
#include "common.h"

/* bbox of a single node */
class bbox_filter : public map_filter<bbox>
{
    public:
        bbox_filter(Symbol* root, soar_interface* si, filter_input* input)
            : map_filter<bbox>(root, si, input)
        {}
        
        bool compute(const filter_params* params, bbox& out)
        {
            const sgnode* n;
            
            if (!get_filter_param(this , params, "node", n))
            {
                return false;
            }
            out = n->get_bounds();
            return true;
        }
};

/*
 Handles all binary operations between bounding boxes. Currently this
 includes intersection and containment.
*/
class bbox_binary_filter : public map_filter<bool>
{
    public:
        bbox_binary_filter(Symbol* root, soar_interface* si, filter_input* input, char type)
            : map_filter<bool>(root, si, input), type(type)
        {}
        
        bool compute(const filter_params* params, bool& out)
        {
            bbox a, b;
            
            if (!get_filter_param(this, params, "a", a))
            {
                return false;
            }
            if (!get_filter_param(this, params, "b", b))
            {
                return false;
            }
            
            switch (type)
            {
                case 'i':
                    out = a.intersects(b);
                    break;
                case 'c':
                    out = a.contains(b);
                    break;
                default:
                    assert(false);
            }
            return true;
        }
        
    private:
        char type;
};

filter* make_bbox(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new bbox_filter(root, si, input);
}

filter* make_bbox_int(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new bbox_binary_filter(root, si, input, 'i');
}

filter* make_bbox_contains(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new bbox_binary_filter(root, si, input, 'c');
}

filter_table_entry* bbox_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "bbox";
    e->parameters.push_back("node");
    e->create = &make_bbox;
    return e;
}

filter_table_entry* bbox_int_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "bbox_intersects";
    e->parameters.push_back("a");
    e->parameters.push_back("b");
    e->create = &make_bbox_int;
    return e;
}

filter_table_entry* bbox_contains_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "bbox_contains";
    e->parameters.push_back("a");
    e->parameters.push_back("b");
    e->create = &make_bbox_contains;
    return e;
}
