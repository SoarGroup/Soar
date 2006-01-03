#ifndef IMP_CODE_GENERATION_UTILITIES
#define IMP_CODE_GENERATION_UTILITIES

#include <string>

//function object for maps
struct stringsLess
{
	bool operator() (const std::string first, const std::string second) const
	{
		return first < second;
	}
};

#endif IMP_CODE_GENERATION_UTILITIES