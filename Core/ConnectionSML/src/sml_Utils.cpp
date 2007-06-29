#include <portability.h>

#include "sml_Utils.h"
#include "sock_Debug.h"
#include <assert.h>

/////////////////////////////////////////////////////////////////////
// Function name  : soar_sleep
// 
// Argument       : long secs
// Argument       : long msecs
// Return type    : void 	
// 
// Description	  : Sleep for the specified seconds and milliseconds
//
/////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////
// Function name  : ReportSystemErrorMessage
// 
// Return type    : void 	
// 
// Description	  : Get the text of the most recent system error
//
/////////////////////////////////////////////////////////////////////
void ReportSystemErrorMessage()
{
	int error = ERROR_NUMBER ;

	char* message;

#ifdef _WIN32
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		error,
		0,
		(char*) &message,
		0, 0 );
#else
	message = strerror(error);
#endif // _WIN32

	PrintDebugFormat("Error: %s", message);

#ifdef _WIN32
	LocalFree(message);
#endif

}