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

#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <map>
#include <list>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif
#include <string>
#include <algorithm>	// To get std::find

namespace sml {

template <typename T>
class ObjectMap
{
typedef T	DataType ;

// Used to store a map from a name to an object
// The name should be owned by the object being stored (so we only delete the object, never the name)

typedef std::map<std::string, DataType>		InternalMap ;
//typedef typename std::map<std::string, T>::iterator			InternalMapIter ;
//typedef typename std::map<std::string, T>::const_iterator	InternalMapConstIter ;
typedef typename InternalMap::iterator			InternalMapIter ;
typedef typename InternalMap::const_iterator	InternalMapConstIter ;

protected:
	InternalMap	m_Map ;

public:
	ObjectMap() { } 

	virtual ~ObjectMap()
	{
		clear() ;
	}

	void clear()
	{
		// Delete the contents of the map
		for(InternalMapIter mapIter = m_Map.begin(); mapIter != m_Map.end(); ++mapIter)
		{
			DataType pObject = mapIter->second ;

			// We only delete the object.  The name is a pointer
			// into the object structure, so it's deleted when the object is deleted.
			delete pObject ;
		}
		m_Map.clear() ;
	}

	int size() const { return (int)m_Map.size() ; }

	bool contains(DataType value)
	{
		for (InternalMapIter mapIter = m_Map.begin() ; mapIter != m_Map.end() ; mapIter++)
		{
			if (mapIter->second == value)
				return true ;
		}

		return false ;
	}

	DataType getIndex(int index)
	{
		for (InternalMapIter mapIter = m_Map.begin() ; mapIter != m_Map.end() ; mapIter++)
		{
			if (index == 0)
				return mapIter->second ;
			index-- ;
		}

		return NULL ;
	}

	void add(char const* pName, DataType pObject)
	{
		// If we already have an object registered with this name delete it.
		// Otherwise we'll have a memory leak.
		remove(pName) ;

		m_Map[pName] = pObject ;
	}

	// Remove all agents not on this list
	void keep(std::list<DataType>* pKeepList)
	{
		for (InternalMapIter mapIter = m_Map.begin() ; mapIter != m_Map.end() ;)
		{
			DataType pObject = mapIter->second ;

			// For lists, you have to use std::find, they don't have a member function
			if (std::find(pKeepList->begin(), pKeepList->end(), pObject) != pKeepList->end())
				mapIter++ ;
			else
				m_Map.erase(mapIter++) ; // changed by voigtjr
		}
	}

	DataType find(char const *pName) const
	{
		InternalMapConstIter mapIter = m_Map.find(pName);

		if (mapIter == m_Map.end())
			return NULL ;

		return mapIter->second ;
	}

	bool remove(char const* pName, bool deleteObject = true)
	{
		InternalMapIter mapIter = m_Map.find(pName);

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
