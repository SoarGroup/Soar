/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  wma.h
 *
 * =======================================================================
 */

#ifndef WMA_H
#define WMA_H

#include <string>

typedef struct wme_struct wme;

//////////////////////////////////////////////////////////
// WMA Constants
//////////////////////////////////////////////////////////
#define WMA_RETURN_LONG 0.1
#define WMA_RETURN_STRING ""

/**
 * WMA on/off
 */
#define WMA_ACTIVATION_ON 1
#define WMA_ACTIVATION_OFF 2

/**
 * Specifies what WMEs will have decay values.
 * O_AGENT - Only o-supported WMEs created by the agent 
 *           (i.e., they have a supporting preference)
 * O_AGENT_ARCH - All o-supported WMEs including 
 *                architecture created WMEs
 * ALL - All wmes are activated
 */
#define WMA_CRITERIA_O_AGENT 1
#define WMA_CRITERIA_O_AGENT_ARCH 2
#define WMA_CRITERIA_ALL 3

/**
 * Are WMEs removed from WM when activation gets too low?
 */
#define WMA_FORGETTING_ON 1
#define WMA_FORGETTING_OFF 2

/**
 * Specifies the mode in which i-supported WMEs 
 * affect activation levels.
 * NONE - i-supported WMEs do not affect activation levels
 * NO_CREATE - i-supported WMEs boost the activation levels 
 *             of all o-supported WMEs in the instantiations 
 *             that test them.  Each WME receives and equal 
 *             boost regardless of "distance" (in the backtrace) 
 *             from the tested WME.
 * UNIFORM - i-supported WMEs boost the activation levels of 
 *           all o-supported WMEs in the instantiations that 
 *           created or test them.  Each WME receives an equal 
 *           boost regardless of "distance" (in the backtrace) 
 *           from the tested WME.
 */
#define WMA_I_NONE 1
#define WMA_I_NO_CREATE 2
#define WMA_I_UNIFORM 3

/**
 * Whether or not an instantiation activates WMEs just once, 
 * or every cycle until it is retracted.
 */
#define WMA_PERSISTENCE_ON 1
#define WMA_PERSISTENCE_OFF 2

/**
 * Level of precision with which activation levels are calculated.
 */
#define WMA_PRECISION_LOW 1
#define WMA_PRECISION_HIGH 2

// names of params
#define WMA_PARAM_ACTIVATION						0
#define WMA_PARAM_DECAY_RATE						1
#define WMA_PARAM_CRITERIA							2
#define WMA_PARAM_FORGETTING						3
#define WMA_PARAM_I_SUPPORT							4
#define WMA_PARAM_PERSISTENCE						5
#define WMA_PARAM_PRECISION							6
#define WMA_PARAMS									7 // must be 1+ last wma param

// names of stats
#define WMA_STAT_DUMMY								0
#define WMA_STATS									1 // must be 1+ last wma stat

//

/**
 * Size of the timelist array
 */
#define WMA_MAX_TIMELIST 200

/**
 * This is the size of the boost history (how much 
 * a WME was boosted on each of the last n cycles... 
 * where this constant defines n.)  this might be 
 * larger since we can have multiple updates on the 
 * same cycle, as opposed to Ron Chong's code which 
 * only allows one per cycle.
 * Default value = 10.
 */
#define WMA_DECAY_HISTORY 10

/**
 * The decay system uses a dynamic program algorithm
 * to calculate integer powers of numbers and avoid
 * calls to pow() after initialization.  This constant 
 * defines the size of the array where the values are 
 * stored.  this size should be bigger than the largest 
 * time interval possible to be seen in the decay history.  
 * One estimate is:
 *   (WMA_DECAY_HISTORY * time_for_a_single_decay) + (sum_of_ints_from_1_to_WMA_DECAY_HISTORY)
 * = (WMA_DECAY_HISTORY * time_for_a_single_decay) + (((WMA_DECAY_HISTORY + 1) * WMA_DECAY_HISTORY) / 2)
 */
#define WMA_POWER_SIZE 270

/**
 * If an external caller asks for the activation level/value 
 * of a WME that is not activated, then this is the value that
 * is returned.
 */
#define WMA_ACTIVATION_NONE_INT -1
#define WMA_ACTIVATION_NONE_DOUBLE 99999999.9999

/**
 * If a WME's activation falls below this level it will be 
 * removed from working memory.
 */
#define WMA_ACTIVATION_CUTOFF -1.6

//////////////////////////////////////////////////////////
// WMA Types
//////////////////////////////////////////////////////////

enum wma_param_type { wma_param_constant = 1, wma_param_number = 2, wma_param_string = 3, wma_param_invalid = 4 };

typedef struct wma_constant_parameter_struct  
{
	long value;
	bool (*val_func)( const long );
	const char *(*to_str)( const long );
	const long (*from_str)( const char * );
} wma_constant_parameter;

typedef struct wma_number_parameter_struct  
{
	double value;
	bool (*val_func)( double );
} wma_number_parameter;

