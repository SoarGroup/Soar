#include "filter.h"
#include "filter_table.h"
#include "scene.h"
#include "common.h"

using namespace std;

bool higher_than(const sgnode* a, const sgnode* b){
		vec3 amin, amax, bmin, bmax;

		bbox ba = a->get_bounds();
		bbox bb = b->get_bounds();
		ba.get_vals(amin, amax);
		bb.get_vals(bmin, bmax);

		if(amax[2] < bmax[2]){
			return false;
		}
		if(amin[0] > bmax[0] + .05){
			return false;
		}
		if(bmin[0] > amax[0] + .05){
			return false;
		}
		if(amin[1] > bmax[1] + .05){
			return false;
		}
		if(bmin[1] > amax[1] + .05){
			return false;
		}
		return true;
}

class higher_than_filter : public typed_select_filter<const sgnode*>{
public:
    higher_than_filter(Symbol *root, soar_interface *si, filter_input *input) 
		: typed_select_filter<const sgnode*>(root, si, input){}
    
    bool compute(const filter_params *params, bool null_out, const sgnode*& out,
			bool& select, bool& changed)
	{
		float newres;
		const sgnode *a, *b;

		
		if (!get_filter_param(this, params, "a", a) ||
			!get_filter_param(this, params, "b", b))
		{
			return false;
		}


		out = b;
		if(higher_than(a, b)){
			select = true;
			changed = true;
		} else {
			select = false;
			changed = false;
		}

		return true;
  }
};

filter *make_higher_than_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
    return new higher_than_filter(root, si, input);
}

filter_table_entry *higher_than_fill_entry() {
    filter_table_entry *e = new filter_table_entry;
    e->name = "higher_than";
	e->parameters.push_back("a");
	e->parameters.push_back("b");
	e->ordered = false;
	e->allow_repeat = false;
    e->create = &make_higher_than_filter;
    return e;
}
