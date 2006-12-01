
#include "WME_Type.h"

#include <iostream>
#include <fstream>

using std::cout;
using std::string;
using std::ofstream;

WME_Type::WME_Type(const string& in_id, const string& in_attribute)
: m_id(in_id), m_attribute(in_attribute)
{}

bool WME_Type::print_object()
{
	cout << " ^" << m_attribute;
	return false;
}

void WME_Type::save_yourself(ofstream* outfile)
{
	*outfile << "add " << m_id << " ^" << m_attribute << " ";
}
