#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// OSspecific class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Operating system specific thread methods.
//
/////////////////////////////////////////////////////////////////

#include "thread_OSspecific.h"

using namespace soar_thread ;

#ifdef _WIN32

//////////////////////////////////////////////////////////////////////
// Windows Versions
//////////////////////////////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <process.h>

void soar_thread::BeginThread(ThreadFuncPtr inThreadFuncPtr,void* inParam)
{
	// Must compile with the /MT switch (for multi-threaded libraries)
	// or you'll get a "function not found" error here.
     _beginthread(inThreadFuncPtr,0,inParam) ;
}

bool soar_thread::SleepMillisecs(long msecs)
{
	Sleep(msecs) ;

	return true ;
}

class WindowsMutex : public OSSpecificMutex
{
protected:
   HANDLE mutex;

public:
	WindowsMutex()			{ mutex = CreateMutex(NULL, FALSE, NULL); }
	virtual ~WindowsMutex() { CloseHandle(mutex); }
	void Lock()				{ WaitForSingleObject(mutex, INFINITE); }
	void Unlock()			{ ReleaseMutex(mutex) ; }
	bool TryToLock()		{ DWORD res = WaitForSingleObject(mutex, 0) ; return (res == WAIT_OBJECT_0) ; }
} ;

OSSpecificMutex* soar_thread::MakeMutex()
{
	return new WindowsMutex() ;
}

class WindowsEvent : public OSSpecificEvent
{
protected:
	HANDLE m_Event ;

public:
	WindowsEvent()					{ m_Event = CreateEvent(NULL, FALSE, FALSE, NULL); }
	virtual ~WindowsEvent()			{ CloseHandle(m_Event) ; }
	void WaitForEventForever()		{ WaitForSingleObject(m_Event, INFINITE); }
	bool WaitForEvent(long milli)	{ DWORD res = WaitForSingleObject(m_Event, milli) ; return (res != WAIT_TIMEOUT) ; }
	void TriggerEvent()				{ SetEvent(m_Event) ; }
} ;

OSSpecificEvent* soar_thread::MakeEvent()
{
	return new WindowsEvent() ;
}

#elif defined (__APPLE__) && defined (__MACH__)
//////////////////////////////////////////////////////////////////////
// Mac OS X Version -- in development
//////////////////////////////////////////////////////////////////////

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
//#define MAC_THREAD_DEBUG true
#ifdef MAC_THREAD_DEBUG
#include <iostream>
#endif

struct ThreadArgs {
    ThreadFuncPtr threadFuncPtr;
    void* param;
};

static void* MacThreadFunc(void* thread_args) {
    ThreadArgs* threadArgs = static_cast<ThreadArgs*>(thread_args);
    threadArgs->threadFuncPtr(threadArgs->param);
    delete threadArgs;
	return 0;	
}

void soar_thread::BeginThread(ThreadFuncPtr inThreadFuncPtr,void* inParam)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    
    pthread_t t;

    ThreadArgs* threadArgs = new ThreadArgs();
    threadArgs->threadFuncPtr = inThreadFuncPtr;
    threadArgs->param = inParam;
	
	#ifdef MAC_THREAD_DEBUG
	std::cout << "about to create thread" << std::endl;
	#endif
	
    pthread_create(&t,&attr,MacThreadFunc,threadArgs);

    pthread_attr_destroy(&attr);
}

bool soar_thread::SleepMillisecs(long msecs)
{
	// usleep takes microseconds
	usleep(msecs * 1000) ;

	return true ;
}

class MacMutex : public OSSpecificMutex
{
protected:
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;
	pthread_t lock_owner;
	int lock_count;
public:
	MacMutex()			
	{ 
		pthread_mutexattr_init(&mutexattr);
		//pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&mutex, &mutexattr);
		lock_owner = 0;
		lock_count = 0;
	}
	virtual ~MacMutex()	
	{ 
		pthread_mutex_destroy(&mutex); 
		pthread_mutexattr_destroy(&mutexattr);
	}

