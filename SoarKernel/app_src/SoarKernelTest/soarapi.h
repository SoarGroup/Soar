/* soarapi.h

   This file contains various api functions that were included in version
   8.4 of the kernel. For the purpose of testing version 8.3, they have
   been included here as well
*/

#ifndef SOARAPI_H
#define SOARAPI_H

#ifndef GSYSPARAM_H
#include "gsysparam.h"
#endif
#include "mem.h"
#include "production.h"
#include "print.h"

#include "callback.h"

#include "definitions.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

typedef struct agent_struct agent;

/*
 *    sys_random()
 *    sys_srandom()
 */

#if defined(THINK_C) || defined(__hpux) || defined(_WINDOWS) || defined(WIN32) || defined(MACINTOSH)

    #define sys_random()     rand()
    #define sys_srandom(x)   srand(x)
#else

    #define sys_random()     random()
    #define sys_srandom(x)   srandom(x)

#endif

#ifdef WIN32 
	
#define sys_sleep( seconds )    _sleep( seconds )

#else /* WIN32 */

#include <unistd.h>
#define sys_sleep( seconds )    sleep( seconds )

#endif /* !WIN32 */

#if defined (THINK_C) || defined(_WINDOWS) || defined (__SC__) || defined(WIN32) || defined(MACINTOSH)

   #define sys_gethostname(buff,len)  (1)

#else
   #define sys_gethostname(buff,len)  gethostname(buff,len)

#endif

#if defined(WIN32)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define popen(command, mode) _popen((command), (mode))
#define pclose(stream) _pclose(stream)
#endif

typedef char Bool;
typedef void * psoar_wme;
typedef void * psoar_agent;
typedef void * soar_call_data;
/*
typedef void * soar_callback_data;
typedef void * soar_callback_agent;
typedef void (*soar_callback_free_fn)(soar_callback_data);
typedef void (*soar_callback_fn)(soar_callback_agent, 
				 soar_callback_data, 
				 soar_call_data);
*/
typedef struct wme_struct wme;
typedef struct preference_struct preference;
typedef list * soar_global_callback_array[NUMBER_OF_GLOBAL_CALLBACKS];

typedef struct sapiwme_st {
  char *id;
  char *attr;
  char *value;
  long timetag;
} soarapi_wme;

typedef struct production_memory_use_struct {
  Symbol *name;
  int mem;
  struct production_memory_use_struct *next;
} production_memory_use;

typedef struct binding_structure {
  Symbol *from, *to;
} Binding;

extern int soar_agent_ids[]; 
extern soar_global_callback_array soar_global_callbacks;
//extern char * preference_name[];

/* This function is defined in soar_build_info.cpp. */
void soar_ecBuildInfo( );

#ifdef __cplusplus
extern "C"
{
#endif

void add_multi_attribute_or_change_value(char *sym, long val);

void Soar_Print (agent* thisAgent, agent * the_agent, char * str);
void Soar_Log (agent* thisAgent, agent * the_agent, char * str);
void Soar_LogAndPrint (agent* thisAgent, agent * the_agent, char * str);
Bool passes_wme_filtering(agent* thisAgent, wme *w, Bool isAdd);

int GDS_PrintCmd (/****ClientData****/ int clientData, 
                  /****Tcl_Interp****/ void * interp,
                  int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#ifdef ATTENTION_LAPSE
void print_current_attention_lapse_settings(void);
#endif

int soar_PWatch (int argc, char *argv[], soarResult *res);
int soar_Watch (int argc, char *argv[], soarResult *res);
void print_memories (int num_to_print, int to_print[]);
production_memory_use *print_memories_insert_in_list(production_memory_use *New,
						     production_memory_use *list);
void print_preference_and_source (preference *pref,
                                  Bool print_source,
                                  wme_trace_type wtt);
void print_current_learn_settings(void);
void print_current_watch_settings (void);
void print_multi_attribute_symbols(void);
int set_watch_setting ( int dest_sysparam_number, char * param, 
			char * arg, soarResult *res);
int set_watch_prod_group_setting (int  prodgroup,
				  char * prodtype, char * arg,
				  soarResult *res);

int soar_Stats ( int argc, char *argv[], soarResult *res);
int parse_system_stats (int argc, char * argv[], soarResult *res);
int parse_memory_stats (int argc, char * argv[], soarResult *res);
int parse_rete_stats ( int argc, char * argv[], soarResult *res);

int printTimingInfo ();
int soar_ExplainBacktraces ( int argc, char *argv[], soarResult *res);
int soar_Sp (int argc, char *argv[], soarResult *res);
int soar_Print (int argc, char *argv[], soarResult *res);
int soar_Excise ( int argc, char *argv[], soarResult *res);

void do_print_for_production_name (char *prod_name, Bool internal,
				   Bool print_filename, Bool full_prod) ;
void do_print_for_wme (wme *w, int depth, Bool internal);
int read_attribute_from_string (Symbol *id, char * the_lexeme, Symbol * * attr);
int read_pref_detail_from_string (char *the_lexeme,
                                  Bool *print_productions,
                                  wme_trace_type *wtt);
void read_pattern_and_get_matching_productions (list **current_pf_list, Bool show_bindings,
                                                Bool just_chunks,Bool no_chunks);
void read_rhs_pattern_and_get_matching_productions (list **current_pf_list, Bool show_bindings,
                                                    Bool just_chunks, Bool no_chunks);
list *read_pattern_and_get_matching_wmes (void);
int read_pattern_component (Symbol **dest_sym);
void neatly_print_wme_augmentation_of_id (wme *w, int indentation);
int soar_OSupportMode (int argc, char *argv[], soarResult *res);

#ifdef USE_STDARGS
void setSoarResultResult ( soarResult *res, const char *format, ...);
void appendSoarResultResult ( soarResult *res, const char *format, ...);
#else
void setSoarResultResult (va_list va_alist);
void appendSoarResultResult (va_list va_alist);
#endif

#ifdef NULL_ACTIVATION_STATS

void null_activation_stats_for_right_activation (rete_node *node);
void null_activation_stats_for_left_activation (rete_node *node);
void print_null_activation_stats ();

#else

#define null_activation_stats_for_right_activation(node) {}
#define null_activation_stats_for_left_activation(node) {}
#define print_null_activation_stats() {}

#endif

Bool funcalls_match(list *fc1, list *fc2);
Symbol *get_binding (Symbol *f, list *bindings);
void reset_old_binding_point(list **bindings, list **current_binding_point);
void free_binding_list (list *bindings);
void print_binding_list (list *bindings);
Bool symbols_are_equal_with_bindings (Symbol *s1, Symbol *s2, list **bindings);
Bool tests_are_equal_with_bindings (test t1, test test2, list **bindings);
Bool conditions_are_equal_with_bindings (condition *c1, condition *c2, list **bindings);
Bool actions_are_equal_with_bindings (action *a1, action *a2, list **bindings);
int print_production_firings (int num_requested);
int compare_firing_counts (const void * e1, const void * e2);

#endif /* SOARAPI_H */
