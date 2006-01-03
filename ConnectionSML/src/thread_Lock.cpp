#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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
//    public: void MyMethod() { Lock(&m_Mutex) ; Safely modify data now ; }
// }
//
/////////////////////////////////////////////////////////////////

#include "thread_Lock.h"
#include "thread_OSspecific.h"

using namespace soar_thread ;

Mutex::Mutex()
{
	m_Imp = MakeMutex() ;
}

Mutex::~Mutex()
{
	delete m_Imp ;
}

void Mutex::Lock()
{
	m_Imp->Lock() ;
}

void Mutex::Unlock()
{
	m_Imp->Unlock() ;
}

bool Mutex::TryToLock()
{
	return m_Imp->TryToLock() ;
}