typedef struct wma_string_parameter_struct  
{
	std::string *value;
	bool (*val_func)( const char * );
} wma_string_parameter;

typedef union wma_parameter_union_class
{
	wma_constant_parameter constant_param;
	wma_number_parameter number_param;
	wma_string_parameter string_param;
} wma_parameter_union;

typedef struct wma_parameter_struct
{
	wma_parameter_union *param;
	wma_param_type type;
	const char *name;
} wma_parameter;

typedef struct wma_stat_struct
{
	double value;
	const char *name;
} wma_stat;

//

typedef struct wma_decay_log_struct wma_decay_log;
typedef struct wma_decay_element_struct wma_decay_element_t;
typedef struct wma_timelist_element_struct wma_timelist_element;

struct wma_decay_log_struct
{
	float *act_log;
	int size;
	int start;
};

/**
 * attached to o-supported WMEs to keep track of its activation.
 */
struct wma_decay_element_struct
{	
	// each entry in the decay timelist contains a
	// linked list of these elements.  So these pointers
	// are needed for that list.
	wma_decay_element_t *next;
	wma_decay_element_t *previous;
	
	// the wme that this element goes with
	wme *this_wme;
	
	// a pointer to decay timelist array entry this struct is attached to
	wma_timelist_element *time_spot;
	
	// when an element if first created additional initialization
	// needs to be performed at move/decay time.
	bool just_created;

	// when a WME is removed from working memory, the data
	// structure is not necessarily deallocated right away
	// because its reference count has not fallen to zero.
	// This flag indicates that the WME is in this "limbo" state.
	bool just_removed;

	// how many times this wme has been referenced so far
	// this cycle
	long num_references;

	// when and how often this wme has been referenced in recent
	// history.  For example, if a WME has been referenced twice
	// in cycle 2, once in cycle 3, zero times in cycle 4 and
	// three times in cycle 5 the array will contain:
	// [2|2|3|5|5|5|?|?|?|?] where '?' is an unknown value
	unsigned long boost_history[ WMA_DECAY_HISTORY ];

	// how many references were matching on this wme at the end of
	// last cycle.
	long history_count;

	wma_decay_log *log;
};

/**
 * An array of these structures is used to keep track
 * of what WMEs are at each activation level.
 */
struct wma_timelist_element_struct
{
	// the cycle when the associated decay element was created
	long time;

	// index of this element in the decay timelist array
	long position;

	// pointer to the associated decay element
	wma_decay_element_t *first_decay_element;
};


//
// These must go below types
//

#include "stl_support.h"

//////////////////////////////////////////////////////////
// Parameter Functions
//////////////////////////////////////////////////////////

// clean memory
extern void wma_clean_parameters( agent *my_agent );

// add parameter
extern wma_parameter *wma_add_parameter( const char *name, double value, bool (*val_func)( double ) );
extern wma_parameter *wma_add_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) );
extern wma_parameter *wma_add_parameter( const char *name, const char *value, bool (*val_func)( const char * ) );

// convert parameter
extern const char *wma_convert_parameter( agent *my_agent, const long param );
extern const long wma_convert_parameter( agent *my_agent, const char *name );

// validate parameter
extern bool wma_valid_parameter( agent *my_agent, const char *name );
extern bool wma_valid_parameter( agent *my_agent, const long param );

// parameter type
extern wma_param_type wma_get_parameter_type( agent *my_agent, const char *name );
extern wma_param_type wma_get_parameter_type( agent *my_agent, const long param );

// get parameter
extern const long wma_get_parameter( agent *my_agent, const char *name, const double test );
extern const char *wma_get_parameter( agent *my_agent, const char *name, const char *test );
extern double wma_get_parameter( agent *my_agent, const char *name );

extern const long wma_get_parameter( agent *my_agent, const long param, const double test );
extern const char *wma_get_parameter( agent *my_agent, const long param, const char *test );
extern double wma_get_parameter( agent *my_agent, const long param );

// validate parameter value
extern bool wma_valid_parameter_value( agent *my_agent, const char *name, double new_val );
extern bool wma_valid_parameter_value( agent *my_agent, const char *name, const char *new_val );
extern bool wma_valid_parameter_value( agent *my_agent, const char *name, const long new_val );

extern bool wma_valid_parameter_value( agent *my_agent, const long param, double new_val );
extern bool wma_valid_parameter_value( agent *my_agent, const long param, const char *new_val );
extern bool wma_valid_parameter_value( agent *my_agent, const long param, const long new_val );

// set parameter
extern bool wma_set_parameter( agent *my_agent, const char *name, double new_val );
extern bool wma_set_parameter( agent *my_agent, const char *name, const char *new_val );
extern bool wma_set_parameter( agent *my_agent, const char *name, const long new_val );

extern bool wma_set_parameter( agent *my_agent, const long param, double new_val );
extern bool wma_set_parameter( agent *my_agent, const long param, const char *new_val );
extern bool wma_set_parameter( agent *my_agent, const long param, const long new_val );

// activation
extern bool wma_validate_activation( const long new_val );
extern const char *wma_convert_activation( const long val );
extern const long wma_convert_activation( const char *val );

