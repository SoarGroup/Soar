#include "RunThread.cxx"

void RunThread::Run()
{
    m_kernel->RunAllAgentsForever();
}

