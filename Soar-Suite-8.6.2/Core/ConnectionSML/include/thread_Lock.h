/////////////////////////////////////////////////////////////////
// Lock class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Locks a section of code so that only one thread can be executing in it
// at a time.
//
// Use it like this:
//
// MyClass 
// {
//    private: Mutex m_Mutex ;
// 
//    public: void MyMethod() { Lock loc(&m_Mutex) ; Safely modify data now ; }
// }
//
// We also include "ConditionLock" here.
//
// Use it like this:
// 
// MyClass
// {
//    private: Mutex m_Mutex ;
//
//    public: bool MyMethod()
//    {
//        ConditionalLock lock ;
//        if (lock.TryToLock(&m_Mutex))
//           { modify data now }
//        else return false ; 
//    }
// }
//
/////////////////////////////////////////////////////////////////

#ifndef THREAD_LOCK_H
#define THREAD_LOCK_H

namespace soar_thread {

// Forward declarations
class Lock ;
class ConditionalLock ;
class OSSpecificMutex ;

// The more primitive mutex class which does the work for the Lock class.
// You create one of these in the object that requires locking
// and then create "lock" objects as necessary to actually do the locking.
// This is safer than exposing the "Lock" and "Unlock" methods which is
// dangerous because it's east to forget to Unlock (or to have an exception or
// other unexcepted exit out of a function) causing a deadlock.
class Mutex
{
	friend class Lock ;
	friend class ConditionalLock ;

protected:
	// OS specific implementation
	OSSpecificMutex*	m_Imp ;

public:
	Mutex() ;
	~Mutex() ;

protected:
	void Lock() ;
	void Unlock() ;
	bool TryToLock() ;
} ;

class Lock
{
protected:
	Mutex* m_Mutex ;

public:
	// When the lock object is created, we lock the mutex.
	// The lock is released when the lock object is destroyed.
	Lock(Mutex* mutex) { m_Mutex = mutex ; m_Mutex->Lock() ; }

	// If you require a finer level of control you can manually lock
	// and unlock the mutex at specific points, but for many simple uses
	// this is not required.
	void UnlockMutex()	   { m_Mutex->Unlock() ; }
	void LockMutex()	   { m_Mutex->Lock() ; }

	// Destroy the lock, unlocking the underlying mutex.
	~Lock()			  { m_Mutex->Unlock() ; }
} ;

// This version of the Lock class allows a user to determine
// whether we'll get the lock or not and behave accordingly.
// It can be useful in certain situations.  If in doubt, just use the "Lock" class.
class ConditionalLock
{
protected:
	Mutex* m_Mutex ;

public:
	ConditionalLock() { m_Mutex = 0 ; }

	// Attempt to get a lock on the mutex.
	// This call will not block, but will return true if
	// the lock is achieved and false if not.
	// If the lock is obtained, the destructor will release it, as usual for a lock.
	bool TryToLock(Mutex* mutex)
	{
		// We can only use this on a single mutex at a time
		if (m_Mutex)
			return false ;

		// Try to obtain the lock
		if (!mutex->TryToLock())
			return false ;

		// Record the mutex that we're using
		m_Mutex = mutex ;
		return true ;
	}

	~ConditionalLock()	{ if (m_Mutex) m_Mutex->Unlock() ; }
} ;

} // Namespace

#endif // THREAD_LOCK_H
