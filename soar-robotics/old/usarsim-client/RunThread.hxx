#ifndef RUN_THREAD_HXX
#define RUN_THREAD_HXX

#include "RunThread.h"
#include <iostream>

void RunThread::Run()
{
	m_kernel->RunAllAgentsForever();
	std::cout << "Run finished." << std::endl;
}

#endif // RUN_THREAD_HXX

