/******************************************************************************************
*  File: has_property.cpp
*  Author: Aaron Mininger
*  Date: 7/3/13
*  Purpose: has_property is a select_filter which selects a node
*      if it has the given property and value. 
*    The property can be numeric or a string
*    You can use this to select a subset of nodes that have a certain property
*      and then feed those into another filter. This allows you to do 
*      things like select the closest red object (for objects that had a ^color red property) 
********************************************************************************************/
#include <iostream>
#include <assert.h>
#include <string>
#include <map>
#include "filter.h"
#include "sgnode.h"
#include "scene.h"
#include "filter_table.h"

using namespace std;
// has_property
// Returns true if the given node has the given property and value associated with it
// property_value can be either a number or a string
bool has_property(const sgnode* node, const string& property_name, const string& property_value){
	bool hasProperty = false;
	double numericValue;
	if(parse_double(property_value, numericValue)){
		// Check for numeric property
		const numeric_properties_map numeric_props = node->get_numeric_properties();
		numeric_properties_map::const_iterator i = numeric_props.find(property_name);
		if(i != numeric_props.end() && i->second == numericValue){
			hasProperty = true;
		}
	} else {
		// Check for string property
		const string_properties_map string_props = node->get_string_properties();
		string_properties_map::const_iterator i = string_props.find(property_name);
		if(i != string_props.end() && i->second == property_value){
			hasProperty = true;
		}
	}
	return hasProperty;
}

/*
This filter takes a property-name and property-value and checks to see if each
node has that att/val pair
*/
class has_property_filter : public select_filter{
public:
	has_property_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input)
	: select_filter(root, si, input), scn(scn)
	{}

	bool compute(const filter_params *p, filter_val*& out, bool &changed) {
		const sgnode *a;
		string prop_name;
		string prop_value;

		if (!get_filter_param(this, p, "a", a)) {
			set_status("expecting parameter a");
			return false;
		}
		if(!get_filter_param(this, p, "property-name", prop_name)){
			set_status("expecting parameter property-name");
			return false;
		}
		if(!get_filter_param(this, p, "property-value", prop_value)){
			set_status("expecting parameter property-value");
			return false;
		}

		filter_val* a_val = new filter_val_c<const sgnode*>(a);

		changed = true;
		bool select = has_property(a, prop_name, prop_value);
		if(select && out == NULL){
			// Create a new filter val
			out = new filter_val_c<const sgnode*>(a);
		} else if(select && a_val != out){
			// The value has changed
			set_filter_val(out, a);
		} else if(!select && out != NULL){
			// We no longer are selecting the value, make it null
			out = NULL;
		} else {
			// the value didn't actually changed
			changed = false;
		}

		delete a_val;
		
		return true;
	}

private:

	scene *scn;
};

filter *make_has_property_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new has_property_filter(root, si, scn, input);
}

filter_table_entry *has_property_fill_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "has-property";
	e->parameters.push_back("a");
	e->parameters.push_back("property-name");
	e->parameters.push_back("property-value");
	e->ordered = false;
	e->allow_repeat = false;
	e->create = &make_has_property_filter;
	return e;
}
