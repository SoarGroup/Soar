#ifndef PORTABILITY_H
#define PORTABILITY_H

/* This file will contain code required on all platforms */

#if defined(_WIN32) || defined(_WINDOWS) || defined(__WIN32__)

/* This file contains code specific to the windows platforms */
#include "portability_windows.h"

#else // not ( _WIN32 || _WINDOWS )

/* This file contains code specific to the posix platforms */
#include "portability_posix.h"

#endif // not ( _WIN32 || _WINDOWS )

#endif // PORTABILITY_H
