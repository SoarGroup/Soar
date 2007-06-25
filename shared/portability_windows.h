#ifndef PORTABILITY_WINDOWS_H
#define PORTABILITY_WINDOWS_H

/* This file contains code specific to the windows platforms */

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
//#include <stdlib.h>
#include <winsock2.h>
#include <direct.h>
#include <time.h>
#include <assert.h>
#include <Lmcons.h> // for UNLEN constant

// Visual Studio 2005 requires these:
#define getcwd _getcwd
#define chdir _chdir
#define strcasecmp _stricmp

#include <ctype.h>
#include <math.h>
#include <signal.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024   /* AGR 536  - from sys/param.h */
#endif

// Map certain functions depending on the OS
#define STRICMP	   stricmp
#define VSNSPRINTF _vsnprintf
#define ENABLE_NAMED_PIPES

#endif // PORTABILITY_WINDOWS_H