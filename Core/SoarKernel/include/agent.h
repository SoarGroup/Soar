/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  agent.h
 *
 * =======================================================================
 *  Initialization for the agent structure.  Also the cleanup routine
 *  when an agent is destroyed.  These routines are usually replaced
 *  by the same-named routines in the Tcl interface file soarAgent.c
 *  The versions in this file are used only when not linking in Tcl.
 *  HOWEVER, this code should be maintained, and the agent structure
 *  must be kept up to date.
 * =======================================================================
 */

#ifndef AGENT_H
#define AGENT_H

#ifndef GSYSPARAMS_H
#include"gsysparam.h"
#endif

#include "kernel.h" 
#include "init_soar.h"
#include "mem.h"
#include "lexer.h"
#include "chunk.h"
#include "callback.h"

/* JC ADDED: Included to allow gski callbacks */
#include "gski_event_system_data.h"

/* JC ADDED: Included so we can put the RHS functions in here */
typedef struct rhs_function_struct rhs_function;

#ifdef __cplusplus
extern "C"
{
#endif

/* RBD Need more comments here, or should this stuff be here at all? */

#define UPDATE_LINKS_NORMALLY 0
#define UPDATE_DISCONNECTED_IDS_LIST 1
#define JUST_UPDATE_COUNT 2

typedef char Bool;
typedef union symbol_union Symbol;
typedef struct hash_table_struct hash_table;
typedef struct wme_struct wme;
typedef struct memory_pool_struct memory_pool;
typedef struct lexer_source_file_struct lexer_source_file;
typedef struct production_struct production;
typedef struct preference_struct preference;
typedef struct pi_struct parent_inst;
typedef struct preference_struct preference;
typedef struct backtrace_struct backtrace_str;
typedef struct explain_chunk_struct explain_chunk_str;
typedef struct io_wme_struct io_wme;
typedef struct multi_attributes_struct multi_attribute;
typedef struct replay_struct replay;
typedef struct kernel_struct Kernel;

// following def's moved here from old interface.h file  KJC nov 05
/* AGR 568 begin */
typedef struct expansion_node {
  struct lexeme_info lexeme;
  struct expansion_node *next;
} expansion_node;

typedef struct alias_struct {
  char *alias;
  struct expansion_node *expansion;
  struct alias_struct *next;
} alias_struct;

typedef struct dir_stack_struct {
  char *directory;
  struct dir_stack_struct *next;
} dir_stack_struct;
/* AGR 568 end */
 

/* This typedef makes soar_callback_array equivalent to an array of list
   pointers. Since it was used only one time, it has been commented out
   here and spelled out completely in that particular instance -ajc
   (5/3/02)
*/
//typedef list * soar_callback_array[NUMBER_OF_CALLBACKS];

typedef signed short goal_stack_level;

/* AGR 564 begins */

/* !!!!!!!!!!!!!!!!  here's the agent structure !!!!!!!!!!!!!!!!!!!!!!!!*/
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Agent structure used to hold what were previously global variables  */
/*  in the single-agent Soar.                                           */
/*                                                                      */
/*----------------------------------------------------------------------*/

/* WARNING!! If you add a new global into the Soar C code, be
   sure to use the current_agent macro to ensure compatibility
   with the multi-agent code!  E.g. if your new global is "foo"
   then do NOT refer to it in the code as "foo" but instead as
   "current_agent(foo)". 
   
   As of version 8.6, the current_agent macro was deprecated
   when gSKI was added as a wrapper.  Use ptr directly, thisAgent->foo. */


/* If you define a new global, initialize it in the create_soar_agent
   routine.  AGR 527c 3-May-94 */

typedef struct alpha_mem_struct alpha_mem;
typedef struct token_struct token;
typedef char * test;

typedef struct agent_struct {
  /* After v8.6.1, all conditional compilations were removed
   * from struct definitions, including the agent struct below
   */

  /* ----------------------- Rete stuff -------------------------- */
  /* 
   * These are used for statistics in rete.cpp.  They were originally
   * global variables, but in the deglobalization effort, they were moved
   * to the (this) agent structure.
   */
  unsigned long actual[256], if_no_merging[256], if_no_sharing[256];

  unsigned long current_retesave_amindex;
  unsigned long reteload_num_ams;
  alpha_mem **reteload_am_table;

  unsigned long current_retesave_symindex;
  unsigned long reteload_num_syms;
  Symbol **reteload_symbol_table;

  token *dummy_matches_node_tokens;

  long highest_rhs_unboundvar_index;

  //
  // Moved here from parser.cpp.  This is used to create temporary unique
  // identifiers for parsing.  This should probably be localized to the
  // production, but for now, this is much better than having it as a 
  // global.
  //
  unsigned long placeholder_counter[26];

  //
  // Used to be a global,  this has been moved here from recmem.cpp
  //
  long firer_highest_rhs_unboundvar_index;

  //
  // This was taked from reorder.cpp, but it is also used in production.cpp
  //
  char *name_of_production_being_reordered;

  //
  // These was taked from print.
  Symbol *action_id_to_match;
  test id_test_to_match;

  //
  // This was taken from production.h
  //
  tc_number current_tc_number;

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  /* Hash tables for alpha memories, and for entries in left & right memories */
  void              * left_ht;
  void              * right_ht;
  hash_table        *(alpha_hash_tables[16]);
  
  /* Number of WMEs, and list of WMEs, the Rete has been told about */
  unsigned long       num_wmes_in_rete;
  wme               * all_wmes_in_rete;
  
  /* Memory pools */
  memory_pool         rete_node_pool;
  memory_pool         rete_test_pool;
  memory_pool         right_mem_pool;
  memory_pool         token_pool;
  memory_pool         alpha_mem_pool;
  memory_pool         ms_change_pool;
  memory_pool         node_varnames_pool;
  
  /* Dummy nodes and tokens */
  struct rete_node_struct * dummy_top_node;
  struct token_struct * dummy_top_token;

  /* Various Rete statistics counters */
  unsigned long       rete_node_counts[256];
  unsigned long       rete_node_counts_if_no_sharing[256];
  unsigned long       token_additions;
  unsigned long       token_additions_without_sharing;
  unsigned long       num_right_activations;
  unsigned long       num_left_activations;
  unsigned long       num_null_right_activations;
  unsigned long       num_null_left_activations;
  
  
  /* Miscellaneous other stuff */
  unsigned long       alpha_mem_id_counter; /* node id's for hashing */
  unsigned long       beta_node_id_counter;
  struct ms_change_struct * ms_assertions;  /* changes to match set */
  struct ms_change_struct * ms_retractions;

  /* ----------------------- Lexer stuff -------------------------- */
  
  lexer_source_file * current_file; /* file we're currently reading */
  char                current_char; /* holds current input character */
  struct lexeme_info  lexeme;       /* holds current lexeme */
  Bool                print_prompt_flag;
  
  /* ---------------- Predefined Symbols -------------------------
     Certain symbols are used so frequently that we create them at
     system startup time and never deallocate them.  
     ------------------------------------------------------------- */

  Symbol            * attribute_symbol;
  Symbol            * choices_symbol;
  Symbol            * conflict_symbol;
  Symbol            * constraint_failure_symbol;
  Symbol            * goal_symbol;
  Symbol            * impasse_symbol;
  Symbol            * io_symbol;
  Symbol            * item_symbol;
  Symbol            * multiple_symbol;
  Symbol            * name_symbol;
  Symbol            * nil_symbol;
  Symbol            * no_change_symbol;
  Symbol            * none_symbol;
  Symbol            * o_context_variable;
  Symbol            * object_symbol;
  Symbol            * operator_symbol;
  Symbol            * problem_space_symbol;
  Symbol            * quiescence_symbol;
  Symbol            * s_context_variable;
  Symbol            * so_context_variable;
  Symbol            * ss_context_variable;
  Symbol            * sso_context_variable;
  Symbol            * sss_context_variable;
  Symbol            * state_symbol;
  Symbol            * superstate_symbol;
  Symbol            * t_symbol;
  Symbol            * tie_symbol;
  Symbol            * to_context_variable;
  Symbol            * ts_context_variable;
  Symbol            * type_symbol;
  Symbol            * wait_symbol;   /* REW:  10.24.97 */

  /* RPM 9/06 begin */
  Symbol			* input_link_symbol;
  Symbol			* output_link_symbol;
  /* RPM 9/06 end */
  
  /* ----------------------- Symbol table stuff -------------------------- */

  unsigned long       current_symbol_hash_id;
  unsigned long       id_counter[26]; 
  
  struct hash_table_struct * float_constant_hash_table;
  struct hash_table_struct * identifier_hash_table;
  struct hash_table_struct * int_constant_hash_table;
  struct hash_table_struct * sym_constant_hash_table;
  struct hash_table_struct * variable_hash_table;
  
  memory_pool         float_constant_pool;
  memory_pool         identifier_pool;
  memory_pool         int_constant_pool;
  memory_pool         sym_constant_pool;
  memory_pool         variable_pool;
  
  /* ----------------------- Top-level stuff -------------------------- */

  /* --- headers of dll's of all productions of each type --- */
  production        * all_productions_of_type[NUM_PRODUCTION_TYPES];
  /* --- counts of how many productions there are of each type --- */
  unsigned long       num_productions_of_type[NUM_PRODUCTION_TYPES];
  
  /* --- lists of symbols (PS names) declared chunk-free and chunky --- */
  list              * chunk_free_problem_spaces;
  list              * chunky_problem_spaces;   /* AGR MVL1 */
  
  /* --- default depth for "print" command --- */
  int                 default_wme_depth;      /* AGR 646 */
  
  /* --- stuff for "input-period" command --- */
  /* --- in Soar8, input runs once at beginning of D cycle, no matter what */
  int                 input_period;      /* AGR REW1 */
  Bool                input_cycle_flag;  /* AGR REW1 */
  
  /* --- current top level phase --- */
  enum top_level_phase current_phase; 
  
  /* --- to interrupt at the end of the current phase, set stop_soar to TRUE
     and reason_for_stopping to some appropriate string --- */
  Bool                stop_soar;
  char              * reason_for_stopping;
  
  /* --- the RHS action (halt) sets this TRUE --- */
  Bool                system_halted;

  /* --- stuff for max-chunks (which is a sysparam) --- */
  unsigned long       chunks_this_d_cycle; /* # chunks built this DC */
  Bool		    max_chunks_reached;
  
  /* --- list of productions whose firings are being traced --- */
  list              * productions_being_traced; 
  
  /* --- various user-settable system parameters --- */
  long                sysparams[HIGHEST_SYSPARAM_NUMBER+1];
  
  /* --- parameters for running Soar --- */
  /*  --- the code loops go_number times over the go_type phases --- */
  long                go_number;     /* How many times to "go" */
  Symbol            * go_slot_attr;  /* The context slot checked */
  goal_stack_level    go_slot_level; /* The goal stack level checked */
  enum go_type_enum   go_type;       /* The phase type used */
  
  /* --- Top-level Statistics --- */
  
  /* running total of WM sizes at end of phases */
  double              cumulative_wm_size;
  /* number of items included in "cumulative_wm_size" sum */
  unsigned long       num_wm_sizes_accumulated; 
  
  unsigned long       max_wm_size;    /* maximum size of WM so far */
  unsigned long       wme_addition_count; /* # of wmes added to WM */
  unsigned long       wme_removal_count;  /* # of wmes removed from WM */
  unsigned long       d_cycle_count;          /* # of DC's run so far */
  unsigned long       e_cycle_count;          /* # of EC's run so far */
  /*  in Soar 8, e_cycles_this_d_cycle is reset to zero for every
      propose and apply phase */
  unsigned long       e_cycles_this_d_cycle;  /* # of EC's run this DC */
  unsigned long       num_existing_wmes;      /* current WM size */
  unsigned long       production_firing_count;  /* # of prod. firings */
  unsigned long       d_cycle_last_output;    /* last time agent produced output */  //KJC 11.17.05
  unsigned long       decision_phases_count;  /* can differ from d_cycle_count.  want for stats */
  //?? unsigned long       out_cycle_count;       /* # of output phases have gen'd output */
  //?? unsigned long       phase_count;       /* # of phases run so far */

  /* REW: begin 09.15.96 */
/* in Soar 8, PE's are done only during the APPLY phase */
  unsigned long       pe_cycle_count;          /* # of PE's run so far */
  unsigned long       pe_cycles_this_d_cycle;  /* # of PE's run this DC */

  parent_inst *parent_list_head;
/* REW: end   09.15.96 */
  
  
  /* ----------------------- Timing statistics -------------------------- */

/* 
For Soar 7, the timing code has been completely revamped.  When the compile
flag NO_TIMING_STUFF is not set, statistics will be now be collected on the
total cpu time, total kernel time, time spent in the individual phases of a
decision cycle, time spent executing the input and output functions, and time
spent executing callbacks (or monitors).  When the DETAILED_TIMING_STATS flag
is set, additional statistics will be collected for ownership, match, and
chunking computations according to the phase in which they occur. (Notice
that DETAILED_TIMING_STATS can only be collected when NO_TIMING_STUFF is not
true.)

The total_cpu_time is turned on when one of the run_<x> functions is
initiated.  This timer is not turned off while the do_one_top_level_phase()
function is executing.  The total_kernel_time timer is turned on just after
the total_cpu_time timer and turned off just before the other is turned off.
This guarantees that the total kernel time -- including the time it takes to
turn on and off the kernel timer -- is a part of the total cpu time.  The
total_kernel_time is also turned off whenever a callback is initiated or when
the input and output functions are executing.

The decision_cycle_phase_timers measure the kernel time for each phase of the
decision cycle (ie, INPUT_PHASE, PREFERENCE_PHASE, WM_PHASE, OUTPUT_PHASE,
and DECISION_PHASE).  Each is turned on at the beginning of its corresponding
phase in do_one_top_level_phase and turned off at the end of that phase.
These timers are also turned off for callbacks and during the execution of
the input and output functions.

The monitors_cpu_time timers are also indexed by the current phase.  Whenever
a callback is initiated, both the total_kernel_time and
decision_cycle_phase_timer for the current phase are turned off and the
monitors_cpu_time turned on.  After the callback has terminated, the kernel
timers are turned back on.  Notice that the same relationship holds here as
it did between the total_cpu_time and total_kernel_time timers.  The
total_kernel_time is always turned off last and turned on first, in
comparison to the decision_cycle_phase_timer.  This means that turning the
decision_cycle_phase_timers on and off is included as part of the kernel time
and helps ensure that the total_kernel_time is always greater than the sum of
the decision_cycle_timers.

The input_function_cpu_time and output_function_cpu_time timers measure the
time it takes to execute the input and output functions respectively.  Both
the total_kernel_time and decision_cycle_phase_timers are turned off when
these timers are turned on (with the same ordering as discussed previously).
The input function is a little tricky.  Because add-wme can be called by the
input routine, which then calls do_buffered_wm_and_ownership_changes, we
can't just turn off the kernel timers for input and expect to get numbers for
both match_time (see next para) and kernel time.  The solution implemented in
the 28.07.96 changes is to not turn off the kernel timers until the actual
INPUT_PHASE_CALLBACK is initiated.  This takes care of all but direct
additions and removals of WMEs.  Since these are done through the add-wme and
remove-wme commands, the input_timer is turned off there was well, and the
kernel timers turned back on (for the buffered wm changes).  However, this is
a hack and may introduce problems when add-wme and remove-wme are used at the
command line or someplace in the decision cycle other than input (probably
rare but possible).

The DETAILED_TIMING_STATS flag enables collection of statistics on match,
ownership and chunking calculations performed in each part of the decision
cycle.  An 'other' value is reported which is simply the difference between
the sum of the deailed timers and the kernel timer for some pahse.  The other
value should always be greater than or equal to zero.

The "stats" command (in soarCommandUtils) has been updated to report these
new timing values.  The output is provided in a spreadsheet-style format to
display the information in a succinct form.  There are also some derived
totals in that report.  The derived totals in the right column are simply the
sum of the all the other columns in a particular row; for example, the
derived total for the first row, kernel time, is just the sum of all the
decision_cycle_phase_timers.  The derived totals in the bottom row are the
sum of all the basic timers in that row (i.e., no DETAILED statistics are
included in the sum).  For example, the derived total under input is equal to
the sum of decision_cycle_phase_timer and the monitors_time for the
INPUT_PHASE, and the input_function_cpu_time and represents the total time
spent in the input phase for the current run.  The number in the lower
right-hand corner is the sum of the derived totals above it in that right
column (and should always be equal to the numbers to the left of it in that
row).

Also reported with the stats command are the values of total_cpu_time and
total_kernel_time.  If the ordering discussed above is strictly enforced,
total_kernel_time should always be slightly greater than the derived total
kernel time and total_cpu_time greater than the derived total CPU time. REW */

  /* REW: begin 28.07.96 */  
  /* If in kernel.h, the timers are disabled by #define NO_TIMING_STUFF,
   * then these timevals will be just wasted space...
   * Usually they are enabled, so conditional compiles removed. July 05
   */
  ////#ifndef NO_TIMING_STUFF
  struct timeval      start_total_tv;
  struct timeval      total_cpu_time;
  struct timeval      start_kernel_tv, start_phase_tv;
  struct timeval      total_kernel_time;

  struct timeval      decision_cycle_phase_timers[NUM_PHASE_TYPES];
  struct timeval      monitors_cpu_time[NUM_PHASE_TYPES]; 
  struct timeval      input_function_cpu_time; 
  struct timeval      output_function_cpu_time; 
 
  /* accumulated cpu time spent in various parts of the system */
  /* only used if DETAILED_TIMING_STATS is #def'd in kernel.h */
  struct timeval      ownership_cpu_time[NUM_PHASE_TYPES];
  struct timeval      chunking_cpu_time[NUM_PHASE_TYPES];
  struct timeval      match_cpu_time[NUM_PHASE_TYPES];
  struct timeval      start_gds_tv, total_gds_time; 
  struct timeval      gds_cpu_time[NUM_PHASE_TYPES];
  /* REW: end 28.07.96 */
  ////#endif

   /* RMJ */
   /* Keep track of real time steps for constant real-time per decision */
   /* used only if #def'd REAL_TIME_BEHAVIOR */
   struct timeval	*real_time_tracker;
   Bool			real_time_idling;
 
   /* RMJ */
   /* Keep track of duration of attentional lapses */
   /* Used only if #def'd ATTENTION_LAPSE in */
   struct timeval	*attention_lapse_tracker;
   Bool			attention_lapsing;
 
  
  /* ----------------------- Chunker stuff -------------------------- */
  
  tc_number           backtrace_number;
  memory_pool         chunk_cond_pool;
  unsigned long       chunk_count;
  unsigned long       justification_count;
  Bool                chunk_free_flag;
  Bool                chunky_flag;     /* AGR MVL1 */
  list              * grounds;
  tc_number           grounds_tc;
  list              * instantiations_with_nots;
  list              * locals;
  tc_number           locals_tc;
  list              * positive_potentials;
  tc_number           potentials_tc;
  chunk_cond_set      negated_set; 
  preference        * results;
  goal_stack_level    results_match_goal_level;
  tc_number           results_tc_number;
  tc_number           variablization_tc;
  Bool                variablize_this_chunk;
  preference        * extra_result_prefs_from_instantiation;
  Bool                quiescence_t_flag;
  char                chunk_name_prefix[kChunkNamePrefixMaxLength];  /* kjh (B14) */
  
  /* ----------------------- Misc. top-level stuff -------------------------- */
  
  memory_pool         action_pool;
  memory_pool         complex_test_pool;
  memory_pool         condition_pool;
  memory_pool         not_pool;
  memory_pool         production_pool;
  
  /* ----------------------- Reorderer stuff -------------------------- */
  
  memory_pool         saved_test_pool;
  
  /* ----------------------- Memory utilities -------------------------- */
  
  /* Counters for memory usage of various types */
  unsigned long       memory_for_usage[NUM_MEM_USAGE_CODES];
  
  /* List of all memory pools being used */
  memory_pool       * memory_pools_in_use;
  
  memory_pool         cons_cell_pool; /* pool for cons cells */
  memory_pool         dl_cons_pool;   /* doubly-linked list cells */
  
  /* ----------------------- Explain.c stuff -------------------------- */
  
  backtrace_str     * explain_backtrace_list;     /* AGR 564 */
  explain_chunk_str * explain_chunk_list;         /* AGR 564 */
  char                explain_chunk_name[256];    /* AGR 564 */
  /* made explain_flag EXPLAIN_SYSPARAM instead, KJC 7/96 */
  /* Bool                explain_flag; */
  
  /* ----------------------- Firer stuff -------------------------- */
  
  memory_pool         instantiation_pool;
  instantiation     * newly_created_instantiations;
  
  /* production_being_fired -- during firing, points to the prod. being fired */
  production        * production_being_fired;
  
  unsigned long       max_rhs_unbound_variables;
  Symbol           ** rhs_variable_bindings;
  
  /* ==================================================================
     Decider stuff 
     =================================================================== */
  
  memory_pool         preference_pool;
  
  unsigned long       current_wme_timetag;
  memory_pool         wme_pool;
  list              * wmes_to_add;
  list              * wmes_to_remove;
  
  /* ---------------------------------------------------------------------
     Top_goal and bottom_goal point to the top and bottom goal identifiers,
     respectively.  (If there is no goal stack at all, they're both NIL.)
     Top_state points to the top state (symbol) if there is a top state, and
     is NIL of there isn't any top state selected.
  --------------------------------------------------------------------- */

  Symbol            * bottom_goal;
  Symbol            * top_goal;
  Symbol            * top_state;

  Symbol            * highest_goal_whose_context_changed;
  dl_list           * changed_slots;
  dl_list           * context_slots_with_changed_acceptable_preferences;
  memory_pool         slot_pool;
  list              * slots_for_possible_removal;

  dl_list           * disconnected_ids;
  goal_stack_level    highest_level_anything_could_fall_from;
  dl_list           * ids_with_unknown_level;
  goal_stack_level    lowest_level_anything_could_fall_to;
  tc_number           mark_tc_number;
  goal_stack_level    level_at_which_marking_started;
  goal_stack_level    walk_level;
  tc_number           walk_tc_number;
  list              * promoted_ids;
  int                 link_update_mode;

  /* ------------------ Printing utilities stuff --------------------- */

  FILE              * log_file;
  char              * log_file_name;
  Bool                logging_to_file;
  char                printed_output_string[MAX_LEXEME_LENGTH*2+10];
  int                 printer_output_column;
  Bool                redirecting_to_file;
  FILE              * redirection_file;
  int                 saved_printer_output_column;
  
  /* kjh(CUSP-B10) begin */
  /* ------------------ Recording/replaying stuff --------------------- */
  /*  Bool                replaying; */
  /* kjh(CUSP-B10) end */
  /* kjc 12/99 capture/replay input cycle files */
  FILE		    * capture_fileID;
  FILE		    * replay_fileID;
  Bool                replay_input_data;
  replay            * replay_data;
  unsigned long     * replay_timetags;
  /* Bool				  capture_input_wmes;
   * Bool				  replay_input_wmes; 
   */
  /* ----------------------- Trace Formats -------------------------- */
  
  struct trace_format_struct *(object_tf_for_anything[3]);
  struct hash_table_struct *(object_tr_ht[3]);
  Bool                printing_stack_traces;
  struct trace_format_struct *(stack_tf_for_anything[3]);
  struct hash_table_struct *(stack_tr_ht[3]);
  tc_number           tf_printing_tc;   
  
  list               * wme_filter_list; /* kjh(CUSP-B2) */

  /* ----------------------- RHS Function Stuff -------------------------- */
  
  /* --- "interrupt" fun. uses this to build "reason_for_stopping" msg. --- */
  //char                interrupt_source[2*MAX_LEXEME_LENGTH+100];
  
  /* --- "make-constant-symbol" counter --- */
  unsigned long       mcs_counter;

  /* ----------------------- O support stuff -------------------------- */
  
  tc_number           o_support_tc;   
  preference        * rhs_prefs_from_instantiation;
  
  /* ----------------------- I/O stuff -------------------------- */
  
  io_wme            * collected_io_wmes;
  struct output_link_struct * existing_output_links;

  struct output_link_struct * output_link_for_tc;
  memory_pool         output_link_pool;
  tc_number           output_link_tc_num;

  Bool                output_link_changed;
  
  Symbol            * io_header;
  wme               * io_header_link;
  
  Symbol            * io_header_input;
  Symbol            * io_header_output;

  memory_pool         io_wme_pool;
  Symbol            * prev_top_state;
  
  /* ------------ Varible Generator stuff (in production.c) ---------------- */
  
  unsigned long       current_variable_gensym_number;
  unsigned long       gensymed_variable_count[26];
  
  /* ------------------- Experimental features ---------------------- */
  int                 o_support_calculation_type;
  int                 attribute_preferences_mode;

  /* ------------------- Info about the agent itself ---------------------- */
  
  char              * name;  /* name of this Soar agent */

/* --------- I (RBD) don't know what the following stuff is ------------ */
  
  /* Soar uses these to generate nicely formatted output strings */
  char		    current_line[1024];
  int	        current_line_index;
 
  /* String redirection */
  Bool		    using_input_string;
  char		  * input_string;
  Bool		    using_output_string;
  char		  * output_string;
  
  /*mvp 5-17-94 */
  list              * variables_set;
  
  multi_attribute   * multi_attributes;
  /* char                path[MAXPATHLEN];    AGR 568 */
  
  /* JC ADDED: Array of callbacks for gSKI objects */
  gSKI_K_CallbackData  gskiCallbacks[gSKI_K_MAX_AGENT_EVENTS];

  //soar_callback_array soar_callbacks;
  list			      * soar_callbacks[NUMBER_OF_CALLBACKS];
  
  alias_struct      * alias_list;   /* AGR 568 */
  char              * alternate_input_string; 
  char              * alternate_input_suffix; 
  Bool                alternate_input_exit; /* Soar-Bugs #54, TMH */
  expansion_node    * lex_alias;         /* AGR 568 */
  Bool                load_errors_quit;  /* AGR 527c */
  dir_stack_struct  * top_dir_stack;   /* AGR 568 */
  

  /* RCHONG: begin 10.11 */
  Bool       did_PE;
  Bool       soar_verbose_flag;
  int        FIRING_TYPE;
  Symbol     *PE_level;

  struct ms_change_struct * ms_o_assertions;  /* changes to match set */
  struct ms_change_struct * ms_i_assertions;  /* changes to match set */
  /* RCHONG: end 10.11 */

  /* REW: begin 08.20.97 */
  Bool       operand2_mode;
  goal_stack_level active_level;
  goal_stack_level previous_active_level;
  Symbol *active_goal;
  Symbol *previous_active_goal;
  struct ms_change_struct *nil_goal_retractions; /* dll of all retractions for removed (ie nil) goals */
  /* REW: end   08.20.97 */

  /* delineate btwn Pref/WM(propose) and Pref/WM(apply) KJC 10.05.98 */
  Bool       applyPhase;

  /* REW: begin 10.24.97 */
  Bool       waitsnc;
  Bool       waitsnc_detect; 
  /* REW: end   10.24.97 */

   /* JC ADDED: link to owning kernel (for convenience and less param passing) */
  Kernel*    kernel;

  /* JC ADDED: Need to store RHS functions here so that agent's don't step on each other */
  rhs_function* rhs_functions;

#ifdef NUMERIC_INDIFFERENCE
  enum ni_mode numeric_indifferent_mode;      /* SW 08.19.2003 */
#endif

} agent;
/*************** end of agent struct *****/

#ifdef USE_MACROS

#define allocate_cons(thisAgent, dest_cons_pointer) \
  allocate_with_pool (thisAgent, &thisAgent->cons_cell_pool, (dest_cons_pointer))

#define free_cons(thisAgent, c) free_with_pool (&thisAgent->cons_cell_pool, (c))

#define push(thisAgent, item, list_header) { \
  cons *push_cons_xy298; \
  allocate_cons (thisAgent, &push_cons_xy298); \
  push_cons_xy298->first = (item); \
  push_cons_xy298->rest = (list_header); \
  (list_header) = push_cons_xy298; }

