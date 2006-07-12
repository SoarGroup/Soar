/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  soarkernel.h
 *
 * =======================================================================
 *
 *                         Soar 6 Include File
 *
 * This file gets #included in all Soar 6 source files.  It defines all
 * sorts of constants and data structures, and gives prototype declarations
 * for various functions.
 * It also has insightful comments and detailed explanations and is
 * recommended reading for anyone who would like to know more about
 * the source code.
 * =======================================================================
 */

#ifndef _SOAR_H_INCLUDED
#define _SOAR_H_INCLUDED

/* =====================================================================*/

#include "kernel.h"

/* =====================================================================*/

/* -------------------------------------------------- */
/*              Names of Rete Structures              */
/* (only pointers to these are used outside the rete) */
/* -------------------------------------------------- */

#ifdef __cplusplus
extern "C"
{
#endif

struct token_struct;
struct rete_node_struct;

/* REW: begin 08.20.97 */
/* The ms_change_struct is exported to the entire system
   (for better or worse) so
   this restricted definition is no longer necessary. */
/* struct ms_change_struct; */
/* REW: end 08.20.97 */

struct node_varnames_struct;

#ifdef __cplusplus
}
#endif

/* ======================================================================*/

#include "mem.h"
#include "lexer.h"
#include "symtab.h"
#include "gdatastructs.h"
#include "rhsfun.h"
#include "instantiations.h"
#include "production.h"
#include "gsysparam.h"
#include "init_soar.h"
#include "wmem.h"
#include "tempmem.h"
#include "decide.h"
#include "consistency.h"
#include "parser.h"
#include "print.h"
#include "reorder.h"
#include "recmem.h"
#include "backtrace.h"
#include "chunk.h"
#include "osupport.h"
#include "rete.h"
#include "trace.h"
#include "callback.h"
#include "io.h"
#include "reinforcement_learning.h"

/* ======================================================================= */

#ifdef __cplusplus
extern "C"
{
#endif

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

#ifdef __cplusplus
}
#endif

/*=====================================================================*/

#include "explain.h"
#include "agent.h"
                              
/*=====================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

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

/* This is deprecated. -AJC (8/9/02) */
//extern char * c_interrupt_msg;

#ifdef __cplusplus
}
#endif

#endif /* _SOAR_H_INCLUDED */
