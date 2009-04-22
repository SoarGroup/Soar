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
#include "soar_module.h"

typedef struct wme_struct wme;

//////////////////////////////////////////////////////////
// WMA Constants
//////////////////////////////////////////////////////////

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
#define WMA_ACTIVATION_NONE 99999999.9999

/**
 * If a WME's activation falls below this level it will be 
 * removed from working memory.
 */
#define WMA_ACTIVATION_CUTOFF -1.6


//////////////////////////////////////////////////////////
// WMA Parameters
//////////////////////////////////////////////////////////

class wma_decay_param;

class wma_param_container: public soar_module::param_container
{
	public:	
		
		enum criteria_choices { crit_agent, crit_agent_arch, crit_all };
		enum isupport_choices { none, no_create, uniform };
		enum precision_choices { low, high };		
		
		soar_module::boolean_param *activation;
		wma_decay_param *decay_rate;
		soar_module::constant_param<criteria_choices> *criteria;
		soar_module::boolean_param *forgetting;
		soar_module::constant_param<isupport_choices> *isupport;
		soar_module::boolean_param *persistence;
		soar_module::constant_param<precision_choices> *precision;
				
		wma_param_container( agent *new_agent );
};

class wma_decay_param: public soar_module::decimal_param
{
	public:
		wma_decay_param( const char *new_name, double new_value, soar_module::predicate<double> *new_val_pred, soar_module::predicate<double> *new_prot_pred );
		virtual void set_value( double new_value );
};

template <typename T>
class wma_activation_predicate: public soar_module::agent_predicate<T>
{	
	public:
		wma_activation_predicate( agent *new_agent );
		bool operator() ( T val );
};

//////////////////////////////////////////////////////////
// WMA Statistics
//////////////////////////////////////////////////////////

class wma_stat_container: public soar_module::stat_container
{
	public:	
		soar_module::integer_stat *dummy;		
				
		wma_stat_container( agent *new_agent );
};


//////////////////////////////////////////////////////////
// WMA Types
//////////////////////////////////////////////////////////

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

// shortcut for determining if WMA is enabled
extern bool wma_enabled( agent *my_agent );


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
extern double wma_get_wme_activation_high( agent *my_agent, wme *w );
extern double wma_get_wme_activation_low( agent *my_agent, wme *w );
extern double wma_get_wme_activation( agent *my_agent, wme *w );

/**
 * This routine performs WME activation
 * and forgetting at the end of each cycle.
 */
extern void wma_move_and_remove_wmes( agent *my_agent );

// quicky printer
extern void wma_print_activated_wmes( agent *my_agent, long n );

#endif
