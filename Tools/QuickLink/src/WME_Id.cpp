
#include "WME_Id.h"
#include "sml_Client.h"
#include "Utilities.h"
#include "QL_Interface.h"

#include <functional>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <fstream>
#include <string>

using std::make_pair; 
using std::string;
using sml::Identifier; using sml::Agent; using sml::WMElement;
using std::cout; using std::for_each;
using std::bind2nd; using std::mem_fun;
using std::ofstream; using std::endl;

WME_Id::WME_Id(const string& in_parent, const string& in_attribute, const string& in_value, Agent* pAgent, Identifier* in_id)
: WME_Type(in_parent, in_attribute), m_value(in_value)
{
	if(in_id)	
		m_object = pAgent->CreateIdWME(in_id, in_attribute.c_str());
	else
	{
		pAgent->SynchronizeInputLink();
		m_object = pAgent->GetInputLink();
		build_children();
	}
}

WME_Id::WME_Id(const string& in_id_name, const string& in_attribute, Identifier* in_object)
: WME_Type(in_id_name, in_attribute)
{
	m_value = in_object->GetValueAsString();
	m_object = in_object;
	build_children();
	QL_Interface::instance().add_created_identifier(this);
}

Smart_Pointer<WME_Id> WME_Id::create(const string& in_parent, const string& in_attribute, const string& in_value, Agent* pAgent, Identifier* in_id)
{
	Smart_Pointer<WME_Id> ptr = new WME_Id(in_parent, in_attribute, in_value, pAgent, in_id);
	return ptr;
}

Smart_Pointer<WME_Id> WME_Id::create_from_sml_object(const string& in_id_name, Identifier* object)
{
	Smart_Pointer<WME_Id> ptr = new WME_Id(in_id_name, object->GetAttribute(), object);
	return ptr;
}

// add child
void WME_Id::add_child(Smart_Pointer<WME_String> in_child)
{
	// key will consist of attribute name followed by value name so that fast identity lookups
	// can be done
	m_string_children.insert(make_pair(make_key(in_child->get_attribute(), in_child->get_value()) , in_child));
	m_all_children.insert(in_child);
}

void WME_Id::add_child(Smart_Pointer<WME_Int> in_child)
{
	// key will consist of attribute name followed by value name so that fast identity lookups
	// can be done
	m_int_children.insert(make_pair(make_key(in_child->get_attribute(), in_child->get_value()) , in_child));
	m_all_children.insert(in_child);
}

void WME_Id::add_child(Smart_Pointer<WME_Float> in_child)
{
	// key will consist of attribute name followed by value name so that fast identity lookups
	// can be done
	m_float_children.insert(make_pair(make_key(in_child->get_attribute(), in_child->get_value()) , in_child));
	m_all_children.insert(in_child);
}

void WME_Id::add_child(Smart_Pointer<WME_Id> in_child)
{
	// key will consist of attribute name followed by value name so that fast identity lookups
	// can be done
	m_id_children.insert(make_pair(make_key(in_child->get_attribute(), in_child->get_value()) , in_child));
	m_all_children.insert(in_child);
}

// check to see if child exists
bool WME_Id::has_child(string attribute, double value)
{
	Smart_Pointer<WME_Float> child = m_float_children[make_key(attribute,string_make(value))];
	if(child)
		return true;

	m_float_children.erase(make_key(attribute,string_make(value)));
	return false;
}

bool WME_Id::has_child(string attribute, int value)
{
	Smart_Pointer<WME_Int> child = m_int_children[make_key(attribute,string_make(value))];
	if(child)
		return true;

	m_int_children.erase(make_key(attribute,string_make(value)));
	return false;
}

bool WME_Id::has_child(string attribute, string value)
{
	// check the strings first
	Smart_Pointer<WME_String> child = m_string_children[make_key(attribute, value)];
	if(child)
		return true;
	m_string_children.erase(make_key(attribute, value));

	// if a string isn't found, check the id's
	Smart_Pointer<WME_Id> id_child = m_id_children[make_key(attribute, value)];
	if(id_child)
		return true;
	// it was not found
	m_id_children.erase(make_key(attribute, value));

	return false;
}

