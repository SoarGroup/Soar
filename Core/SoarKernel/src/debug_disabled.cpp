/*
 * debug_disabled.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: mazzin
 */


#include <portability.h>
#include "kernel.h"
#include "debug.h"

#ifndef SOAR_DEBUG_UTILITIES

//#include "soar_db.h"

/* -- Empty functions that should get optimized away in the release build -- */
extern void dprint(TraceMode mode, const char* format, ...) {}
extern void dprint_noprefix(TraceMode mode, const char* format, ...) {}

extern void debug_test(int type) {}

#endif
