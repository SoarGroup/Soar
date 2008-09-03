/////////////////////////////////////////////////////////////////
// IdMap class
//
// Author: Douglas Pearson	www.threepenny.net
// Date  : August 2004
//
// Simple class used to record a list of objects
// that we own.  When this map is destroyed, we delete
// the objects in the map.
//
// If you'd like to have a map, but the objects shouldn't
// be deleted, then make sure to clear it before the destructor is called.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_ID_MAP_H
#define SML_ID_MAP_H

#include <map>

namespace sml {

class ClientObject ;

// Function object used for ordering agentMap keys
struct IDStringLessThan
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

// Map and interator (and associated) typedefs
typedef std::map<const char*, ClientObject*, IDStringLessThan> idMap_t;
typedef idMap_t::iterator		idMapItr_t;
typedef idMap_t::const_iterator idMapItrConst_t;

class IdMap
{
protected:
	idMap_t	m_Map ;

public:
	IdMap(void);
	virtual ~IdMap(void);

	// Find the client object, based on id (returns NULL if no entry)
	ClientObject* lookup(char const* pID) const ;

	ClientObject* getFirst() ;

	// Add a new client object into the map, indexed by its GetId() value.
	void add(ClientObject* pObject) ;

	// Remove an object from the map (optionally deleting it).  Returns true if found it.
	bool remove(char const* pID, bool deleteObject = true) ;

	// Go through all objects in the map and call IRelease::Release on them.
	// Then optionally delete the entire map.
	void releaseAll(bool deleteObjects = true) ;

	// Remove all objects from the map (optionally deleting them).
	void clear(bool deleteObjects = true) ;

	// Return the number of objects in the map.
	int  size()	const { return (int)m_Map.size() ; }

	// Provide full access in case we need it
	idMap_t	getMap() { return m_Map ; }
};

}	// end of namespace

#endif	// SML_ID_MAP_H