#else

#ifdef __cplusplus
}
#endif
 
template <typename T>
inline void allocate_cons(agent* thisAgent, T * dest_cons_pointer)
{
  allocate_with_pool (thisAgent, &thisAgent->cons_cell_pool, (dest_cons_pointer));
}

template <typename T>
inline void free_cons(agent* thisAgent, T * c)
{
  free_with_pool (&thisAgent->cons_cell_pool, (c));
}

template <typename P, typename T>
inline void push(agent* thisAgent, P item, T * & list_header)
{
  cons *push_cons_xy298;
  allocate_cons (thisAgent, &push_cons_xy298);
  push_cons_xy298->first = (item);
  push_cons_xy298->rest = (list_header);
  (list_header) = push_cons_xy298;
}

#ifdef __cplusplus
extern "C"
{
#endif

#endif /* USE_MACROS */

extern char * soar_version_string;

//extern agent * soar_agent;

extern agent * create_soar_agent (Kernel* thisKernel, char * name);
extern void    destroy_soar_agent (Kernel* thisKernel, agent* soar_agent);

void initialize_soar_agent(Kernel *thisKernel, agent* thisAgent);

/* Ideally, this should be in "lexer.h", but to avoid circular dependencies
   among header files, I am forced to put it here. */
#ifdef USE_MACROS
#define reading_from_top_level(soar_agent) (!soar_agent->current_file->parent_file)
#else

inline Bool reading_from_top_level(agent* soarAgent)
{
   return (!soarAgent->current_file->parent_file);
}

#endif /* USE_MACROS */

#ifdef __cplusplus
}
#endif

#endif
