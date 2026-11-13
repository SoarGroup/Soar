#include "portability.h"

/////////////////////////////////////////////////////////////////
// Thread class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Represents a thread.  To use this class
// derive from it and implement the "Run" method.
//
// Then create an instance of your class and call "Start".
//
/////////////////////////////////////////////////////////////////

#include "sml_Utils.h"
#include "thread_Thread.h"
#include "thread_OSspecific.h"

using namespace soar_thread ;

void ThreadStartFunction(void* pThreadObject)
{
    Thread* pThread = static_cast<Thread*>(pThreadObject);
    pThread->Run() ;
    
    // When Run() terminates this "ThreadStartFunction" will terminate
    // which will cause the thread itself to terminate.
    // At least, that works for Windows.  If it fails on Linux we'll
    // need an explicit "End" method.
    pThread->SetStopped(true) ;
}

Thread::Thread()
{
    m_QuitNow.store(false) ;
    m_Stopped.store(true) ;
    m_Started.store(false) ;
}

Thread::~Thread()
{
    m_QuitNow.store(true) ;
    if (m_Thread.joinable())
    {
        if (m_Thread.get_id() != std::this_thread::get_id())
        {
            m_Thread.join() ;
        }
        else
        {
            m_Thread.detach() ;
        }
    }
}

void Thread::Start()
{
    // If we've already been started ignore subsequent attempts
    // to start this thread.
    if (m_Started.exchange(true))
    {
        return ;
    }
    
    m_Stopped.store(false) ;
    m_Thread = std::thread(ThreadStartFunction, this) ;
}

void Thread::Stop(bool waitTillStopped)
{
    // Signal the thread and ask it to stop.
    m_QuitNow.store(true) ;
    
    if (!m_Started.load())
    {
        return ;
    }
    
    if (waitTillStopped && m_Thread.joinable())
    {
        if (m_Thread.get_id() != std::this_thread::get_id())
        {
            m_Thread.join() ;
        }
    }
}

