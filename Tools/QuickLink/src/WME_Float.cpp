
#include "WME_Float.h"

#include <cassert>
#include <iostream>
#include <fstream>

using sml::Agent; using sml::Identifier; using sml::FloatElement;
using std::string; using std::cout; using std::endl;
using std::ofstream;

// called by named constructor, actually creates object
WME_Float::WME_Float(const std::string& in_id_name, const string& in_attribute, const double in_value, Agent* pAgent, Identifier* in_id)
: WME_Type(in_id_name, in_attribute)
{
	assert(in_id);
	m_object = pAgent->CreateFloatWME(in_id, in_attribute.c_str(), in_value);
}

// used when sml object has already been created
WME_Float::WME_Float(const string& in_id_name, const string& in_attribute, FloatElement* in_object)
: WME_Type(in_id_name, in_attribute)
{
	assert(in_object);
	m_object = in_object;
}

// named constructor when sml object needs to be created
Smart_Pointer<WME_Float> WME_Float::create(const std::string& in_id_name, const string& in_attribute, const double in_value, Agent* pAgent, Identifier* in_id)
{
	Smart_Pointer<WME_Float> ptr = new WME_Float(in_id_name, in_attribute, in_value, pAgent, in_id);
	return ptr;
}

// named constructor when sml object doesn't need to be created
Smart_Pointer<WME_Float> WME_Float::create_from_sml_object(const string& in_id_name, FloatElement* in_object)
{
	Smart_Pointer<WME_Float> ptr = new WME_Float(in_id_name, in_object->GetAttribute(), in_object);
	return ptr;
}

// print this objects info to cout
bool WME_Float::print_object()
{
	WME_Type::print_object();
	cout << " " << m_object->GetValueAsString();
	return false;
}

// print this objects info to the outfile
void WME_Float::save_yourself(ofstream* outfile)
{
	WME_Type::save_yourself(outfile);
	*outfile << m_object->GetValueAsString() << endl;
}
