#include <portability.h>

#include "sml_Utils.h"
#include <assert.h>

void soar_sleep(long secs, long msecs)
{
	assert(msecs < 1000 && "Specified milliseconds too large; use seconds argument to specify part of time >= 1000 milliseconds");
#ifdef _WIN32

	Sleep( (secs * 1000) + msecs) ;

#else // not _WIN32

	// if sleep 0 is requested, then don't sleep at all (an actual sleep 0 is very slow on Linux, and probably OS X, too)
	if(msecs || secs) {
		struct timespec sleeptime;
		sleeptime.tv_sec = secs;
		sleeptime.tv_nsec = msecs * 1000000;
		nanosleep(&sleeptime, 0);
	}

#endif // not _WIN32
}

