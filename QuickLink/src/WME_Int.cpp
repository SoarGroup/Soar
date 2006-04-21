
#include "WME_Int.h"

#include <cassert>
#include <iostream>
#include <fstream>

using sml::Agent; using sml::Identifier; using sml::IntElement;
using std::string; using std::cout;
using std::ofstream; using std::endl;

// called by named constructor, actually creates object
WME_Int::WME_Int(const std::string& in_id_name, const string& in_attribute, const int in_value, Agent* pAgent, Identifier* in_id)
: WME_Type(in_id_name, in_attribute)
{
	assert(in_id);
	m_object = pAgent->CreateIntWME(in_id, in_attribute.c_str(), in_value);
}

// used when sml object has already been created
WME_Int::WME_Int(const string& in_id_name, const string& in_attribute, IntElement* in_object)
: WME_Type(in_id_name, in_attribute)
{
	assert(in_object);
	m_object = in_object;
}

// named constructor when sml object needs to be created
Smart_Pointer<WME_Int> WME_Int::create(const std::string& in_id_name, const string& in_attribute, const int in_value, Agent* pAgent, Identifier* in_id)
{
	Smart_Pointer<WME_Int> ptr = new WME_Int(in_id_name, in_attribute, in_value, pAgent, in_id);
	return ptr;
}

// named constructor when sml object doesn't need to be created
Smart_Pointer<WME_Int> WME_Int::create_from_sml_object(const string& in_id_name, IntElement* object)
{
	Smart_Pointer<WME_Int> ptr = new WME_Int(in_id_name, object->GetAttribute(), object);
	return ptr;
}

// print this objects info to cout
bool WME_Int::print_object()
{
	WME_Type::print_object();
	cout << " " << m_object->GetValueAsString();
	return false;
}

// print this objects info to the outfile
void WME_Int::save_yourself(ofstream* outfile)
{
	WME_Type::save_yourself(outfile);
	*outfile << m_object->GetValueAsString() << endl;
}
