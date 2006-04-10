/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                               activate.h
======================================================================= */

#ifndef ACTIVATE_H
#define ACTIVATE_H


#include "wmem.h"
#include "gdatastructs.h"
#include "instantiations.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* =======================================================================
                             activate.c

   This file implements the WME activation and decay mechanism for Soar.

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


   See activate.c for more information.
   ======================================================================= */

extern void decay_init(agent *thisAgent);
extern void decay_deinit(agent *thisAgent);
extern void decay_update_new_wme(agent *thisAgent, wme *w, int num_refs);
extern void activate_wmes_in_pref(agent *thisAgent, preference *pref);
extern void activate_wmes_in_inst(agent *thisAgent, instantiation *inst);
extern void decay_update_wmes_tested_in_prods(agent *thisAgent);
extern void decay_update_wmes_in_retracted_inst(agent *thisAgent, instantiation *inst);
extern int decay_activation_level(agent *thisAgent, wme *w);
extern float decay_activation(agent *thisAgent, wme *w);
extern void decay_move_and_remove_wmes(agent *thisAgent);
extern void decay_deactivate_element(agent *thisAgent, wme *w);
extern void decay_remove_element(agent *thisAgent, wme *w);
extern void decay_print_most_activated_wmes(agent *thisAgent, int n);
    
/*
 * Decay constants
 *
 * MAX_DECAY          - These constants define the size of the decay 
 * & DECAY_ARRAY_SIZE   timelist array
 *
 * DECAY_HISTORY_SIZE - This is the size of the boost history (how much a WME
 *                      was boosted on each of the last n cycles...where this
 *                      constant defines n.)  this might be larger since we can
 *                      have multiple updates on the same cycle, as opposed to
 *                      Ron Chong's code which only allows one per cycle.
 *                      Default value = 10.
 *
 * DECAY_POWER_ARRAY_SIZE - The decay system uses a dynamic program algorithm
 *                          to calculate integer powers of numbers and avoid
 *                          calls to pow() after initialization.  This
 *                          constant defines the size of the array where the
 *                          values are stored.  this size should be bigger than
 *                          the largest time interval possible to be seen in the
 *                          decay history.  One estimate is:
 *                          (DECAY_HISTORY_SIZE * time_for_a_single_decay) + (sum_of_ints_from_1_to_DECAY_HISTORY_SIZE)
 *                          = (DECAY_HISTORY_SIZE * time_for_a_single_decay) + (((DECAY_HISTORY_SIZE + 1) * DECAY_HISTORY_SIZE) / 2)
 *
 * DECAY_ACTIVATION_CUTOFF - If a WME's activation falls below this level it
 *                           will be removed from working memory.
 *
 * DECAY_EXPONENT_DIVISOR - The user can specify the exponent on the command
 *                          line using an integer.  The integer is divided
 *                          by this constant in order to calculate an real
 *                          valued exponent.
 *
 * DECAY_DEFAULT_EXPONENT - Default value (-800) for the WME_DECAY_EXPONENT_SYSPARAM
 *                          which specifies the rate at which WMEs decay.  Note
 *                          that since sysparams must be integers the value is
 *                          specified as a number to be divided by the
 *                          DECAY_EXPONENT_DIVISOR (above).  Thus the default
 *                          exponent is actually -0.8.
 *
 * DECAY_DEFAULT_WME_CRITERIA - Default value for the
 *                              WME_DECAY_DEFAULT_WME_CRITERIA sysparam which
 *                              specifies what WMEs will have decay values.
 *                              Currently there are two modes:
 *     DECAY_WME_CRITERIA_O_SUPPORT_ONLY - only o-supported WMEs created by
 *                                         the agent (i.e., they have a
 *                                         supporting preference)
 *     DECAY_WME_CRITERIA_O_ARCH         - All o-supported WMEs including
 *                                         architecture created WMEs like
 *                                         ^superstate and ^input-link WMEs.
 *     DECAY_WME_CRITERIA_ALL            - All wmes are activated
 *
 * DECAY_DEFAULT_ALLOW_FORGETTING - Default value (TRUE) for the
 *                                  WME_DEFAULT_ALLOW_FORGETTING sysparam.
 *
 * DECAY_DEFAULT_I_SUPPORT_MODE   - Default value for the
 *                                  WME_DEFAULT_I_SUPPORT_MODE sysparam whic
 *                                  specifies the mode in which i-supported WMEs
 *                                  affect activation levels.  Currently there are
 *                                  two supported modes:
 *     DECAY_I_SUPPORT_MODE_NONE        - i-supported WMEs do not affect
 *                                        activation levels
 *     DECAY_I_SUPPORT_MODE_NO_CREATE   - i-supported WMEs boost the activation
 *                                        levels of all o-supported WMEs in the
 *                                        instantiations that test them.  Each
 *                                        WME receives and equal boost
 *                                        regardless of "distance" (in the
 *                                        backtrace) from the tested WME.
 *     DECAY_I_SUPPORT_MODE_UNIFORM     - i-supported WMEs boost the activation
 *                                        levels of all o-supported WMEs in the
 *                                        instantiations that created or test
 *                                        them.  Each
 *                                        WME receives and equal boost
 *                                        regardless of "distance" (in the
 *                                        backtrace) from the tested WME.
 *
 * DECAY_DEFAULT PERSISTENT_ACTIVATION - Default value (FALSE) for the
 *                                       WME_DECAY_PERSISTENT_ACTIVATION
 *                                       sysparam which specifies whether or 
 *                                       not an instantiation activates
 *                                       WMEs just once, or every cycle until
 *                                       it is retracted.
 *
 *
 * DECAY_INT_NO_ACTIVATION               If an external caller asks for the
 *                                       activation level of a WME that is not
 *                                       activated, then this is the value that
 *                                       is returned. 
 * DECAY_FLOAT_NO_ACTIVATION             If an external caller asks for the
 *                                       activation of a WME that is not
 *                                       activated, then this is the value that
 *                                       is returned. 
 */
