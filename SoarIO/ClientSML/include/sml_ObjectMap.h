/////////////////////////////////////////////////////////////////
// ObjectMap class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used to keep a map of objects which we own.
// When the map is deleted, so are the objects within the map.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_OBJECT_MAP_H
#define SML_OBJECT_MAP_H

#include <map>

namespace sml {

// We need a comparator to make the map we're about to define work with char*
struct strCompareObjectMap
{
  bool operator()(const char* s1, const char* s2) const
  {
    return std::strcmp(s1, s2) < 0;
  }
};

template <typename T>
class ObjectMap
{
typedef T	DataType ;

// Used to store a map from a name to an object
// The name should be owned by the object being stored (so we only delete the object, never the name)

typedef std::map<const char*, DataType, strCompareObjectMap>	InternalMap ;

protected:
	InternalMap	m_Map ;

public:
	ObjectMap() { } 

	virtual ~ObjectMap()
	{
		// Delete the contents of the map
		for(InternalMap::iterator mapIter = m_Map.begin(); mapIter != m_Map.end(); ++mapIter)
		{
			DataType pObject = mapIter->second ;

			// We only delete the object.  The name is a pointer
			// into the object structure, so it's deleted when the object is deleted.
			delete pObject ;
		}
	}

	// NOTE: The name of the object being added needs to be owned by the object (we just delete the object at the end)
	// Also, if there's already an object with this name in the map, this could cause a leak.  The caller is currently
	// in charge of checking for that.
	void add(char const* pName, DataType pObject)
	{
		m_Map[pName] = pObject ;
	}

	DataType find(char const *pName) const
	{
		InternalMap::const_iterator mapIter = m_Map.find(pName);

		if (mapIter == m_Map.end())
			return NULL ;

		return mapIter->second ;
	}

	bool remove(char const* pName, bool deleteObject = true)
	{
		InternalMap::iterator mapIter = m_Map.find(pName);

		if (mapIter == m_Map.end())
			return false ;

		DataType pObject = mapIter->second ;

		if (deleteObject)
			delete pObject ;

		m_Map.erase(mapIter) ;

		return true ;
	}
};

}//closes namespace

#endif //SML_OBJECT_MAP_H