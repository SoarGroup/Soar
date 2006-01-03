/////////////////////////////////////////////////////////////////
// ArgMap class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Used to store a mapping between argument names (for a command)
// and <arg> tags in an SML document.
/////////////////////////////////////////////////////////////////

#ifndef ARGMAP_H
#define ARGMAP_H

#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <map>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif
#include <vector>
#include "ElementXMLHandle.h"

using namespace std ;

namespace sml {

// Forward declarations
class ElementXML ; 

// We need a comparator to make the map we're about to define work with char*
struct strCompareAnalysisXML
{
  bool operator()(const char* s1, const char* s2) const
  {
    return std::strcmp(s1, s2) < 0;
  }
};

// Used to store a map from arg name to the <arg> tag element
typedef std::map<char const*, ElementXML_Handle, strCompareAnalysisXML>	xmlArgMap ;
typedef xmlArgMap::iterator											xmlArgMapIter ;
typedef xmlArgMap::const_iterator									xmlArgMapConstIter ;

// Used to store a vector from arg position to <arg> tag elements(in case this doc is using un-named args)
typedef std::vector<ElementXML_Handle>	xmlArgList ;
typedef xmlArgList::iterator			xmlArgListIter ;
typedef xmlArgList::const_iterator		xmlArgListConstIter ;

class ArgMap
{
protected:
	xmlArgMap	m_ArgMap ;		// From argument name to <arg>
	xmlArgList	m_ArgList ;		// From argument position (0-indexed) to <arg> (this is the fallback list if we don't get a name)

public:
	ArgMap(void);
	~ArgMap(void);

	// NOTE: Must record the arguments in the same order they exist in the original document
	// so we can look them up by position.  This map takes ownership of the pArg object.
	void RecordArg(ElementXML_Handle pArg) ;

	// Returns the value of the named argument.  If that name is not found, returns the value for the
	// given position.  If you are not interested in looking up by position pass -1 for position
	// (in which case you'll get back NULL if the name'd arg doesn't exist).
	char const* GetArgValue(char const* pName, int position) const ;

	// Returns a handle to the <arg> tag.  We return a handle rather than an ElementXML* object
	// so we don't create ElementXML objects we don't need.  (It's just for efficiency).
	ElementXML_Handle GetArgHandle(char const* pName, int position) const ;

};

}

#endif // ARGMAP_H