#define MAX_DECAY 200
#define DECAY_ARRAY_SIZE (MAX_DECAY + 1)
#define DECAY_HISTORY_SIZE 10
#define DECAY_POWER_ARRAY_SIZE 270
#define DECAY_ACTIVATION_CUTOFF -1.6
#define DECAY_EXPONENT_DIVISOR 1000.0
#define DECAY_DEFAULT_EXPONENT -800
#define DECAY_ACTIVATION_LOG_SIZE 10

#define DECAY_WME_CRITERIA_O_SUPPORT_ONLY       0
#define DECAY_WME_CRITERIA_O_ARCH               1
#define DECAY_WME_CRITERIA_ALL                  2
#define DECAY_DEFAULT_WME_CRITERIA              DECAY_WME_CRITERIA_O_ARCH

#define DECAY_DEFAULT_ALLOW_FORGETTING          0

#define DECAY_I_SUPPORT_MODE_NONE               0
#define DECAY_I_SUPPORT_MODE_NO_CREATE          1
#define DECAY_I_SUPPORT_MODE_UNIFORM            2
#define DECAY_DEFAULT_I_SUPPORT_MODE            DECAY_I_SUPPORT_MODE_NONE

#define DECAY_DEFAULT_PERSISTENT_ACTIVATION         0

#define DECAY_PRECISION_HIGH                    0
#define DECAY_PRECISION_LOW                     1
#define DECAY_DEFAULT_PRECISION                 DECAY_PRECISION_HIGH

#define DECAY_DEFAULT_LOGGING                   0

#define DECAY_INT_NO_ACTIVATION                 -1
#define DECAY_FLOAT_NO_ACTIVATION               99999999.9999
    

/*
 * Decay Data Structures
 * 
 * wme_decay_element - this struct is attached to o-supported WMEs to keep track
 *                     of its activation.
 *    Fields: 
 *    next/previous - each entry in the decay timelist contains a
 *                    linked list of these elements.  So these pointers
 *                    are needed for that list.
 *    this_wme      - the wme that this element goes with
 *    time_spot     - a pointer to decay timelist array entry this struct
 *                    is attached to.
 *    just_created  - when an element if first created additional initialization
 *                    needs to be performed at move/decay time.
 *    just_removed  - when a WME is removed from working memory, the data
 *                    structure is not necessarily deallocated right away
 *                    because its reference count has not fallen to zero.
 *                    This flag indicates that the WME is in this "limbo" state.
 *                    NOTE:  Should this flag be moved to the wme struct?
 *    num_references- how many times this wme has been referenced so far
 *                    this cycle
 *    boost_history - when and how often this wme has been referenced in recent
 *                    history.  For example, if a WME has been referenced twice
 *                    in cycle 2, once in cycle 3, zero times in cycle 4 and
 *                    three times in cycle 5 the array will contain:
 *                     [2|2|3|5|5|5|?|?|?|?] where '?' is an unknown value
 *    history_count - how many references were matching on this wme at the end of
 *                    last cycle.
 *
 *
 * decay_timelist_element - An array of these structures is used to keep track
 *                          of what WMEs are at each activation level.
 *    Fields:
 *    time                     - the cycle when the associated decay element was
 *                               created
 *    position                 - index of this element in  the decay timelist
 *                               array 
 *    wme_decay_element_struct - pointer to the associated decay element
 *
 */

typedef struct decay_log_struct
{
    float                                   *act_log;
    int                                      size;
    int                                      start;
} decay_log;

typedef struct wme_decay_element_struct
{
	struct wme_decay_element_struct			*next;
	struct wme_decay_element_struct			*previous;
	struct wme_struct						*this_wme;
	struct decay_timelist_element_struct	*time_spot;
    bool                                     just_created;
    bool                                     just_removed;
    int                                      num_references;
    unsigned long                            boost_history[DECAY_HISTORY_SIZE];
    int                                      history_count;
    decay_log                               *log;
} wme_decay_element;


typedef struct decay_timelist_element_struct
{
	long int							time;
	int									position;
	struct wme_decay_element_struct		*first_decay_element;
} decay_timelist_element;


#ifdef __cplusplus
}//extern "C"
#endif


#endif  //ACTIVATE_H
