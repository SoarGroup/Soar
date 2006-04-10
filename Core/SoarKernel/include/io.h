/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                                 io.h

                  General Soar I/O System Routines

   User-defined Soar I/O routines should be added at system startup time
   via calls to add_input_function() and add_output_function().  These 
   calls add things to the system's list of (1) functions to be called 
   every input cycle, and (2) symbol-to-function mappings for output
   commands.  File io.cpp contains the system I/O mechanism itself (i.e.,
   the stuff that calls the input and output functions), plus the text
   I/O routines.

   Init_soar_io() does what it say.  Do_input_cycle() and do_output_cycle()
   perform the entire input and output cycles -- these routines are called 
   once per elaboration cycle.  (once per Decision cycle in Soar 8).
   The output module is notified about WM changes via a call to
   inform_output_module_of_wm_changes().
======================================================================= */

#ifndef IO_H
#define IO_H

#include "callback.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef unsigned char byte;
typedef struct cons_struct cons;
typedef struct wme_struct wme;
typedef struct agent_struct agent;
typedef struct callback_struct soar_callback;
typedef cons list;
typedef union symbol_union Symbol;

typedef void * soar_callback_agent;
typedef void * soar_callback_data;
typedef void * soar_call_data;
  /*
    RDF 20020709: These are now defined in callback .h

    typedef void (*soar_callback_free_fn)(soar_callback_data);
    typedef void (*soar_callback_fn)(soar_callback_agent, 
    soar_callback_data, 
    soar_call_data);
  */

extern void init_soar_io (agent* thisAgent);
extern void do_input_cycle (agent* thisAgent);
extern void do_output_cycle (agent* thisAgent);

extern void inform_output_module_of_wm_changes (agent* thisAgent, 
												list *wmes_being_added,
                                                list *wmes_being_removed);

extern Symbol *get_next_io_symbol_from_text_input_line (agent* thisAgent, 
														char **text_read_position); /* in io.cpp */

/* =======================================================================
                            Input Functions
 
   Input functions take one parameter--a mode (integer) indicating why the 
   function is being called.  The mode is either TOP_STATE_JUST_CREATED, 
   NORMAL_INPUT_CYCLE, or TOP_STATE_JUST_REMOVED.  In the input cycle
   immediately following the installation of the top state, each input
   function is called once with TOP_STATE_JUST_CREATED and then once with
   NORMAL_INPUT_CYCLE.  In the input cycle immediately following the removal
   of the top state, the functions are called with TOP_STATE_JUST_REMOVED.
   If the top state is *replaced*, the functions are called with 
   TOP_STATE_JUST_REMOVED, then TOP_STATE_JUST_CREATED, and then
   NORMAL_INPUT_CYCLE.

   Input routines create, modify, and delete input structures via calls
   to add_input_wme() and remove_input_wme().  The arguments to add_input_wme()
   indicate the id/attr/value components of the wme to be added.  Each of
   these components must be either (1) the current value of the global
   variable "top_state", or (2) the returned value from a call to 
   get_new_io_identifier(), get_io_sym_constant(), get_io_int_constant(),
   or get_io_float_constant().  [The idea behind creating the components this
   way is to avoid having I/O functions deal with the reference counts on
   symbols.]  For every call an I/O function makes to get_xxx(), it should
   later call release_io_symbol().  Release_io_symbol() should *not* be
   called with the value of "top_state"--*only* the components obtained via
   get_xxx().
   
   The add_input_wme() routine returns a pointer to the wme added.  The input
   routine shouldn't use this pointer in any way except to save it around for
   a later call to remove_input_wme().  Example:

         float current_sensor_value;
         wme *w;
         Symbol *s1,*s2;
         ... insert code to read value into current_sensor_value here ...
         s1 = get_io_sym_constant ("sensor-value");
         s2 = get_io_float_constant (current_sensor_value);
         ... add to working memory (S1 ^sensor-value 37.5) ...
         w = add_input_wme (top_state, s1, s2);
         release_io_symbol (s1);
         release_io_symbol (s2);
   
   On some later call, the input function might call remove_input_wme (w)
   to remove (S1 ^sensor-value 37.5) from working memory.

   To remove an entire input structure, it is sufficient for the input
   function to call remove_input_wme() on just the top link wme.  The input
   function need not call remove_input_wme() on each and every wme in the
   structure.  (Soar automagically garbage collects all the wmes in the
   now-disconnected structure.)  Note that when an input function is called
   with TOP_STATE_JUST_REMOVED, all existing input structures have already
   been garbage collected (since the top state no longer exists), so the 
   input function should never call remove_input_wme() when mode is
   TOP_STATE_JUST_REMOVED.  Remove_input_wme() normally returns TRUE,
   indicating success.  It returns FALSE if an error occurs (e.g., if the
   wme argument isn't in WM).
======================================================================= */

