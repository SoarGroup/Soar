/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* soar.h */

#ifndef _SOAR_H
#define _SOAR_H

//#include "standard.h"

#ifndef tolower
/* I can't believe Sun's ctype.h doesn't have this. */
//extern int tolower(int);
#endif

#ifdef _WINDOWS
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#if !defined(WIN32)
#define UNIX
#endif /* !WIN32 */
#endif /* _WINDOWS */

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
#define extern extern "C"
#endif

/* necessary for function prototypes below */
typedef union symbol_union Symbol;

/* Uncomment the following line to debug memory usage */
/* #define DEBUG_MEMORY */

/* Comment out the following line to avoid the overhead of keeping statistics
   on memory pool usage */
/* #define MEMORY_POOL_STATS */

/* Uncomment the following line to eliminate all timing statistics.
   The "stats" command will have much shorter output as well. */
/* #define NO_TIMING_STUFF  */

/* Comment out the following line to avoid the overhead of keeping statistics
   on how much time is spent in various parts of the system.  If you
   have DETAILED_TIMING_STATS defined, you must NOT define NO_TIMING_STUFF */
/* #define DETAILED_TIMING_STATS */

/* UNcomment the following line to have Soar maintain reference counts 
   on wmes and prefs at the top level.  This can result in larger
   memory growth due to top-level objects that never get deallocated 
   because the ref counts never drop to 0.  The default for Soar v6 - v8.2
   was to maintain the ref cts.  It's possible that in your particular
   application, weird things could happen if you don't do these ref cts,
   but if you are trying to improve performance and reduce memory, it's
   worth testing your system with the top-level-ref-cts turned off.
   See comments in recmem.cpp  */
#define DO_TOP_LEVEL_REF_CTS

/* UNComment the following line to eliminate all callbacks except those
   for I/O, AFTER_DECISION_PHASE, and printing. */
/* #define NO_CALLBACKS */

/* UNcomment the following line to get some marginally useful msgs
   about creating chunk names. */
/*  #define DEBUG_CHUNK_NAMES */

/* Uncomment following line if you want to use the ANSI stdarg facility */
#define USE_STDARGS

/* UnComment the following lines if you want to use X windows rather    */
/* than standard I/O and are not using Tcl. */
/* #ifndef USE_TCL */
/* #define USE_X_DISPLAY */
/* #endif */ /* #ifndef USE_TCL */


/* --------------------------- */
/* Current Soar version number */
/* --------------------------- */

#define MAJOR_VERSION_NUMBER 8
#define MINOR_VERSION_NUMBER 6
#define MICRO_VERSION_NUMBER 1
#define GREEK_VERSION_NUMBER 0

extern char * soar_version_string;
extern char * soar_news_string;

/* REW: begin 05.05.97 */
#define OPERAND2_MODE_NAME "Operand2/Waterfall"
/* REW: end   05.05.97 */

/* --------------------------------------------------------- */
/* Line width of terminal (used for neatly formatted output) */
/* --------------------------------------------------------- */

#define COLUMNS_PER_LINE 80

/* ------------------------------ */
/* Global type declarations, etc. */
/* ------------------------------ */

/* ----------------- */
/* Goal Stack Levels */
/* ----------------- */

typedef signed short goal_stack_level;
#define TOP_GOAL_LEVEL 1
#define ATTRIBUTE_IMPASSE_LEVEL 32767
#define LOWEST_POSSIBLE_GOAL_LEVEL 32767

/* --------------------------------------------------------------------
                    Transitive Closure Numbers

   In many places, we do transitive closures or some similar process in
   which we mark identifiers and/or variables so as not to repeat them
   later.  Marking is done by setting the "tc_num" field of the symbol
   to the "current transitive closure number".  We don't have to go
   back and unmark stuff when done--we just increment the current
   transitive closure number next time.  Whenever we need to start a
   new marking, we call get_new_tc_number() (see production.cpp
   comments below).
-------------------------------------------------------------------- */

typedef char Bool;
typedef unsigned long tc_number;
typedef unsigned char byte;

/*typedef char bool;
#ifndef __cplusplus
#define bool char
#endif*/

/* Some compilers define these. */
#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#define NIL (0)

#define EOF_AS_CHAR ((char)EOF)

/* functions defined in Soar\kernel that are not prototyped in any other header */

static void memory_error_and_abort (agent* thisAgent);
static char *xmalloc (agent* thisAgent, int bytes);
static char *xrealloc (agent* thisAgent, char *pointer, int bytes);
//void print (char *format, ... );

/* functions defined in Soar\interface\ */

extern void do_print_for_identifier (Symbol *id, int depth, Bool internal);
extern void add_results_for_id (agent* thisAgent, Symbol *id);

/* Main pgm stuff -- moved here from soarkernel.h -ajc (5/3/02) */

extern void init_soar (void);
extern int  terminate_soar (void);

#ifdef __cplusplus
#undef extern
#endif

#ifdef __cplusplus
}
#endif

#endif
