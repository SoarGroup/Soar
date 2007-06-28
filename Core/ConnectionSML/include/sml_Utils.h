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

void soar_sleep(long secs, long msecs);

#endif // SML_UTILS_H