#define TOP_STATE_JUST_CREATED 1
#define NORMAL_INPUT_CYCLE 2
#define TOP_STATE_JUST_REMOVED 3

extern Symbol *get_new_io_identifier (agent* thisAgent, char first_letter);
extern Symbol *get_io_sym_constant (agent* thisAgent, char *name);
extern Symbol *get_io_int_constant (agent* thisAgent, long value);
extern Symbol *get_io_float_constant (agent* thisAgent, float value);
extern void release_io_symbol (agent* thisAgent, Symbol *sym);

extern wme *add_input_wme (agent* thisAgent, Symbol *id, Symbol *attr, Symbol *value);
extern Bool remove_input_wme (agent* thisAgent, wme *w);

/* =======================================================================
                            Output Functions
 
   Output functions take two parameters--a mode (integer) indicating why the 
   function is being called, and a pointer to a chain of io_wme structures.
   The mode is either ADDED_OUTPUT_COMMAND (used when an output link is first
   created), MODIFIED_OUTPUT_COMMAND (used when the transitive closure of an
   existing link changes), or REMOVED_OUTPUT_COMMAND (used when the output
   link is removed from working memory).

   The chain of io_wme structures is connected via the "next" fields in the
   structures; for the last io_wme, next==NIL.  When mode is either
   ADDED_OUTPUT_COMMAND or MODIFIED_OUTPUT_COMMAND, this chain contains
   all the wmes in the current transitive closure of the output link
   (including the output link wme itself).  When mode is
   REMOVED_OUTPUT_COMMAND, the chain consists of just one io_wme--the top-level
   ouput link being removed.
  
   Output functions should inspect the io_wme chain and take whatever
   actions are appropriate.  Note that Soar deallocates the io_wme chain
   after calling the output function, so the output function is responsible
   for saving any necessary information around for later.
   
   How can an output function examine the io_wme's?  The io_wme structures
   indicate the id/attr/value of the wmes in the output structure.  See
   the comments above for symtab.cpp for an explanation of the structure
   of these symbols.

   Get_output_value() is a simple utility routine for finding things in
   an io_wme chain.  It takes "outputs" (the io_wme chain), and "id" and
   "attr" (symbols to match against the wmes), and returns the value from
   the first wme in the chain with a matching id and attribute.  Either
   "id" or "attr" (or both) can be specified as "don't care" by giving
   NULL (0) pointers for them instead of pointers to symbols.  If no matching
   wme is found, the function returns a NULL pointer.
======================================================================= */

typedef char Bool;

typedef struct io_wme_struct {
  struct io_wme_struct *next;  /* points to next io_wme in the chain */
  Symbol *id;                  /* id, attribute, and value of the wme */
  Symbol *attr;
  Symbol *value;
} io_wme;

typedef struct replay_struct {
  struct replay_struct *next, *prev;
  unsigned long orig_timetag;
  char  data_string[1024];
} replay;
  
typedef struct output_link_struct {
  struct output_link_struct *next, *prev;  /* dll of all existing links */
  byte status;                             /* current xxx_OL_STATUS */
  wme *link_wme;                           /* points to the output link wme */
  list *ids_in_tc;                         /* ids in TC(link) */
  soar_callback *cb;                       /* corresponding output function */
} output_link;


#define ADDED_OUTPUT_COMMAND 1
#define MODIFIED_OUTPUT_COMMAND 2
#define REMOVED_OUTPUT_COMMAND 3

typedef struct output_call_info_struct {
  int mode;
  io_wme * outputs;
} output_call_info;

extern Symbol *get_output_value (io_wme *outputs, Symbol *id, Symbol *attr);

extern void add_input_function (agent * a, soar_callback_fn f, 
				soar_callback_data cb_data, 
				soar_callback_free_fn free_fn,
				char * name);
extern void remove_input_function (agent * a, char * name);
extern void add_output_function (agent * a, soar_callback_fn f, 
				 soar_callback_data cb_data, 
				 soar_callback_free_fn free_fn,
				 char * output_link_name);
extern void remove_output_function (agent * a, char * name);

#ifdef __cplusplus
}
#endif

#endif
