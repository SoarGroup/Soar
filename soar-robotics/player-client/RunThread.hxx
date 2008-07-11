#ifndef RUN_THREAD_HXX
#define RUN_THREAD_HXX

#include "RunThread.h"

void RunThread::Run()
{
    m_kernel->RunAllAgentsForever();
}

#endif // RUN_THREAD_HXX

