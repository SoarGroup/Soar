/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* stl_support.h */

/*************************************************************************
 * This file defines two custom STL allocators for use with Soar,
 *  as well as some typedefs to make them easier to use.
 *
 * SoarMemoryPoolAllocator allows the STL to use Soar's memory pools.
 * SoarMemoryAllocator allows the STL to use Soar's allocate_memory and
 *  free_memory functions so memory statistics can be kept on objects
 *  not in the pools.
 *************************************************************************/

#ifndef STL_SUPPORT_H
#define STL_SUPPORT_H

#include <list>
#include <vector>
#include <set>
#include <deque>

#include "agent.h"

////////////////////////////////////
//
// This is a custom STL allocator which uses Soar's memory pools.
// The default constructor is left inaccessible because we MUST
//  use the constructor that takes the agent and pool args. If
//  this is not used, then these will be undefined and the program
//  will crash when it tries to allocate memory.
//
// This code is heavily based on an example from Rogue Wave Software
//  which I found here: http://www.roguewave.com/support/docs/sourcepro/toolsug/12-6.html
//
////////////////////////////////////

template <class T>
class SoarMemoryPoolAllocator
{
private:
	agent* m_agent;
	memory_pool* m_pool;
	SoarMemoryPoolAllocator() {}
public:
  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;
  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;
  
  SoarMemoryPoolAllocator(const SoarMemoryPoolAllocator& alloc) {m_agent = alloc.getAgent(); m_pool = alloc.getPool(); }
  SoarMemoryPoolAllocator(agent* a, memory_pool* p) : m_agent(a), m_pool(p) {}
  
  agent*		getAgent() const { return m_agent; }
  memory_pool*	getPool() const { return m_pool; }

  pointer   allocate(size_type n, const void * = 0) {
			  T* t;
			  allocate_with_pool(m_agent, m_pool, &t);
              return t;
            }
  
  void      deallocate(void* p, size_type) {
              if (p) {
			    free_with_pool(m_pool, p);
              } 
            }

  pointer           address(reference x) const { return &x; }
  const_pointer     address(const_reference x) const { return &x; }
  SoarMemoryPoolAllocator<T>& operator=(const SoarMemoryPoolAllocator&) { return *this; }
  void              construct(pointer p, const T& val) 
                    { new ((T*) p) T(val); }
  void              destroy(pointer p) { p->~T(); }

  size_type         max_size() const { return size_t(-1); }

  template <class U>
  struct rebind { typedef SoarMemoryPoolAllocator<U> other; };

  template <class U>
	  SoarMemoryPoolAllocator(const SoarMemoryPoolAllocator<U>& alloc) { m_agent = alloc.getAgent(); m_pool = alloc.getPool(); }

  template <class U>
  SoarMemoryPoolAllocator& operator=(const SoarMemoryPoolAllocator<U>&) { return *this; }
};

//
// These will make it easier to define Soar STL containers for wmes (example usage immediately follows)
// Add new typedefs for other types as needed
//
typedef std::list<wme*, SoarMemoryPoolAllocator<wme*> > SoarSTLWMEPoolList;
typedef std::vector<wme*, SoarMemoryPoolAllocator<wme*> > SoarSTLWMEPoolVector;
typedef std::vector<wme*, SoarMemoryPoolAllocator<wme*> > SoarSTLWMEPoolDeque;
typedef std::set<wme*, std::less<wme*>, SoarMemoryPoolAllocator<wme*> > SoarSTLWMEPoolSet;
//
// Example usage
// Notice how we give the constructor our custom SoarMemoryPoolAllocator with the agent and memory pool to use
// Also note how the set is different from the others
//
// SoarSTLWMEPoolList* list = new SoarSTLWMEList(SoarMemoryPoolAllocator<wme*>(thisAgent, &thisAgent->wme_pool));
// SoarSTLWMEPoolVector* vect = new SoarSTLWMEVector(SoarMemoryPoolAllocator<wme*>(thisAgent, &thisAgent->wme_pool));
// SoarSTLWMEPoolDeque* deq = new SoarSTLWMEDeque(SoarMemoryPoolAllocator<wme*>(thisAgent, &thisAgent->wme_pool));
// SoarSTLWMEPoolSet* set = new SoarSTLWMESet(std::less<wme*>(), SoarMemoryPoolAllocator<wme*>(thisAgent, &thisAgent->wme_pool));
//

