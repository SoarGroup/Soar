/////////////////////////////////////////////////////////////////
// OSspecific class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Operating system specific thread methods.
//
/////////////////////////////////////////////////////////////////

#ifndef THREAD_OS_SPECIFIC_H
#define THREAD_OS_SPECIFIC_H

namespace soar_thread {

// Methods for starting a thread
typedef void (*ThreadFuncPtr)(void*);

void BeginThread(ThreadFuncPtr inThreadFuncPtr,void* inParam) ;

bool SleepThread(long seconds, long msecs) ;

// Methods for creating a mutex
class OSSpecificMutex
{
public:
	virtual ~OSSpecificMutex() {} ;
	virtual void Lock() = 0 ;
	virtual void Unlock() = 0 ;
	virtual bool TryToLock() = 0 ;
} ;

OSSpecificMutex* MakeMutex() ;

// Methods for creating an event
class OSSpecificEvent
{
public:
	virtual ~OSSpecificEvent() {} ;
	virtual void WaitForEventForever() = 0 ;
	virtual bool WaitForEvent(long seconds, long milliseconds) = 0 ;
	virtual void TriggerEvent() = 0 ;
} ;

OSSpecificEvent* MakeEvent() ;

} // Namespace


#endif // THREAD_OS_SPECIFIC_H
