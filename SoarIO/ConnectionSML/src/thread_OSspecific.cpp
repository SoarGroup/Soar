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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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

#else // WIN32

//////////////////////////////////////////////////////////////////////
// Linux Versions -- untested
//////////////////////////////////////////////////////////////////////

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif // HAVE_PTHREAD_H

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif // HAVE_SYS_TYPES_H

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

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

public:
	LinuxMutex()			{ pthread_mutex_init(&mutex,0); }
	virtual ~LinuxMutex()	{ pthread_mutex_destroy(&mutex); }

	void Lock()			{ pthread_mutex_lock(&mutex); }
	void Unlock()		{ pthread_mutex_unlock(&mutex); }
	bool TryToLock()	{ /* TODO: Needs to be written with pthread_mutex_trylock(&mutex) ; */ return false; }
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

