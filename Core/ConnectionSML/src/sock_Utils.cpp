#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

// Utils.cpp: implementation of the Utils functions
//
//////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <string.h>
#include "sock_Utils.h"

using namespace sock ;

//////////////////////////////////////////////////////////////////////
// String utilties
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Function name  : SafeStrncpy
// 
// Return type    : char* 				// Pointer to pDest returned
// Argument       : char* pDest			// Place to copy chars to
// Argument       : char const* pSrc	// Place to copy chars from
// Argument       : size_t count		// Max chars available in pDest
// 
// Description	  : 
// Same as strncpy but ensures pDest is null terminated.
// Count therefore includes the null -- so count-1 is the max chars copied
//
/////////////////////////////////////////////////////////////////////
char* SafeStrncpy(char* pDest, char const* pSrc, size_t count)
{
	// Check for passing NULL in as the src
	if (!pSrc)
	{
		pDest[0] = '\0' ;
		return pDest ;
	}

	// Use standard strncpy to copy the string
	strncpy(pDest, pSrc, count);

	// Make sure that we end the string with a NULL
	pDest[count-1] = '\0';

	return pDest ;
}
