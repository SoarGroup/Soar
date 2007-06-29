#ifndef SML_UTILS_H
#define SML_UTILS_H

/////////////////////////////////////////////////////////////////
// Utility header
//
// Author: Jonathan Voigt, Bob Marinier
// Date  : June 2007
//
// This header collects some useful code used throughout Soar.
//
/////////////////////////////////////////////////////////////////

// Silences unreferenced formal parameter warning
#define unused(x) (void)(x)

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
void soar_sleep(long secs, long msecs);

/////////////////////////////////////////////////////////////////////
// Function name  : ReportSystemErrorMessage
// 
// Return type    : void 	
// 
// Description	  : Get the text of the most recent system error
//
/////////////////////////////////////////////////////////////////////
void ReportSystemErrorMessage();

#endif // SML_UTILS_H