// decay
extern bool wma_validate_decay( const double new_val );

// criteria
extern bool wma_validate_criteria( const long new_val );
extern const char *wma_convert_criteria( const long val );
extern const long wma_convert_criteria( const char *val );

// forgetting
extern bool wma_validate_forgetting( const long new_val );
extern const char *wma_convert_forgetting( const long val );
extern const long wma_convert_forgetting( const char *val );

// i-support
extern bool wma_validate_i_support( const long new_val );
extern const char *wma_convert_i_support( const long val );
extern const long wma_convert_i_support( const char *val );

// persistence
extern bool wma_validate_persistence( const long new_val );
extern const char *wma_convert_persistence( const long val );
extern const long wma_convert_persistence( const char *val );

// precision
extern bool wma_validate_precision( const long new_val );
extern const char *wma_convert_precision( const long val );
extern const long wma_convert_precision( const char *val );

// shortcut for determining if WMA is enabled
extern bool wma_enabled( agent *my_agent );

//////////////////////////////////////////////////////////
// Stat Functions
//////////////////////////////////////////////////////////

// memory clean
extern void wma_clean_stats( agent *my_agent );
extern void wma_reset_stats( agent *my_agent );

// add stat
extern wma_stat *wma_add_stat( const char *name );

// convert stat
extern const long wma_convert_stat( agent *my_agent, const char *name );
extern const char *wma_convert_stat( agent *my_agent, const long stat );

// valid stat
extern bool wma_valid_stat( agent *my_agent, const char *name );
extern bool wma_valid_stat( agent *my_agent, const long stat );

// get stat
extern double wma_get_stat( agent *my_agent, const char *name );
extern double wma_get_stat( agent *my_agent, const long stat );

// set stat
extern bool wma_set_stat( agent *my_agent, const char *name, double new_val );
extern bool wma_set_stat( agent *my_agent, const long stat, double new_val );

//////////////////////////////////////////////////////////
// The Meat
//////////////////////////////////////////////////////////

/* =======================================================================
                             wma.h

   This file implements working memory activation and decay mechanism for Soar.

   decay_init() must be called once before execution to init execution.
                Currently it is called by init_soar_agent(void) in agent.c
   decay_update_new_wme() - Adds a decay element to an existing WME so that it
                            becomes an activated WME
   remove_decay_structure(() - Removes a decay element from an existing WME so
                               that it is no longer activated.
   activate_wmes_in_pref() - Given a preference, this routine increments the
                             reference count of all its WMEs (as necessary).
   activate_wmes_in_inst() - Given a production instantiation, this routine
                             increments the reference count of all the WMEs
                             in its condition list.
   decay_update_wmes_in_retracted_inst() - Given a production instantiation,
                                           decrements the reference count of all
                                           the WMEs in its condition list.
   decay_update_wmes_tested_in_prods() - Increments the reference count of all
                                         WMEs that have been referenced this
                                         cycle
   decay_move_and_remove_wmes()        - This routine performs WME activation
                                         and forgetting at the end of each cycle.


   See wma.cpp for more information.
   ======================================================================= */

/**
 * Must be called before execution to init wma
 */
extern void wma_init( agent *my_agent );

/**
 * Prevents hanging pointers
 */
extern void wma_deinit( agent *my_agent );

/**
 * Adds a decay element to an existing WME so that it
 * becomes an activated WME
 */
extern void wma_update_new_wme( agent *my_agent, wme *w, int num_refs );

/**
 * Removes a decay element from an existing WME so that 
 * it is no longer activated.
 */
extern void wma_remove_decay_element( agent *my_agent, wme *w );

/**
 * Marks a decay element as being attached to a 
 * wme struct that has been removed from working memory.
 */
extern void wma_deactivate_element( agent * my_agent, wme *w );

/**
 * Given a preference, this routine increments the
 * reference count of all its WMEs (as necessary).
 */
extern void wma_activate_wmes_in_pref( agent *my_agent, preference *pref );

/**
 * Given a production instantiation, this routine
 * increments the reference count of all the WMEs
 * in its condition list.
 */
extern void wma_activate_wmes_in_inst( agent *my_agent, instantiation *inst );

/**
 * Given a production instantiation, 
 * decrements the reference count of all
 * the WMEs in its condition list.
 */
extern void wma_update_wmes_in_retracted_inst( agent *my_agent, instantiation *inst );

/**
 * Increments the reference count of all
 * WMEs that have been referenced this
 * cycle.
 */
extern void wma_update_wmes_tested_in_prods( agent *my_agent );

/**
 * Retrieve wme activation exact/approximate
 */
extern double wma_get_wme_activation( agent *my_agent, wme *w );
extern long wma_get_wme_activation_level( agent *my_agent, wme *w );

/**
 * This routine performs WME activation
 * and forgetting at the end of each cycle.
 */
extern void wma_move_and_remove_wmes( agent *my_agent );

// quicky printer
extern void wma_print_activated_wmes( agent *my_agent, long n );

#endif