	void Lock()
	{
		//have to implement PTHREAD_MUTEX_RECURSIVE_NP by hand
		// reference: 
		// http://publib.boulder.ibm.com/iseries/v5r2/ic2924/index.htm?info/apis/users_55.htm
		#ifdef MAC_THREAD_DEBUG
		std::cout << "Lock: " << (long long)pthread_self() << std::endl;
		#endif
		if (TryToLock())
		{
			//locked safely
			#ifdef MAC_THREAD_DEBUG
			std::cout << "\tnew lock, safely." << std::endl;
			#endif
			return;
		}
		else
		{
			//this thread does not own the lock, standard lock.
			#ifdef MAC_THREAD_DEBUG
			std::cout << "\tlock wait..." << std::endl;
			#endif
			pthread_mutex_lock(&mutex);
			lock_owner = pthread_self();
			lock_count = 1;
		}
	}
	void Unlock()
	{
		#ifdef MAC_THREAD_DEBUG
		std::cout << "Unlock: " << (long long)pthread_self() << std::endl;
		#endif
		if (pthread_equal(lock_owner,pthread_self()))
		{
			//i am the keymaster
			#ifdef MAC_THREAD_DEBUG
			std::cout << "\tlock decremented: " << lock_count -1 << std::endl;
			#endif
			lock_count--;
			if (lock_count <= 0)
			{
				lock_owner = 0;
				lock_count = 0;
				#ifdef MAC_THREAD_DEBUG
				std::cout << "\tlock released!" << std::endl;
				#endif
				pthread_mutex_unlock(&mutex);
			}
		}
		else
		{
			//It was here before... It'll just fail and return an error anyway.
			#ifdef MAC_THREAD_DEBUG
			std::cout << "\tunlock wait..." << std::endl;
			#endif
			if (0 == pthread_mutex_unlock(&mutex))
			{
				lock_owner = 0;
				lock_count = 0;
				#ifdef MAC_THREAD_DEBUG
				std::cout << "\tlock released!" << std::endl;
				#endif
			}
			else
			{
				#ifdef MAC_THREAD_DEBUG
				std::cout << "\tunlock failed!" << std::endl;
				#endif
			}
		}
	}
	bool TryToLock()
	{
		#ifdef MAC_THREAD_DEBUG
		std::cout << "TryToLock: " << (long long)pthread_self() << std::endl;
		#endif
		pthread_t currThread = pthread_self();
		if (pthread_equal(lock_owner, currThread))
		{
			//you own the lock.
			#ifdef MAC_THREAD_DEBUG
			std::cout << "\tlock incremented: " << lock_count +1 << std::endl;
			#endif
			lock_count++;
			return true;
		}
		else
		{
			if (pthread_mutex_trylock(&mutex) != EBUSY)
			{
				//success, locked
				#ifdef MAC_THREAD_DEBUG
				std::cout << "\tlocked safely." << std::endl;
				#endif
				lock_owner = currThread;
				lock_count = 1;
				return true;
			}
			else
			{
				//locking failed
				#ifdef MAC_THREAD_DEBUG
				std::cout << "\tlock failed." << std::endl;
				#endif
				return false;
			}
		}
	}
} ;

OSSpecificMutex* soar_thread::MakeMutex()
{
	return new MacMutex() ;
}

