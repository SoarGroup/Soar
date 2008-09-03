#include "thread_Thread.h"
#include "thread_Lock.h"
#include "thread_Event.h"

#include <stdlib.h>
#include <iostream>

#ifdef _MSC_VER
// Use Visual C++'s memory checking functionality
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // _MSC_VER

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(1000*x)
#endif // _WIN32

using namespace soar_thread;
using namespace std;

class Test
{
protected:
	char const* m_pTestName ;

public:
	Test(char const* pTestName)
	{
		m_pTestName = pTestName ;
	}

	bool Result(bool ok)
	{
		if (ok)
			printf("Test %s passed\n\n", m_pTestName) ;
		else
			printf("Test %s failed\n\n", m_pTestName) ;

		return ok ;
	}

	virtual bool Run() = 0 ;

} ;

class ThreadBusy : public Thread {
public:
	ThreadBusy() { }

	void Run() {
		cout << "*";
		cout.flush();
		while (!this->m_QuitNow) {}
	}
};

const int BUF_SIZE = 10;
int buf[BUF_SIZE];
int bufStart = 0;
int bufEnd = 0;
int bufSize = 0;
Mutex* g_pMutex = 0;
Event* g_pWriteEvent = 0;
Event* g_pReadEvent = 0;

class ThreadReader : public Thread {
public:
	ThreadReader() { }

	void Run() {
		Lock* p_Lock;
		while (!this->m_QuitNow) {
			p_Lock = new Lock(g_pMutex);
			while (bufStart == bufEnd) {
				delete p_Lock;
				cout << "R";
				cout.flush();
				if (rand() & 2) {
					Sleep(rand() % 1000);
				}
				g_pWriteEvent->WaitForEventForever();
				p_Lock = new Lock(g_pMutex);
			}
			++bufStart;
			bufStart %= BUF_SIZE;
			g_pReadEvent->TriggerEvent();
			--bufSize;
			cout << bufSize;
			cout.flush();
			delete p_Lock;
		}
	}
};

class ThreadWriter : public Thread {
public:
	ThreadWriter() { }

	void Run() {
		Lock* p_Lock;
		for (int i = 0; !this->m_QuitNow; ++i) {
			p_Lock = new Lock(g_pMutex);
			while (bufStart == ((bufEnd + 1) % BUF_SIZE)) {
				delete p_Lock;
				cout << "W";
				cout.flush();
				if (rand() & 2) {
					Sleep(rand() % 1000);
				}
				g_pReadEvent->WaitForEventForever();
				p_Lock = new Lock(g_pMutex);
			}
			buf[bufEnd] = i;
			++bufEnd;
			bufEnd %= BUF_SIZE;
			g_pWriteEvent->TriggerEvent();
			++bufSize;
			cout << bufSize;
			cout.flush();
			delete p_Lock;
		}
	}
};

class TestThread_1 : public Test
{
public:
	TestThread_1() : Test("'busy thread (wait for stop)'") { }

	bool Run()
	{
		cout << "Starting " << m_pTestName << "...";
		ThreadBusy thread;
		thread.Start();
		sleep(5);
		thread.Stop(true);
		return Result(thread.IsStopped()) ;
	}
};

class TestThread_2 : public Test
{
public:
	TestThread_2() : Test("'busy thread (stop, sleep, check)'") { }

	bool Run()
	{
		ThreadBusy thread;
		cout << "Starting " << m_pTestName << "...";
		thread.Start();
		sleep(5);
		thread.Stop(false);
		sleep(2);
		return Result(thread.IsStopped()) ;
	}
};

class TestThread_3 : public Test
{
public:
	TestThread_3() : Test("'classic simple buffer reader/writer'") { }

	bool Run()
	{
		g_pMutex = new Mutex();
		g_pReadEvent = new Event();
		g_pWriteEvent = new Event();
		
		ThreadReader threadR;
		ThreadWriter threadW;
		cout << "Starting " << m_pTestName << "... ";
		threadW.Start();
		sleep(1);
		threadR.Start();
		sleep(5);
		threadR.Stop(false);
		threadW.Stop(false);
		sleep(2);
		
		delete g_pMutex;
		delete g_pReadEvent;
		delete g_pWriteEvent;
		
		return Result(threadR.IsStopped() && threadW.IsStopped()) ;
	}
};

class TestThread_4 : public Test
{
public:
	TestThread_4() : Test("'recursive locking'") { }

	bool Run()
	{
		g_pMutex = new Mutex();
		
		cout << "Starting " << m_pTestName << "... ";
		Lock* pLock1 = new Lock(g_pMutex); 
		cout << "About to lock again, this may deadlock if not implemented correctly (control-c to quit)" << endl;
		Lock* pLock2 = new Lock(g_pMutex);
		
		delete pLock1;
		delete pLock2;
		delete g_pMutex;
		
		return Result(true) ;
	}
};

int main(int argc, char* argv[])
{
	// Start off with some general tests of ElementXML
	TestThread_1 test1;
	TestThread_2 test2;
	TestThread_3 test3;
	TestThread_4 test4;

	bool ok = true ;

	ok = ok && test1.Run() ;
	ok = ok && test2.Run() ;
	ok = ok && test3.Run() ;
	ok = ok && test4.Run() ;	 // Warning:  should never return!

	if (ok)
		printf("\n\nAll tests passed\n") ;
	else
		printf("\n\n*** Error: At least one test failed.  Stopped testing at that point.\n") ;

#ifdef _MSC_VER
//	A deliberate memory leak which I can use to test the memory checking code is working.
//	char* pTest = new char[10] ;

	printf("\nNow checking memory.  Any leaks will appear below.\nNothing indicates no leaks detected.\n") ;
	printf("\nIf no leaks appear here, but some appear in the output\nwindow in the debugger, they have been leaked from a DLL.\nWhich is reporting when it's unloaded.\n\n") ;

	// Set the memory checking output to go to Visual Studio's debug window (so we have a copy to keep)
	// and to stdout so we can see it immediately.
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	// Now check for memory leaks.
	// This will only detect leaks in objects that we allocate within this executable and static libs.
	// If we allocate something in a DLL then this call won't see it because it works by overriding the
	// local implementation of malloc.
	_CrtDumpMemoryLeaks();
#endif // _MSC_VER

	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	printf("\n\nPress <return> to exit\n") ;
	char line[100] ;
	char* str = gets(line) ;
	str = 0;

	return 0;
}

