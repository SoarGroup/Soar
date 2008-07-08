#include "RunThread.h"

void RunThread::Run()
{
    m_kernel->RunAllAgentsForever();
}

