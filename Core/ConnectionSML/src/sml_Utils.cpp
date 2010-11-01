#include <portability.h>

#include "sml_Utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>



/////////////////////////////////////////////////////////////////////
// Function name  : Sleep
// 
// Argument       : int secs
// Argument       : int msecs
// Return type    : void 	
// 
// Description	  : Sleep for the specified seconds and milliseconds
//
/////////////////////////////////////////////////////////////////////
void sml::Sleep(int secs, int msecs)
{
	assert(msecs < 1000 && "Specified milliseconds too large; use seconds argument to specify part of time >= 1000 milliseconds");
#ifdef _WIN32

	// Use the Windows API Sleep function (need to specify the Global namespace to distinguish from sml::Sleep)
	::Sleep( (secs * 1000) + msecs) ;

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
// Function name  : sml::ReportSystemErrorMessage
// 
// Return type    : void 	
// 
// Description	  : Get the text of the most recent system error
//
/////////////////////////////////////////////////////////////////////
void sml::ReportSystemErrorMessage()
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

	//PrintDebugFormat("Error: %s", message);

#ifdef _WIN32
	LocalFree(message);
#endif

}

/////////////////////////////////////////////////////////////////
// Debug stuff
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// Some handy methods for displaying debug output and for
// tracking entry and exit from functions.
// 
/////////////////////////////////////////////////////////////////

static FILE* pTraceFile = 0 ;

// Defined below
void PrintDebugSimple(char const* pStr) ;

#ifdef DEBUG_CALLS

// Initialize our nest level to zero
int	CTDebugEnterMethod::m_nCurrentNestLevel=0;

// Initialize the current method to "No Method"
//const char*	CTDebugEnterMethod::m_pCurrentMethodName = "No Method" ;

// Decide whether method names are printed when they are entered/exited
bool CTDebugEnterMethod::m_bPrintOnEnterMethod = false ;
bool CTDebugEnterMethod::m_bPrintOnExitMethod  = false ;

// Decide whether output will be indented with each function call
bool CTDebugEnterMethod::m_bIndentOutput = true ;

// Decide whether output will show previous stack trace
bool CTDebugEnterMethod::m_bShowStackTrace = true ;

// The stack of method names
StackTrace CTDebugEnterMethod::m_stackTrace ;

// Pointer into the stack showing the last item printed
long CTDebugEnterMethod::m_stackTraceDisplayPos = 0 ;

// Static method
int CTDebugEnterMethod::GetCurrentNestLevel()
{
	return CTDebugEnterMethod::m_nCurrentNestLevel;
}

#endif	// DEBUG_CALLS

// Note: This may be Windows specific way of handling
// variable args--there are other methods.
void sml::PrintDebugFormat(char const* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);

	char szBuffer[10000];

	int nBuf = VSNPRINTF(szBuffer, sizeof(szBuffer), pFormat, args);

	// was there an error? was the expanded string too long?
	if (nBuf < 0)
	{
		strcpy(szBuffer, "** Debug message too long for PrintDebugFormat's buffer **") ;
	}

#ifdef DEBUG_CALLS
	CTDebugEnterMethod::PrintStackTrace() ;

	PrintDebugMethod(CTDebugEnterMethod::GetCurrentMethodName(), szBuffer) ;
#else
	PrintDebugSimple(szBuffer) ;
#endif

	va_end(args);
}

void sml::PrintDebug(char const* pStr)
{
#ifdef DEBUG_CALLS
	CTDebugEnterMethod::PrintStackTrace() ;

	PrintDebugMethod(CTDebugEnterMethod::GetCurrentMethodName(), pStr) ;
#else
	PrintDebugSimple(pStr) ;
#endif
}

#ifdef _WIN32
void sml::PrintDebugMethod(char const* pMethodName, char const* pStr)
{
	// We want PrintDebug to be able to output to the test
	// application as well as the debug stream.
	// BADBAD: I'm not sure how we should pass an "error handler"
	// into these functions in a general way for Unix and Windows.
	// For now I'll go with the assumption that this function is
	// called soon enough that the active window is the frame of the
	// test app.  This will be true almost always (unless break at
	// the first line in the debugger for instance).
	/*
	static HWND s_OutputWnd = NULL ;

	if (s_OutputWnd == NULL)
		s_OutputWnd = ::GetActiveWindow() ;

	if (s_OutputWnd)
	{
		// Text that is our string indent
		char szIndent[1024];
		szIndent[0]='\0';

		// Add the debug string
		TCHAR textDebugString[1024] ;
		wsprintf(textDebugString, "%s%s %s\r\n", szIndent, pMethodName, pStr) ;

		// Ask the view to display this string
		// Note: Must use SendMessage or text will go out of scope.
		// Also s_OutputWnd must be in the same process as us.
		SendMessage(s_OutputWnd, WM_USER+1, (WPARAM)textDebugString, NULL) ;
	}
	*/

	// We add a newline which may be O/S specific.
	OutputDebugString(pMethodName) ;
	OutputDebugString(" ") ;
	OutputDebugString(pStr) ;
	OutputDebugString("\n") ;
}

void PrintDebugSimple(char const* pStr)
{
#ifdef _DEBUG
	if (!pTraceFile)
	{
		pTraceFile = fopen("smltrace.txt", "w") ;
	}

	if (pTraceFile)
	{
		// Dump to trace file and flush immediately (so if we crash we'll be up to date)
		fprintf(pTraceFile, "%s\n", pStr) ;
		fflush(pTraceFile) ;
	}
#endif

	fprintf(stderr, pStr) ;
	fprintf(stderr, "\n") ;
	fflush(stderr) ;
//	OutputDebugString(pStr) ;
//	OutputDebugString("\n") ;
}

#else	// _WINDOWS
// On Linux, dump to stderr (the console)
void sml::PrintDebugMethod(char const* pMethodName, char const* pStr)
{
	fprintf(stderr, "%s", pMethodName) ;
	fprintf(stderr, "%s", pStr) ;
	fprintf(stderr, "\n") ;
}

void PrintDebugSimple(char const* pStr)
{
	unused(pTraceFile); // quells gcc warning when _DEBUG isn't defined
	fprintf(stderr, "%s", pStr) ;
	fprintf(stderr, "\n") ;
}

#endif	// _WINDOWS

///// End debug stuff
////////////////////////////

