#include "WME_Shared.h"
#include "QL_Interface.h"

#include <fstream>
#include <iostream>

using std::string;
using sml::Identifier; using sml::Agent;
using std::ofstream; using std::cout; using std::endl;

WME_Shared::WME_Shared(const string& in_parent, const string& in_attribute, const string& in_value, Agent* pAgent, Identifier* id_start, Identifier* id_end)
: WME_Type(in_parent, in_attribute), m_value(in_value)
{
	m_object = pAgent->CreateSharedIdWME(id_start, in_attribute.c_str(), id_end);
}

WME_Shared::WME_Shared(const string& in_id_name, const string& in_attribute, Identifier* in_object)
: WME_Type(in_id_name, in_attribute)
{
	m_value = in_object->GetValueAsString();
	m_object = in_object;
}

WME_Shared::~WME_Shared()
{
	//Agent* pAgent = QL_Interface::instance().get_agent_ptr();
	//destroy_sml_object(pAgent);
}

Smart_Pointer<WME_Shared> WME_Shared::create(const string& in_parent, const string& in_attribute, const string& in_value, Agent* pAgent, Identifier* in_start, Identifier* in_end)
{
	Smart_Pointer<WME_Shared> ptr = new WME_Shared(in_parent, in_attribute, in_value, pAgent, in_start, in_end);
	return ptr;
}

Smart_Pointer<WME_Shared> WME_Shared::create_from_sml_object(const string& in_id_name, Identifier* object)
{
	Smart_Pointer<WME_Shared> ptr = new WME_Shared(in_id_name, object->GetAttribute(), object);
	return ptr;
}

void WME_Shared::destroy_sml_object(Agent*)
{
//	pAgent->DestroyWME(m_object);
}

void WME_Shared::save_yourself(ofstream* outfile)
{
	WME_Type::save_yourself(outfile);
	*outfile << "/" << m_value << endl;
}

bool WME_Shared::print_object()
{
	WME_Type::print_object();
	cout << " " << m_value;
	return false;
}

