/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* kernel.h */

#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* When the Soar Kernel was moved to a C++ compiler, the 
 * C precompiler macros were converted to inline functions
 * The macros still work, but can make it confusing to step
 * through a debugger during code execution.  However it's
 * possible that some compilers on some operating systems
 * would not inline the code properly, making Soar slower
 * and consuming more memory.  To use the precompiler macros
 * instead of the inline functions, define the variable below.
 */
//#define USE_MACROS

/* -------------------------------------------------- */
/*              Names of Rete Structures              */
/* (only pointers to these are used outside the rete) */
/* -------------------------------------------------- */

struct token_struct;
struct rete_node_struct;

/* REW: begin 08.20.97 */
/* The ms_change_struct is exported to the entire system
   (for better or worse) so
   this restricted definition is no longer necessary. */
/* struct ms_change_struct; */
/* REW: end 08.20.97 */

/* -------------------------------------------------------------------- */
/*                                                                      */
/* Macros for handling multi-agent switching in Soar.                   */

//#ifdef USE_X_DISPLAY
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//
//typedef struct x_info_struct {
//  Window  parent_window;
//  Window  window;
//  GC      gc;
//  XFontStruct * font_struct;
//  int     char_height;
//  char    input_buffer[2000];
//  int     input_buffer_index;
//  int     window_x;
//  int     window_y;
//  int     width;
//  int     height;
//  unsigned long foreground;
//  unsigned long background;
//  int     borderwidth;
//  Symbol * last_op_id; /* Used in monitors */
//} x_info;
//#endif


/* These functions define the protocol functions for the X interface to  */
/* Soar.                                                                 */

//#ifdef USE_X_DISPLAY
//  extern char *  x_input_buffer;
//  extern int     x_input_buffer_index;
//  extern Bool    waiting_for_command;
//  extern void create_agent_window (agent *, char * agent_class);
//  extern void create_monitor_window (agent *, char * command);
//  extern void handle_soar_x_events (void);
//  extern void refresh_monitor_window (agent *);
//  extern void destroy_soar_display (void);
//
//  extern Bool text_io_mode;
//
//#ifdef USE_STDARGS
//  extern void print_x_format_string (x_info * window, char *format, ... );
//#else
//  extern void print_x_format_string ();
//#endif
//
//#endif  /* USE_X_DISPLAY */
//
/* This is deprecated. -AJC (8/9/02) */
//extern char * c_interrupt_msg;


#ifdef __cplusplus
#define extern extern "C"
#endif

/* necessary for function prototypes below */
typedef union symbol_union Symbol;

/* Uncomment the following line to debug memory usage */
/* #define DEBUG_MEMORY */

/* Comment out the following line to avoid the overhead of keeping statistics
   on memory pool usage */
//#define MEMORY_POOL_STATS

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
   because the ref counts never drop to 0.  The default for Soar v6 - v8.6.1
   was to maintain the ref cts.  It's possible that in your particular
   application, weird things could happen if you don't do these ref cts,
   but if you are trying to improve performance and reduce memory, it's
   worth testing your system with the top-level-ref-cts turned off.
   Soar will be much more efficient.  See comments in recmem.cpp  */
//#define DO_TOP_LEVEL_REF_CTS

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

/* UnComment the following line to enable numeric indifference preferences */
#define NUMERIC_INDIFFERENCE

#ifdef NUMERIC_INDIFFERENCE
/* Possible modes for numeric indifference */
enum ni_mode {
    NUMERIC_INDIFFERENT_MODE_AVG,
    NUMERIC_INDIFFERENT_MODE_SUM,
};
#endif 

/* Comment the following line to disable workaround for bug 139 */
#define BUG_139_WORKAROUND

/* UnComment the following to print a warning whenever we 
   are ignoring a situation when there's no instance to retract for a justification */
/*#define BUG_139_WORKAROUND_WARNING*/

/* UnComment the following to enable Soar to deal with certain productions in a more 
   intuitive manner.  In particular, productions that modify a wme value by reject 
   its current value and asserting its new value need to ensure that the current and
   new values differ.  This option may add a small run time cost, since two loops are made
   through the preferences list. */
#define O_REJECTS_FIRST

/**
 *  \def AGRESSIVE_ONC
 * 
 *       Setting this option enables Soar to generate operator no changes
 *       without requring a dedicated decision cycle.  This occurs when
 *       Soar recognizes that no productions will fire the the PE phase.
 *       This is the standard behavior in Soar 8.3.5 and Soar 8.4.5, but 
 *       is no longer standard as of Soar 8.5
 */
//#define AGRESSIVE_ONC


#define SOAR_WMEM_ACTIVATION    // compile switch
#define EPISODIC_MEMORY         // compile switch
    
    
/* --------------------------- */
/* Current Soar version number */
/* --------------------------- */

#define MAJOR_VERSION_NUMBER 8
#define MINOR_VERSION_NUMBER 6
#define MICRO_VERSION_NUMBER 2
#define GREEK_VERSION_NUMBER 0

#ifdef _MSC_VER
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

#define BUFFER_MSG_SIZE 128

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
typedef struct agent_struct agent;

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

/*
static char *xmalloc (agent* thisAgent, int bytes);
static char *xrealloc (agent* thisAgent, char *pointer, int bytes);
void print (char *format, ... );
*/

/*void Soar_LogAndPrint (agent* thisAgent, agent * the_agent, char * str);*/

/* This is defined in the Soar interface. */
/*extern void do_print_for_identifier (Symbol *id, int depth, Bool internal);*/

#ifdef __cplusplus
#undef extern
#endif

#ifdef __cplusplus
}
#endif

#endif

