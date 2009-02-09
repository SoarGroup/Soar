#ifndef RUN_THREAD_H
#define RUN_THREAD_H

#include "sml_Client.h"
#include "thread_Thread.h"

class RunThread: public soar_thread::Thread
{
public:
    RunThread( sml::Kernel* kernel ) : m_kernel( kernel ) {}
    
    virtual void Run();
    
private:
    sml::Kernel* m_kernel;
};

#endif