class MacEvent : public OSSpecificEvent
{
protected:
    pthread_cond_t m_cond;
    pthread_mutex_t m_mutex;
    bool m_signaled ;

public:
	MacEvent()
	{
		m_signaled = false ;
		pthread_cond_init(&m_cond, NULL);
		pthread_mutex_init(&m_mutex, NULL);
	}
	virtual ~MacEvent()
	{
		pthread_cond_destroy(&m_cond);
		pthread_mutex_destroy(&m_mutex);
	}
	void WaitForEventForever()
	{
		pthread_mutex_lock(&m_mutex);
		while(!m_signaled)
		{
			pthread_cond_wait(&m_cond,&m_mutex);
		}
		m_signaled = false;
		pthread_mutex_unlock(&m_mutex);
	}
	bool WaitForEvent(long milli)
	{
		/*
		Still Needs to be implemented using -- 
		pthread_cond_timedwait(cond,mutex, abstime ) ;
		*/
		return false;
	}
	void TriggerEvent()
	{
		pthread_mutex_lock(&m_mutex);
		m_signaled = true;
		pthread_mutex_unlock(&m_mutex);
		pthread_cond_signal(&m_cond);
	}
} ;

OSSpecificEvent* soar_thread::MakeEvent()
{
	return new MacEvent() ;
}



#else
//////////////////////////////////////////////////////////////////////
// Linux Versions -- untested
//////////////////////////////////////////////////////////////////////

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

struct ThreadArgs {
    ThreadFuncPtr threadFuncPtr;
    void* param;
};

static void* LinuxThreadFunc(void* thread_args) {
    ThreadArgs* threadArgs = static_cast<ThreadArgs*>(thread_args);
    threadArgs->threadFuncPtr(threadArgs->param);
    delete threadArgs;
	return 0;	
}

void soar_thread::BeginThread(ThreadFuncPtr inThreadFuncPtr,void* inParam)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    
    pthread_t t;

    ThreadArgs* threadArgs = new ThreadArgs();
    threadArgs->threadFuncPtr = inThreadFuncPtr;
    threadArgs->param = inParam;

    pthread_create(&t,&attr,LinuxThreadFunc,threadArgs);

    pthread_attr_destroy(&attr);
}

bool soar_thread::SleepMillisecs(long msecs)
{
	// usleep takes microseconds
	usleep(msecs * 1000) ;

	return true ;
}

class LinuxMutex : public OSSpecificMutex
{
protected:
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;

public:
	LinuxMutex()			
	{ 
		pthread_mutexattr_init(&mutexattr);
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&mutex, &mutexattr); 
	}
	virtual ~LinuxMutex()	
	{ 
		pthread_mutex_destroy(&mutex); 
		pthread_mutexattr_destroy(&mutexattr);
	}

	void Lock()			{ pthread_mutex_lock(&mutex); }
	void Unlock()		{ pthread_mutex_unlock(&mutex); }
	bool TryToLock()	{ return (pthread_mutex_trylock(&mutex) != EBUSY); }
} ;

OSSpecificMutex* soar_thread::MakeMutex()
{
	return new LinuxMutex() ;
}

class LinuxEvent : public OSSpecificEvent
{
protected:
    pthread_cond_t m_cond;
    pthread_mutex_t m_mutex;
    bool m_signaled ;

public:
	LinuxEvent()					{ m_signaled = false ; pthread_cond_init(&m_cond, NULL); pthread_mutex_init(&m_mutex, NULL); }
	virtual ~LinuxEvent()			{ pthread_cond_destroy(&m_cond); pthread_mutex_destroy(&m_mutex); }
	void WaitForEventForever()		{ pthread_mutex_lock(&m_mutex); while(!m_signaled) { pthread_cond_wait(&m_cond,&m_mutex); } m_signaled = false; pthread_mutex_unlock(&m_mutex); }
	bool WaitForEvent(long milli)	{ /* Still Needs to be implemented using -- pthread_cond_timedwait(cond,mutex, abstime ) ; */ return false; }
	void TriggerEvent()				{ pthread_mutex_lock(&m_mutex); m_signaled = true; pthread_mutex_unlock(&m_mutex); pthread_cond_signal(&m_cond); }
} ;

OSSpecificEvent* soar_thread::MakeEvent()
{
	return new LinuxEvent() ;
}

#endif // _WIN32