// remove functions
void WME_Id::remove_id_child(const string& att, const string& value, Agent* pAgent)
{
	string key = make_key(att, value);
	Smart_Pointer<WME_Id> child = m_id_children[key];
	assert(child.get_raw_ptr());
	m_id_children.erase(key);
	m_all_children.erase(child);
	child->destroy_sml_object(pAgent);
}

void WME_Id::remove_wme_child(const string& att, double value, Agent* pAgent)
{
	string key = make_key(att, string_make(value));
	Smart_Pointer<WME_Float> child = m_float_children[key];
	assert(child.get_raw_ptr());
	m_float_children.erase(key);
	m_all_children.erase(child);
	child->destroy_sml_object(pAgent);
}

void WME_Id::remove_wme_child(const string& att, int value, Agent* pAgent)
{
	string key = make_key(att, string_make(value));
	Smart_Pointer<WME_Int> child = m_int_children[key];
	assert(child.get_raw_ptr());
	m_int_children.erase(key);
	m_all_children.erase(child);
	child->destroy_sml_object(pAgent);
}

void WME_Id::remove_wme_child(const string& att, const string& value, Agent* pAgent)
{
	string key = make_key(att, value);
	Smart_Pointer<WME_String> child = m_string_children[key];
	assert(child.get_raw_ptr());
	m_string_children.erase(key);
	m_all_children.erase(child);
	child->destroy_sml_object(pAgent);
}

void WME_Id::remove_all_children(Agent* pAgent)
{
	// go through and have all of the children delete their sml objects
	for_each(m_all_children.begin(), m_all_children.end(), bind2nd(mem_fun(&WME_Type::destroy_sml_object), pAgent));
	// clear each of the containers
	m_all_children.clear();
	m_float_children.clear();
	m_string_children.clear();
	m_int_children.clear();
	m_id_children.clear();
}

void WME_Id::update_wme_child(const string& att, double old_value, double new_value, Agent* pAgent)
{
	string key = make_key(att, string_make(old_value));
	Smart_Pointer<WME_Float> child = m_float_children[key];
	assert(child.get_raw_ptr());
	child->update_sml_object(new_value, pAgent);
}

void WME_Id::update_wme_child(const string& att, int old_value, int new_value, Agent* pAgent)
{
	string key = make_key(att, string_make(old_value));
	Smart_Pointer<WME_Int> child = m_int_children[key];
	assert(child.get_raw_ptr());
	child->update_sml_object(new_value, pAgent);
}

void WME_Id::update_wme_child(const string& att, const string& old_value, const string& new_value, Agent* pAgent)
{
	string key = make_key(att, old_value);
	Smart_Pointer<WME_String> child = m_string_children[key];
	assert(child.get_raw_ptr());
	child->update_sml_object(new_value, pAgent);
}

// get a const set of all the children
const all_children_t WME_Id::notify_of_children()
{
	return m_all_children;
}

bool WME_Id::print_object()
{
	WME_Type::print_object();
	cout << " " << m_value;
	return true;
}

void WME_Id::save_yourself(ofstream* outfile)
{
	// print all of the beginning command info
	if(get_id_name() != "" && get_attribute() != "") // make sure this is not the input-link
	{
		WME_Type::save_yourself(outfile);
		*outfile << "/" << m_value << endl;
	}

	// print all of the children info
	for_each(m_all_children.begin(), m_all_children.end(), bind2nd(mem_fun(&WME_Type::save_yourself), outfile));
}

// helper functions
void WME_Id::build_children()
{
	int num_children = m_object->GetNumberChildren();
	for(int i = 0; i < num_children; i++)
	{
		WMElement* child = m_object->GetChild(i);
		if(child->IsIdentifier())
		{
			Smart_Pointer<WME_Id> new_object = 	WME_Id::create_from_sml_object(m_value, child->ConvertToIdentifier());
			add_child(new_object);
		}
		else
		{
			const string type = child->GetValueType();
			if(type == "string")
				add_child(WME_String::create_from_sml_object(m_value, child->ConvertToStringElement()));
			else if(type == "int")
				add_child(WME_Int::create_from_sml_object(m_value, child->ConvertToIntElement()));
			else if(type == "float")
				add_child(WME_Float::create_from_sml_object(m_value, child->ConvertToFloatElement()));
		}
	}
}