////////////////////////////////////
//
// This is a custom STL allocator which uses Soar's allocate_memory and free_memory functions
//  so we can get memory usage statistics with STL stuff.
// The default constructor is left inaccessible because we MUST
//  use the constructor that takes the agent and usage args. If
//  this is not used, then these will be undefined and the program
//  will crash when it tries to allocate memory.
//
// This code is heavily based on an example from Rogue Wave Software
//  which I found here: http://www.roguewave.com/support/docs/sourcepro/toolsug/12-6.html
//
////////////////////////////////////

template <class T>
class SoarMemoryAllocator
{
private:
	agent* m_agent;
	int m_usage;
	SoarMemoryAllocator() {}
public:
  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;
  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;
  
  SoarMemoryAllocator(const SoarMemoryAllocator& alloc) {m_agent = alloc.getAgent(); m_usage = alloc.getUsage(); }
  SoarMemoryAllocator(agent* a, int u) : m_agent(a), m_usage(u) {}
  
  agent*	getAgent() const { return m_agent; }
  int		getUsage() const { return m_usage; }

  pointer   allocate(size_type n, const void * = 0) {
			  T* t = static_cast<T*>(allocate_memory(m_agent, n * sizeof(T), m_usage));
              return t;
            }
  
  void      deallocate(void* p, size_type) {
              if (p) {
			    free_memory(m_agent, p, m_usage);
              } 
            }

  pointer           address(reference x) const { return &x; }
  const_pointer     address(const_reference x) const { return &x; }
  SoarMemoryAllocator<T>& operator=(const SoarMemoryAllocator&) { return *this; }
  void              construct(pointer p, const T& val) 
                    { new ((T*) p) T(val); }
  void              destroy(pointer p) { p->~T(); }

  size_type         max_size() const { return size_t(-1); }

  template <class U>
  struct rebind { typedef SoarMemoryAllocator<U> other; };

  template <class U>
	  SoarMemoryAllocator(const SoarMemoryAllocator<U>& alloc) { m_agent = alloc.getAgent(); m_usage = alloc.getUsage(); }

  template <class U>
  SoarMemoryAllocator& operator=(const SoarMemoryAllocator<U>&) { return *this; }
};

//
// These will make it easier to define Soar STL containers for wmes (example usage immediately follows)
// Add new typedefs for other types as needed
//
typedef std::list<wme*, SoarMemoryAllocator<wme*> > SoarSTLWMEList;
typedef std::vector<wme*, SoarMemoryAllocator<wme*> > SoarSTLWMEVector;
typedef std::vector<wme*, SoarMemoryAllocator<wme*> > SoarSTLWMEDeque;
typedef std::set<wme*, std::less<wme*>, SoarMemoryAllocator<wme*> > SoarSTLWMESet;
//
// Example usage
// Notice how we give the constructor our custom SoarAllocator with the agent and memory type to use
// Also note how the set is different from the others
//
// SoarSTLWMEList* list = new SoarSTLWMEList(SoarMemoryAllocator<wme*>(thisAgent, MISCELLANEOUS_MEM_USAGE));
// SoarSTLWMEVector* vect = new SoarSTLWMEVector(SoarMemoryAllocator<wme*>(thisAgent, MISCELLANEOUS_MEM_USAGE));
// SoarSTLWMEDeque* deq = new SoarSTLWMEDeque(SoarMemoryAllocator<wme*>(thisAgent, MISCELLANEOUS_MEM_USAGE));
// SoarSTLWMESet* set = new SoarSTLWMESet(std::less<wme*>(), SoarMemoryAllocator<wme*>(thisAgent, MISCELLANEOUS_MEM_USAGE));
//

#endif //STL_SUPPORT_H
