/////////////////////////////////////////////////////////////////
// ListMap class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This class represents a map from T1 to a list of
// objects of type T2.
//
// Specifically, we use this to maintain a list of functions
// to call when a particular event comes in.
//
// In its current form, the objects being stored are not
// deleted when the map or list is destroyed (since we're planning
// to go from ints to function pointers).
//
/////////////////////////////////////////////////////////////////

#ifndef SML_LIST_MAP_H
#define SML_LIST_MAP_H

#include <map>
#include <list>
#include <algorithm>	// To get std::find

namespace sml {

template <typename T1, typename T2>
class ListMap
{
public:
typedef T1	KeyType ;
typedef T2	ValueType ;

typedef std::list<ValueType>				ValueList ;
typedef typename ValueList::iterator		ValueListIter ;
typedef typename ValueList::const_iterator	ValueListConstIter ;

typedef std::map<KeyType, ValueList*>		KeyMap ;
typedef typename KeyMap::iterator			KeyMapIter ;
typedef typename KeyMap::const_iterator		KeyMapConstIter ;

// In order to do a deletion or lookup using a test on a value
// derive a class from this and implement "isEqual".
// (We use this so we can extract a value from the ValueType and test that
//  without having to rebuild the complete ValueType.  In practice this is helpful).
class ValueTest
{
public:
	virtual bool isEqual(ValueType value) = 0 ;
} ;

protected:
	KeyMap	m_Map ;

public:
	ListMap() { } 

	virtual ~ListMap()
	{
		clear() ;
	}

	void clear()
	{
		// Delete the contents of the map
		for(KeyMapIter mapIter = m_Map.begin(); mapIter != m_Map.end(); ++mapIter)
		{
			// Delete the lists that we created.
			ValueList* pList = mapIter->second ;
			delete pList ;
		}
		m_Map.clear() ;
	}

	void add(KeyType key, ValueType value, bool addToBack)
	{
		// See if we already have a list
		ValueList* pList = getList(key) ;

		// If not, create one
		if (!pList)
		{
			pList = new ValueList() ;
			m_Map[key] = pList ;
		}

		// Add this item to the list
		if (addToBack)
			pList->push_back(value) ;
		else
			pList->push_front(value) ;
	}

	// Remove a specific value from the "key" list
	void remove(KeyType key, ValueType value)
	{
		ValueList* pList = getList(key) ;

		if (pList)
			pList->remove(value) ;
	}

	// Search all values and remove the given one(s)
	// The way we identify which one(s) to remove is by passing in an object which implements "isEqual".
	// When it's true we remove the object.  Returns true if at least one object was removed.
	bool removeAllByTest(ValueTest* pTest)
	{
		bool removedAnObject = false ;

		for (KeyMapIter mapIter = m_Map.begin() ; mapIter != m_Map.end() ; mapIter++)
		{
			KeyType key = mapIter->first ;
			ValueList* pList = getList(key) ;

			if (pList)
			{
				// Walk the list, removing items based on if they match the test
				for (ValueListIter iter = pList->begin() ; iter != pList->end() ;)
				{
					ValueType value = *iter ;

					if (pTest->isEqual(value))
					{
						pList->erase(iter++) ;
						removedAnObject = true ;
					}
					else
						iter++ ;
				}
			}
		}

		return removedAnObject ;
	}

	// Search all values to find a matching value and return the key for that value.
	// The "match" is based on implementing "isEqual" in a ValueTest class.
	// The first match is returned.
	// If there is no match, returns "notFoundKey" that you pass in (so you can choose an appropriate value that's not a key)
	KeyType findFirstKeyByTest(ValueTest* pTest, KeyType notFoundKey)
	{
		for (KeyMapIter mapIter = m_Map.begin() ; mapIter != m_Map.end() ; mapIter++)
		{
			KeyType key = mapIter->first ;
			ValueList* pList = getList(key) ;

			if (pList)
			{
				// Walk the list, removing items based on if they match the test
				for (ValueListIter iter = pList->begin() ; iter != pList->end() ;)
				{
					ValueType value = *iter ;

					if (pTest->isEqual(value))
						return key ;
					else
						iter++ ;
				}
			}
		}

		// Return the value we are passed as a "notFoundKey" if nothing matches
		return notFoundKey ;
	}

	// Search all values to find the first value that matches this test.
	bool findFirstValueByTest(ValueTest* pTest, ValueType* pReturnValue)
	{
		for (KeyMapIter mapIter = m_Map.begin() ; mapIter != m_Map.end() ; mapIter++)
		{
			KeyType key = mapIter->first ;
			ValueList* pList = getList(key) ;

			if (pList)
			{
				// Walk the list, removing items based on if they match the test
				for (ValueListIter iter = pList->begin() ; iter != pList->end() ;)
				{
					ValueType value = *iter ;

					if (pTest->isEqual(value))
					{
						*pReturnValue = value ;
						return  true ;
					}
					else
						iter++ ;
				}
			}
		}

		// Return NULL if nothing matches
		return false ;
	}


	// Remove all values for a specific key
	void removeAll(KeyType key)
	{
		ValueList* pList = getList(key) ;

		if (pList)
			pList.clear() ;
	}

	// Get the number of items in a specific list
	int getListSize(KeyType key)
	{
		ValueList* pList = getList(key) ;

		// There is no list for this key at all
		if (!pList)
			return 0 ;

		return (int)pList->size() ;
	}

	// Returns the list of items associated with the key (can be NULL)
	ValueList*	getList(KeyType key)
	{
		KeyMapIter mapIter = m_Map.find(key) ;

		if (mapIter == m_Map.end())
			return NULL ;
		
		return mapIter->second ;
	}

	int getSize()
	{
		return (int)m_Map.size() ;
	}
};

}//closes namespace

#endif //SML_LIST_MAP_H
