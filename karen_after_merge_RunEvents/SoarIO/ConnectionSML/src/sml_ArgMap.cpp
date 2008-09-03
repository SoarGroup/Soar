#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// ArgMap class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Used to store a mapping between argument names (for a command)
// and <arg> tags in an SML document.
/////////////////////////////////////////////////////////////////

#include "sml_ArgMap.h"
#include "sml_ElementXML.h"
#include "sml_Names.h"
#include "ElementXMLInterface.h"

using namespace sml ;

ArgMap::ArgMap(void)
{
}

ArgMap::~ArgMap(void)
{
	m_ArgList.clear() ;
	m_ArgMap.clear() ;
}

void ArgMap::RecordArg(ElementXML_Handle hArg)
{
	if (!hArg)
		return ;

	m_ArgList.push_back(hArg) ;

	// Look up the argument's name (if it has one)
	char const* pName = ::sml_GetAttribute(hArg, sml_Names::kArgParam) ;

	if (pName)
	{
		m_ArgMap[pName] = hArg ;
	}
}

char const* ArgMap::GetArgValue(char const* pName, int position) const
{
	ElementXML_Handle hArg = GetArgHandle(pName, position) ;

	if (!hArg)
		return NULL ;

	return ::sml_GetCharacterData(hArg) ;
}

ElementXML_Handle ArgMap::GetArgHandle(char const* pName, int position) const
{
	// First try to look up the argument by name.
	xmlArgMapConstIter mapIter = m_ArgMap.find(pName) ;

	if (mapIter != m_ArgMap.end())
	{
		ElementXML_Handle hArg = mapIter->second ;
		return hArg ;
	}

	// Check if we're asking for a position that doesn't exist
	// in this argument list.  (This isn't necessarily an error
	// if we have optional arguments).
	if (position < 0 || position >= (int)m_ArgList.size())
	{
		return NULL ;
	}

	return m_ArgList[position] ;
}
