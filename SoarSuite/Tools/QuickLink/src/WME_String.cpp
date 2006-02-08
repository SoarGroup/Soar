
#include "WME_String.h"
#include "sml_Client.h"

#include <cassert>
#include <iostream>
#include <fstream>

using sml::Agent; using sml::Identifier; using sml::StringElement;
using std::string; using std::cout; using std::endl;
using std::ofstream;

WME_String::WME_String(const std::string& in_id_name, const string& in_attribute, const string& in_value, Agent* pAgent, Identifier* in_id)
: WME_Type(in_id_name, in_attribute)
{
	assert(in_id);
	m_object = pAgent->CreateStringWME(in_id, in_attribute.c_str(), in_value.c_str());
}

// create a WME_String from and existing sml_object
WME_String::WME_String(const string& parent_name, const string& attribute, StringElement* object)
: WME_Type(parent_name, attribute)
{
	assert(object);
	m_object = object;
}

Smart_Pointer<WME_String> WME_String::create(const std::string& in_id_name, const string& in_attribute, const string& in_value, Agent* pAgent, Identifier* in_id)
{
	Smart_Pointer<WME_String> ptr = new WME_String(in_id_name, in_attribute, in_value, pAgent, in_id);
	return ptr;
}

Smart_Pointer<WME_String> WME_String::create_from_sml_object(const string& in_id_name, StringElement* object)
{
	Smart_Pointer<WME_String> ptr = new WME_String(in_id_name, object->GetAttribute(), object);
	return ptr;
}

bool WME_String::print_object()
{
	WME_Type::print_object();
	cout << " " << m_object->GetValueAsString();
	return false;
}

void WME_String::save_yourself(ofstream* outfile)
{
	WME_Type::save_yourself(outfile);
	*outfile << m_object->GetValueAsString() << endl;
}
