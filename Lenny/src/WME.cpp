#include "WME.h"
#include <sstream>

string WME::GetString() {
	std::stringstream ss;
	ss << "(" << id->GetString() << " ^" << attr->GetString() << " " << val->GetString() << " " << uid << ")";
	return ss.str();
}