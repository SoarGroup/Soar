#ifndef PORTABILITY_POSIX_H
#define PORTABILITY_POSIX_H

/* This file contains code specific to the posix platforms */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <signal.h>

#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

#include <assert.h>
#include <time.h>

#include <regex.h>

/* this needs to be defined */
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024   /* AGR 536  - from sys/param.h */
#endif // MAXPATHLEN

/* necessary in connectionsml socket code */
#define ENABLE_LOCAL_SOCKETS

// from gski, remove when things compile
//#ifdef WIN32
//#include <direct.h>
//#define GetCwd _getcwd
//#else
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#define GetCwd getcwd
//#endif

#endif // PORTABILITY_POSIX_H

