/////////////////////////////////////////////////////////////////
// Simpler timer
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This is a timer class that uses the Win32 high resolution timers.
// It's just a handy class that we can drop into a project to get
// some timing numbers.
// It is NOT connected to the rest of the code in this profiler project.
// It is just this one, standalone header file.
//
// Usage is just:
// #include "simple_timer.h"
//
// SimpleTimer timer ;
// timer.Start() ; // Optional (if ommitted, starts when object created).
// double elapsed1 = timer.Elapsed() ;
// double elapsed2 = timer.Elapsed() ;
//
/////////////////////////////////////////////////////////////////

#ifndef SIMPLE_TIMER_H
#define SIMPLE_TIMER_H

#ifdef _WIN32
#include <Windows.h>

class SimpleTimer
{
private:
	LARGE_INTEGER startTime, stopTime;
	LARGE_INTEGER freq;

    // Constructor
public:
	SimpleTimer()
    {
        QueryPerformanceFrequency(&freq) ;
        QueryPerformanceCounter(&startTime);
    }

    // Start the timer
	// Returns the current time
    double Start()
    {
        QueryPerformanceCounter(&startTime);
		return (double)startTime.QuadPart / (double)freq.QuadPart ;
    }

	// Returns the elapsed time since start
    double Elapsed()
    {
        QueryPerformanceCounter(&stopTime);
		return (double)(stopTime.QuadPart - startTime.QuadPart) / (double) freq.QuadPart;
    }
} ;
#else	// _WIN32

// Linux version using clock()
#include <time.h>

class SimpleTimer
{
private:
	clock_t startTime, stopTime;
	const static clock_t freq = CLOCKS_PER_SEC ;

    // Constructor
public:
	SimpleTimer()
    {
		startTime = clock() ;
    }

    // Start the timer
	// Returns the current time
    double Start()
    {
		startTime = clock() ;
		return (double)startTime / (double)freq ;
    }

	// Returns the elapsed time since start
    double Elapsed()
    {
		stopTime = clock() ;
		return (double)(stopTime - startTime) / (double) freq;
    }
} ;

#endif	// _WIN32

#endif // SIMPLE_TIMER_H

