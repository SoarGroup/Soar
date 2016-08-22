/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

#include "kernel.h"

#include "Export.h"
#include "mem.h"
#include "memory_manager.h"
#include "misc.h"

#include <string>
#include <map>
#include <unordered_map>

// JRV: Added to support XML management inside Soar
// These handles should not be used directly, see xml.h
typedef void* xml_handle;

/* RBD Need more comments here, or should this stuff be here at all? */

#define UPDATE_LINKS_NORMALLY 0
#define UPDATE_DISCONNECTED_IDS_LIST 1
#define JUST_UPDATE_COUNT 2

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

class stats_statement_container;
#ifndef NO_SVS
class svs_interface;
#endif

typedef struct EXPORT agent_struct
{

    /* -- Rete stuff: These are used for statistics in rete.cpp -- */
    uint64_t actual[256], if_no_merging[256], if_no_sharing[256];

    uint64_t current_retesave_amindex;
    uint64_t reteload_num_ams;
    alpha_mem** reteload_am_table;

    uint64_t current_retesave_symindex;
    uint64_t reteload_num_syms;
    Symbol** reteload_symbol_table;

    token* dummy_matches_node_tokens;

    int64_t highest_rhs_unboundvar_index;

    //
    // Moved here from parser.cpp.  This is used to create temporary unique
    // identifiers for parsing.  This should probably be localized to the
    // production, but for now, this is much better than having it as a
    // global.
    //
    uint64_t placeholder_counter[26];

    //
    // Used to be a global,  this has been moved here from recmem.cpp
    //
    int64_t firer_highest_rhs_unboundvar_index;

    //
    // This was taked from reorder.cpp, but it is also used in production.cpp
    //
    char* name_of_production_being_reordered;

    //
    // These was taked from print.
    Symbol* action_id_to_match;
    test id_test_to_match;

    //
    // This was taken from production.h
    //
    tc_number current_tc_number;

    /////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    /* Hash tables for alpha memories, and for entries in left & right memories */
    void*               left_ht;
    void*               right_ht;
    hash_table*        (alpha_hash_tables[16]);

    /* Number of WMEs, and list of WMEs, the Rete has been told about */
    uint64_t            num_wmes_in_rete;
    wme*                all_wmes_in_rete;

    /* Dummy nodes and tokens */
    struct rete_node_struct* dummy_top_node;
    struct token_struct* dummy_top_token;

    /* Various Rete statistics counters */
    uint64_t       rete_node_counts[256];
    uint64_t       rete_node_counts_if_no_sharing[256];
    uint64_t       token_additions;
    uint64_t       token_additions_without_sharing;
    uint64_t       num_right_activations;
    uint64_t       num_left_activations;
    uint64_t       num_null_right_activations;
    uint64_t       num_null_left_activations;


    /* Miscellaneous other stuff */
    uint32_t       alpha_mem_id_counter; /* node id's for hashing */
    uint32_t       beta_node_id_counter;
    struct ms_change_struct* ms_assertions;   /* changes to match set */
    struct ms_change_struct* ms_retractions;

    Symbol_Manager*             symbolManager;
    WM_Manager*                 WM;
    RL_Manager*                 RL;
    SMem_Manager*               SMem;
    EpMem_Manager*              EpMem;
    Explanation_Based_Chunker*  explanationBasedChunker;
    Memory_Manager*             memoryManager;
    Output_Manager*             outputManager;
    Explanation_Memory*         explanationMemory;
    GraphViz_Visualizer*        visualizationManager;


    // Used by the parser to promote LTIs in rules after database is loaded
    LTI_Promotion_Set*  LTIs_sourced;

    /* ----------------------- Top-level stuff -------------------------- */

    /* --- headers of dll's of all productions of each type --- */
    production*         all_productions_of_type[NUM_PRODUCTION_TYPES];
    /* --- counts of how many productions there are of each type --- */
    uint64_t            num_productions_of_type[NUM_PRODUCTION_TYPES];

    /* --- default depth for "print" command --- */
    int                 default_wme_depth;      /* AGR 646 */

    /* --- stuff for "input-period" command --- */
    /* --- in Soar8, input runs once at beginning of D cycle, no matter what */
    int                 input_period;      /* AGR REW1 */
    bool                input_cycle_flag;  /* AGR REW1 */

    /* --- current top level phase --- */
    enum top_level_phase current_phase;

    /* --- to interrupt at the end of the current phase, set stop_soar to true
     and reason_for_stopping to some appropriate string --- */
    bool                stop_soar;
    const char*         reason_for_stopping;

    /* --- the RHS action (halt) sets this true --- */
    bool                system_halted;

    /* --- list of productions whose firings are being traced --- */
    ::list*             productions_being_traced;

    /* --- various user-settable system parameters --- */
    int64_t             sysparams[HIGHEST_SYSPARAM_NUMBER + 1];

    /* --- parameters for running Soar --- */
    /*  --- the code loops go_number times over the go_type phases --- */
    int64_t             go_number;     /* How many times to "go" */
    Symbol*             go_slot_attr;  /* The context slot checked */
    goal_stack_level    go_slot_level; /* The goal stack level checked */
    enum go_type_enum   go_type;       /* The phase type used */

    /* --- Top-level Statistics --- */

    /* running total of WM sizes at end of phases */
    double              cumulative_wm_size;
    /* number of items included in "cumulative_wm_size" sum */
    uint64_t            num_wm_sizes_accumulated;

    uint64_t            max_wm_size;    /* maximum size of WM so far */
    uint64_t            wme_addition_count; /* # of wmes added to WM */
    uint64_t            wme_removal_count;  /* # of wmes removed from WM */

    uint64_t            start_dc_wme_addition_count; /* for calculating max_dc_wm_changes */
    uint64_t            start_dc_wme_removal_count;  /* for calculating max_dc_wm_changes */
    uint64_t            max_dc_wm_changes_value;  /* # of wmes added + removed in a single dc */
    uint64_t            max_dc_wm_changes_cycle;  /* # cycle of max_dc_wm_changes */

    uint64_t            init_count;             /* # of inits done so far */
    uint64_t            d_cycle_count;          /* # of DC's run so far */
    uint64_t            e_cycle_count;          /* # of EC's run so far */
    /*  in Soar 8, e_cycles_this_d_cycle is reset to zero for every
        propose and apply phase */
    uint64_t            e_cycles_this_d_cycle;  /* # of EC's run this DC */
    uint64_t            num_existing_wmes;      /* current WM size */
    uint64_t            production_firing_count;  /* # of prod. firings */
    uint64_t            start_dc_production_firing_count;  /* # of prod. firings this decision cycle */
    uint64_t            max_dc_production_firing_count_value;  /* max # of prod. firings per dc */
    uint64_t            max_dc_production_firing_count_cycle;  /* cycle of max_dc_production_firing_count_value */
    uint64_t            d_cycle_last_output;    /* last time agent produced output */  //KJC 11.17.05
    uint64_t            decision_phases_count;  /* can differ from d_cycle_count.  want for stats */
    //?? uint64_t            out_cycle_count;       /* # of output phases have gen'd output */
    //?? uint64_t            phase_count;       /* # of phases run so far */
    /* DJP 2/22/07: These counts are based around the counts that the run command understands and are intended to capture the same semantics as run expects.
       That may differ from some of the other counters above which historically may track slightly different values */
    uint64_t            run_phase_count ;             /* # of phases run since last init-soar */
    uint64_t            run_elaboration_count ;       /* # of elaboration cycles run since last init-soar.  A phase where nothing happens counts as an elaboration cycle */
    uint64_t            run_last_output_count ;       /* # of output phases since this agent last generated output */
    uint64_t            run_generated_output_count ;  /* # of output phases when this agent either generated output or reached "max-nil-output" cycles since last init-soar */

    /* REW: begin 09.15.96 */
    /* in Soar 8, PE's are done only during the APPLY phase */
    uint64_t            pe_cycle_count;          /* # of PE's run so far */
    uint64_t            pe_cycles_this_d_cycle;  /* # of PE's run this DC */

    parent_inst* parent_list_head;
    /* REW: end   09.15.96 */

    /* State for new waterfall model */
    uint64_t            inner_e_cycle_count;     /* # of inner elaboration cycles run so far */

    /* ----------------------- Timing statistics -------------------------- */

    /*
    When the compile flag NO_TIMING_STUFF is off, statistics will be collected on
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
#ifndef NO_TIMING_STUFF
    soar_timer timers_cpu;    // start_total_tv
    soar_timer timers_kernel; // start_kernel_tv
    soar_timer timers_phase;  // start_phase_tv

    soar_timer_accumulator timers_total_cpu_time;                         // total_cpu_time
    soar_timer_accumulator timers_total_kernel_time;                      // total_kernel_time
    soar_timer_accumulator timers_decision_cycle_phase[NUM_PHASE_TYPES];  // decision_cycle_phase_timers

    soar_timer_accumulator timers_monitors_cpu_time[NUM_PHASE_TYPES]; // monitors_cpu_time, uses timers_phase
    soar_timer_accumulator timers_input_function_cpu_time;            // input_function_cpu_time, uses timers_kernel
    soar_timer_accumulator timers_output_function_cpu_time;           // output_function_cpu_time, uses timers_kernel

    uint64_t last_derived_kernel_time_usec;       // Total of the time spent in the phases of the decision cycle,
    // excluding Input Function, Output function, and pre-defined callbacks.
    // Computed at the end of the output phase, so it is not valid for current
    // phase until then.
    uint64_t max_dc_time_usec;                    // Holds maximum amount of decision cycle time
    uint64_t max_dc_time_cycle;                   // Holds cycle_count that maximum amount of decision cycle time happened

    double max_dc_epmem_time_sec;                 // Holds maximum amount epmem time
    double total_dc_epmem_time_sec;               // Holds last amount epmem time, used to calculate delta
    uint64_t max_dc_epmem_time_cycle;             // Holds what cycle max_dc_epmem_time_sec was acheived

    double max_dc_smem_time_sec;                  // Holds maximum amount smem time
    double total_dc_smem_time_sec;                // Holds last amount smem time, used to calculate delta
    uint64_t max_dc_smem_time_cycle;              // Holds what cycle max_dc_smem_time_sec was acheived

    soar_timer_accumulator callback_timers[NUMBER_OF_CALLBACKS];

    /* accumulated cpu time spent in various parts of the system */
    /* only used if DETAILED_TIMING_STATS is #def'd in kernel.h */
#ifdef DETAILED_TIMING_STATS
    soar_timer timers_gds;                                        // start_gds_tv
    soar_timer_accumulator timers_ownership_cpu_time[NUM_PHASE_TYPES];    // ownership_cpu_time
    soar_timer_accumulator timers_chunking_cpu_time[NUM_PHASE_TYPES];     // chunking_cpu_time
    soar_timer_accumulator timers_match_cpu_time[NUM_PHASE_TYPES];        // match_cpu_time
    soar_timer_accumulator timers_gds_cpu_time[NUM_PHASE_TYPES];          // gds_cpu_time
#endif // DETAILED_TIMING_STATS
    /* REW: end 28.07.96 */
#endif // NO_TIMING_STUFF

    /* RMJ */
    /* Keep track of real time steps for constant real-time per decision */
    /* used only if #def'd REAL_TIME_BEHAVIOR */
    struct timeval*   real_time_tracker;
    bool              real_time_idling;

    /* RMJ */
    /* Keep track of duration of attentional lapses */
    /* Used only if #def'd ATTENTION_LAPSE in */
    struct timeval*   attention_lapse_tracker;
    bool              attention_lapsing;

    /* ----------------------- Firer stuff -------------------------- */

    instantiation*      newly_created_instantiations;

    /* production_being_fired -- during firing, points to the prod. being fired */
    production*         production_being_fired;

    uint64_t            max_rhs_unbound_variables;
    Symbol**            rhs_variable_bindings;

    /* ==================================================================
       Decider stuff
       =================================================================== */

    uint64_t            current_wme_timetag;
    ::list*             wmes_to_add;
    ::list*             wmes_to_remove;

    /* ---------------------------------------------------------------------
       Top_goal and bottom_goal point to the top and bottom goal identifiers,
       respectively.  (If there is no goal stack at all, they're both NIL.)
       Top_state points to the top state (symbol) if there is a top state, and
       is NIL of there isn't any top state selected.
    --------------------------------------------------------------------- */

    Symbol*             bottom_goal;
    Symbol*             top_goal;
    Symbol*             top_state;

    Symbol*             highest_goal_whose_context_changed;
    dl_list*            changed_slots;
    dl_list*            context_slots_with_changed_acceptable_preferences;
    ::list*             slots_for_possible_removal;

    dl_list*            disconnected_ids;
    goal_stack_level    highest_level_anything_could_fall_from;
    dl_list*            ids_with_unknown_level;
    goal_stack_level    lowest_level_anything_could_fall_to;
    tc_number           mark_tc_number;
    goal_stack_level    level_at_which_marking_started;
    goal_stack_level    walk_level;
    tc_number           walk_tc_number;
    ::list*             promoted_ids;
    int                 link_update_mode;

    /* ----------------------- Trace Formats -------------------------- */

    struct trace_format_struct* (object_tf_for_anything[3]);
    struct hash_table_struct* (object_tr_ht[3]);
    bool               printing_stack_traces;
    struct trace_format_struct* (stack_tf_for_anything[3]);
    struct hash_table_struct* (stack_tr_ht[3]);
    tc_number           tf_printing_tc;

    ::list*             wme_filter_list; /* kjh(CUSP-B2) */

    /* ----------------------- RHS Function Stuff -------------------------- */

    /* --- "make-constant-symbol" counter --- */
    uint64_t            mcs_counter;

    /* ----------------------- O support stuff -------------------------- */

    tc_number           o_support_tc;
    preference*         rhs_prefs_from_instantiation;

    /* ----------------------- I/O stuff -------------------------- */

    io_wme*             collected_io_wmes;
    struct output_link_struct* existing_output_links;

    struct output_link_struct* output_link_for_tc;
    tc_number           output_link_tc_num;

    bool               output_link_changed;

    Symbol*             io_header;
    wme*                io_header_link;

    Symbol*             io_header_input;
    Symbol*             io_header_output;

    Symbol*             prev_top_state;

    /* ------------------- Experimental features ---------------------- */
    int                 o_support_calculation_type;

    /* ------------------- Info about the agent itself ---------------------- */

    char*               name;  /* name of this Soar agent */

    /* --------- I (RBD) don't know what the following stuff is ------------ */

    /* Soar uses these to generate nicely formatted output strings */
    char          current_line[1024];
    int           current_line_index;

    /*mvp 5-17-94 */
    ::list*             variables_set;

    multi_attribute*    multi_attributes;
    /* char                path[MAXPATHLEN];    AGR 568 */

    //soar_callback_array soar_callbacks;
    ::list*                   soar_callbacks[NUMBER_OF_CALLBACKS];

    /* RCHONG: begin 10.11 */
    bool      did_PE;
    bool      soar_verbose_flag;
    int        FIRING_TYPE;
    Symbol*     PE_level;

    struct ms_change_struct* ms_o_assertions;   /* changes to match set */
    struct ms_change_struct* ms_i_assertions;   /* changes to match set */
    /* RCHONG: end 10.11 */

    struct ms_change_struct* postponed_assertions;   /* New waterfall model: postponed assertion list */

    goal_stack_level active_level;
    goal_stack_level previous_active_level;
    Symbol* active_goal;
    Symbol* previous_active_goal;
    struct ms_change_struct* nil_goal_retractions; /* dll of all retractions for removed (ie nil) goals */

    /**
     * State for new waterfall model
     * Represents the original active level of the elaboration cycle, saved so that we can modify the active
     * level during the inner preference loop and restore it before working memory changes.
     */
    goal_stack_level highest_active_level;
    /**
     * State for new waterfall model
     * Same as highest_active_level, just the goal that the level represents.
     */
    Symbol* highest_active_goal;
    /**
     * State for new waterfall model
     * Can't fire rules at this level or higher (lower int)
     */
    goal_stack_level change_level;
    /**
     * State for new waterfall model
     * Next change_level, in next iteration of inner preference loop.
     */
    goal_stack_level next_change_level;

    /* delineate btwn Pref/WM(propose) and Pref/WM(apply) KJC 10.05.98 */
    bool      applyPhase;

    /* REW: begin 10.24.97 */
    bool      waitsnc;
    bool      waitsnc_detect;
    /* REW: end   10.24.97 */

    /* JC ADDED: Need to store RHS functions here so that agent's don't step on each other */
    rhs_function* rhs_functions;

    enum ni_mode numeric_indifferent_mode;      /* SW 08.19.2003 */

    // select
    select_info* select;

    // predict
    uint32_t     predict_seed;
    std::string* prediction;

    // debug parameters
    debug_param_container* debug_params;

    // parser symbol clean-up list
    ::list*             parser_syms;

    AgentOutput_Info* output_settings;

    // BasicWeightedCue from JSoar for unit testing
	class BasicWeightedCue
	{
	public:
		const wme_struct* cue;
		const long weight;

		BasicWeightedCue(wme_struct* c, long w) : cue(c), weight(w) {}
	};
	BasicWeightedCue* lastCue;


    // dynamic RHS counters
    std::unordered_map< std::string, uint64_t >* dyn_counters;


    // JRV: Added to support XML management inside Soar
    // These handles should not be used directly, see xml.h
    xml_handle xml_destination;       // The current destination for all XML generation, essentially either == to xml_trace or xml_commands
    xml_handle xml_trace;             // During a run, xml_destination will be set to this pointer.
    xml_handle xml_commands;          // During commands, xml_destination will be set to this pointer.

    // stats database
    bool dc_stat_tracking;
    soar_module::sqlite_database* stats_db;
    stats_statement_container* stats_stmts;

    // Soar execution will be interrupted when this substate level is removed
    goal_stack_level substate_break_level;

#ifndef NO_SVS
    svs_interface* svs;
#endif
} agent;
/*************** end of agent struct *****/

template <typename T>
inline void allocate_cons(agent* thisAgent, T* dest_cons_pointer)
{
    thisAgent->memoryManager->allocate_with_pool(MP_cons_cell, (dest_cons_pointer));
}

template <typename T>
inline void free_cons(agent* thisAgent, T* c)
{
    thisAgent->memoryManager->free_with_pool(MP_cons_cell, (c));
}

template <typename P, typename T>
inline void push(agent* thisAgent, P item, T*& list_header)
{
    cons* push_cons_xy298;
    allocate_cons(thisAgent, &push_cons_xy298);
    push_cons_xy298->first = (item);
    push_cons_xy298->rest = (list_header);
    (list_header) = push_cons_xy298;
}

void    init_soar_agent(agent* thisAgent);
agent*  create_soar_agent(char* name);
void    destroy_soar_agent(agent* soar_agent);
bool    reinitialize_agent(agent* thisAgent);

#endif

