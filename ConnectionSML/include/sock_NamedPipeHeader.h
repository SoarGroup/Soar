// OS specific header files

// True when running the Windows compiler
#ifdef _WIN32
#define WIN_HEADERS
#endif

// True on Windows, unless building a command line app
#ifdef _WINDOWS
#define WIN_HEADERS
#endif

#ifdef WIN_HEADERS
#ifndef __STDC__ 
#include "winerror.h"

#define PIPE_ERROR_NUMBER	GetLastError()

#define PIPE_INVALID_HANDLE	ERROR_INVALID_HANDLE
#define PIPE_BROKEN			ERROR_BROKEN_PIPE
#define PIPE_ALREADY_EXISTS	ERROR_ALREADY_EXISTS
#define PIPE_BAD			ERROR_BAD_PIPE
#define PIPE_BUSY			ERROR_PIPE_BUSY
#define PIPE_NO_DATA		ERROR_NO_DATA
#define PIPE_NOT_CONNECTED	ERROR_PIPE_NOT_CONNECTED
#define PIPE_MORE_DATA		ERROR_MORE_DATA

#endif

// Linux stuff here

#else	// WIN_HEADERS



#endif