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

// This is a Win32 only solution.
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

#endif // SIMPLE_TIMER_H

