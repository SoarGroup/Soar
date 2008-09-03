/////////////////////////////////////////////////////////////////
// Map event ids to and from strings
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2005
//
/////////////////////////////////////////////////////////////////

#ifndef SML_EVENT_ID_H
#define SML_EVENT_ID_H

#include "sml_ClientEvents.h"

#include <map>
#include <string>
#include <assert.h>

namespace sml {

// This class is used to convert to and from the string form of these IDs
class Events
{
protected:
	// We keep two maps, one for each direction
	typedef std::map<int, std::string> ToStringMap ;
	typedef std::map<std::string, int> ToEventMap  ;

	ToStringMap m_ToStringMap ;
	ToEventMap  m_ToEventMap ;

	void		RegisterEvent(int id, char const* pStr) ;

	/*************************************************************
	* @brief Convert from a string version of an event to the int (enum) version.
	*		 Returns smlEVENT_INVALID_EVENT (== 0) if the string is not recognized.
	*************************************************************/
	inline int InternalConvertToEvent(char const* pStr)
	{
		Events::ToEventMap::iterator mapIter = m_ToEventMap.find(pStr);

		if (mapIter == m_ToEventMap.end())
		{
			return smlEVENT_INVALID_EVENT ;
		}

		return mapIter->second ;
	}

	/*************************************************************
	* @brief Convert from int version of an event to the string form.
	*		 Returns NULL if the id is not recognized.
	*************************************************************/
	inline char const* InternalConvertToString(int id)
	{
		Events::ToStringMap::iterator mapIter = m_ToStringMap.find(id);

		if (mapIter == m_ToStringMap.end())
		{
			return NULL ;
		}

		return mapIter->second.c_str() ;
	}

public:
	// Construction is expensive, so do this once and then use the methods repeatedly
	Events() ;

	// Methods to convert event IDs from int to string values and back.
	// We do this because the list of IDs will likely change from one version of Soar to another
	// but we'd like the list of names to remain largely constant (and just be added to).
	// This will allow clients built with an earlier version to work with later Soar kernels.
	// As a side effect it makes the XML much more human readable as you get event "init-soar" not event 23.

	/*************************************************************
	* @brief Convert from a string version of an event to the int (enum) version.
	*		 Returns smlEVENT_INVALID_EVENT (== 0) if the string is not recognized.
	*************************************************************/
	inline int ConvertToEvent(char const* pStr)
	{
		return InternalConvertToEvent(pStr) ;
	}

	/*************************************************************
	* @brief Convert from int version of an event to the string form.
	*		 Returns NULL if the id is not recognized.
	*************************************************************/
	inline char const* ConvertToString(int id)
	{
		char const* pName = InternalConvertToString(id) ;

		// This event ID needs to be added to the list registered in the constructor
		assert(pName != NULL) ;

		return pName ;
	}
} ;

} ;	// End of namespace

#endif	// Header
