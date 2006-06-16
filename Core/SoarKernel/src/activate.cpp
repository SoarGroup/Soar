/*************************************************************************
 *
 *  file:  activate.cpp
 *
 * =======================================================================
 *  
 *        Memory Activation and Decay Routines for Working Memory
 *
 * Original code by Michael James in 2001-2002
 * Overhauled by Andrew Nuxoll in 2003
 * Ported to Soar 8.6 by Andrew Nuxoll in 2005
 *  
 * 
 * decay_init() must be called once before execution to init execution.
 *              Currently it is called by init_soar_agent(void) in agent.c
 * decay_deinit() called when the user turns decay off.  This cleans up
 *                the decay timelist
 * decay_update_new_wme() - Adds a decay element to an existing WME so that it
 *                          becomes an activated WME
 * decay_remove_element() - Removes a decay element from an existing WME so
 *                          that it is no longer activated.
 * activate_wmes_in_pref() - Given a preference, this routine increments the
 *                           reference count of all its WMEs (as necessary).
 * activate_wmes_in_inst() - Given a production instantiation, this routine
 *                           increments the reference count of all the WMEs
 *                           in its condition list.
 * decay_update_wmes_in_retracted_inst() - Given a production instantiation,
 *                                         decrements the reference count of all
 *                                         the WMEs in its condition list.
 * decay_update_wmes_tested_in_prods() - Increments the reference count of all
 *                                       WMEs that have been referenced this
 *                                       cycle
 * decay_move_and_remove_wmes()        - This routine performs WME activation
 *                                       and forgetting at the end of each cycle.
 *
 *
 * These decay routines are stored in other files (%%%OUT OF DATE%%%):
 *     soar_Decay (soarapi.c) - command line function that allows the user
 *                              to toggle activation and make other settings.
 *     print_current_decay_settings - prints the status of the decay
 *     (soarapiUtils.c)               including a list of info on WMEs that
 *                                    have decay elements
 *     soar_cSetDecay    -  command line function that allows the user to
 *     (soar_core_api.c)    toggle activation and make other settings.
 * 
 * =======================================================================
 *
 * Copyright (c) 1995-2003 Carnegie Mellon University,
 *                         The Regents of the University of Michigan,
 *                         University of Southern California/Information
 *                         Sciences Institute.  All rights reserved.
 *
 * The Soar consortium proclaims this software is in the public domain, and
 * is made available AS IS.  Carnegie Mellon University, The University of 
 * Michigan, and The University of Southern California/Information Sciences 
 * Institute make no warranties about the software or its performance,
 * implied or otherwise.
 *
 * =======================================================================
 */

/*************************************************************************
    OVERVIEW: How Soar's Decay Mechanism Works
    ------------------------------------------

    I.  High Level Data Structures

    O-supported WMEs have an associated decay data structure.  This structure
    (wme_decay_element_struct) is declared in soarkernel.h.  Key among the
    attributes of this structure are:
        1.  num_references - The number of times the WME has been
            "referenced" this cycle (see definition of referenced below)
        2.  boost_history - A log of how many times the WME has been
            referenced in each of the last 50 cycles

    For this implementation a WME is referenced if:
    1.  It is created by an o-supported production
    2.  If it is re-created (i.e., same WME created by a different
        instantiation) by an o-supported production
    3.  It is tested in the condition list of a new production instantiation
    4.  If the default behavior is overriden with the command 'decay
        -activation persistent', then a WME will also be referenced if
        it is in the conditions of a production instantiation created
        in a previous cycle that has not yet been retracted.

    The decay system maintains a global "decay timelist" (declared in
    soarkernel.h) for each agent.  This timelist is an array indexed by
    removal time and treated as a circular queue.  Thus the current entry
    in the array contains a list of WMEs (reached via a
    "decay_timelist_element") which will decay at the end of the current
    cycle if they are not referenced.

    It should be noted that the wme_decay_element_struct does *not*
    maintain an actual numerical activation level for the WME.  This value
    is calculated only when needed.  It is needed at two times:
       1.  To calculate the WME's new position in the decay timelist
           whenever the WME is referenced
       2.  To display the activation value to the user when prompted


    II.  High Level Functional Overview

        A.  Creation

            During the action phase, the system detects when an
            o-supported production creates a new WME.  A
            wme_decay_element_struct is allocated and attached to the WME.
            Additionally the WME's refernece count for the current cycle
            is set to 1.  These WMEs are henceforth called "activated
            WMEs" meaning that they have an activation value associated
            with them.

        B.  Referencing

            During each elaboration phase of a cycle (i.e., proposal and
            action) the decay system detects which activated WMEs are
            referenced and increments those WMEs' reference counts
            appropriately (once for each reference).

            If a production instantiation is retracted then the reference
            count is decremented.  Instantiation retractions do not affect
            references counts from previous cycles.

        C.  Activation

            At the conclusion of each cycle, the decay system reviews all
            activated WMEs.  When it encounters a WME that has been
            referenced during the cycle it recalculates that WMEs current
            activation level using a logarithmic function derived
            empirically from experiemnts in cognitive psychology (Anderson
            & Schooler 1991).  As a result of the change in activation
            level, the WME is moved to a new location in the decay
            timelist.

        D.  Decay

            Once the the decay timelist has been updated, the WMEs that
            remain in the current position in the decay timelist are
            removed from working memory (i.e., forgotten).  The associated
            wme_decay_element_struct is also deallocated.  NOTE:
            Forgetting can be turned off with the command 'decay
            -forgetting off'.

            The current position pointer in the decay timelist is
            also incremented in preparation for the next cycle.



    REFERENCES

    Anderson, J.E. & Schooler, L.J. (1991) Reflections of the environment
    in memory.  /Psychological Science, 2, 396-408.

   ======================================================================  */

/*
  Feature Wish List:
  1.  Allow user to log what productions have activated each WME (and when)
  2.  Allow user to turn off persistent activation for non-retracted
      instantiations ('decay -activation once')
  3.  I-support support: Testing an i-supported wme should activate o-supported
      WMEs that support it.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

#ifndef GSYSPARAMS_H
#include"gsysparam.h"
#endif

#include "kernel.h"
#include "agent.h"
#include "explain.h"
#include "rete.h"
#include "decide.h"
#include "prefmem.h"
#include "print.h"
#include "activate.h"

#ifdef SOAR_WMEM_ACTIVATION



//#define DECAY_DEBUG
#ifdef DECAY_DEBUG
    #define log_activation(w,x,y,z) print_with_symbols(thisAgent, w, x,y,z)
    #define log_prod_name(x,y) print(thisAgent, x,y)
#else
    #define log_activation(w, x,y,z)
    #define log_prod_name(x,y)
#endif



/* ===========================================================================
   External routines
   =========================================================================== */
void gds_invalid_so_remove_goal (agent *thisAgent, wme *w);



/* ===================================================================
   compare_int

   Compares two integers stored in void * pointers.
   (Used for qsort() calls)
   
   Created: 26 Aug 2003
   =================================================================== */
int compare_int( const void *arg1, const void *arg2 )
{
    return *((int *)arg1) - *((int *)arg2);
}

/* ============================================================================
   decay_init_quickboost(void)

   If the user selects low precision decay, then the boost received when a
   WME is referenced is based on the length of its activation history and
   the number of references it has received this cycle.  These boost values 
   pre-calculated at init time here.

   Created:  09 Sep 2003
   ========================================================================= */
#define CURR_CYCLE 10           // How old a reference in the generated history can be
#define NUM_ITER 1000           // How many iterations to average together 

void decay_init_quickboost(agent *thisAgent)
{
    int history_iter;
    float activation_level;
    float sum;
    wme_decay_element el;
    int i;
    int test_iter;
    long time_iter;
    unsigned long avg;          // used to calc average boost

    //Loop over all possible history counts
    for(el.history_count = 1;
        el.history_count < DECAY_HISTORY_SIZE;
        el.history_count++)
    {
        avg = 0;
        
        //Perform multiple tests and find the average boost
        for(test_iter = 0; test_iter < NUM_ITER; test_iter++)
        {
            //Create a random history of the required size
            for(i = 1; i < el.history_count; i++)
            {
                el.boost_history[i] = (int)(((double)rand()) / ((double)RAND_MAX))
                    * CURR_CYCLE + 1;
            }
        
            //At least one of the references will be from the current cycle
            el.boost_history[0] = CURR_CYCLE;
            el.boost_history[1] = CURR_CYCLE;
            el.boost_history[2] = CURR_CYCLE;

            //Remaining array entries receive an arbitrarily large num
            for(i = el.history_count; i < DECAY_HISTORY_SIZE; i++)
            {
                el.boost_history[i] = MAX_DECAY*2;
            }

            //Count the number of references from this cycle
            el.num_references = 0;
            for(i = 0; i < el.history_count; i++)
            {
                if (el.boost_history[i] == CURR_CYCLE)
                {
                    el.num_references++;
                }
            }

            //The boost routine expects the history to be in numerical order
            qsort( (void *)el.boost_history,
                   (size_t)DECAY_HISTORY_SIZE,
                   sizeof(int),
                   compare_int );


        
            //Calculate the amount of boost received
            time_iter = CURR_CYCLE;

            do
            {
                sum = 0;
            
                //Existing WME
                for (history_iter = 0;
                     history_iter <= (el.history_count - 1);
                     history_iter++)
                {
                    int n = time_iter - el.boost_history[history_iter] + 1;
                    if (n < DECAY_POWER_ARRAY_SIZE)
                    {
                        sum += thisAgent->decay_power_array[n];
                    }
                }
        
                activation_level = (float) log((double) sum);

                time_iter++;
        
            } while(activation_level > DECAY_ACTIVATION_CUTOFF);

            //Why -1?  because the number of references will be added to the
            //boost (num_refs*2 at creation time) in order to simulate the extra
            //boost you get for recent references.
            avg += (time_iter - CURR_CYCLE) - 1;
        
        }//for
        
        avg /= NUM_ITER;
        (thisAgent->decay_quick_boost[el.history_count]) = avg;
        
    }//for
    
}//decay_init_quickboost

/* ============================================================================
   decay_init()

   decay_init will set up the memory pool which holds the decay_elements (which
   are the elements of the linked lists at each decay_timelist position).  It
   also sets up the timelist for all positions, and sets the current pointer to
   the first of those.  When this function is used to re-init the decay
   structure, a new memory pool will be created, which should not cause problems
   except for wasted memory.  Care should be taken to make sure that all
   pointers are removed when decay is turned off, otherwise seg faults..

   Created:  23 May 2001
   ========================================================================= */

void decay_init(agent *thisAgent)
{
	int i;
	unsigned long current_time;
	decay_timelist_element *temp_timelist;
	
    start_timer(thisAgent, &(thisAgent->decay_tv));

	init_memory_pool (thisAgent, &(thisAgent->decay_element_pool),
                      sizeof(wme_decay_element), "wme_decay");
    

	current_time = (thisAgent->d_cycle_count);

	// set up the array
	for(i=0; i<DECAY_ARRAY_SIZE; i++)
    {
		temp_timelist = &(thisAgent->decay_timelist[i]);
		temp_timelist->position = i;
		temp_timelist->time = current_time + i;
		temp_timelist->first_decay_element = NIL;
	}
	
	// init the current pointer
	(thisAgent->current_decay_timelist_element) = &(thisAgent->decay_timelist[0]);
    

    // Pre-compute the integer powers of the decay exponent in order to avoid
    // repeated calls to pow() at runtime
    for(i=0;i<DECAY_POWER_ARRAY_SIZE;i++)
    {
        (thisAgent->decay_power_array)[i] = (float) pow((double) i, (double) ((thisAgent->sysparams)[WME_DECAY_EXPONENT_SYSPARAM] / DECAY_EXPONENT_DIVISOR));
    } 

    //Init the quick boost values
    decay_init_quickboost(thisAgent);

    stop_timer(thisAgent, &(thisAgent->decay_tv), &(thisAgent->total_decay_time));

}//decay_init

/* ============================================================================
   decay_deinit()

   decay_deinit will set the decay_elements for all of the wmes in the
   decay timelist to NIL, otherwise there will be dangling pointers if
   decay is turned back on.

   Created:  14 July 2005
   ========================================================================= */

void decay_deinit(agent *thisAgent)
{
    int first_spot, last_spot, i;
    wme_decay_element *remove_this;

    first_spot = (thisAgent->current_decay_timelist_element)->position;
    last_spot = (first_spot - 1 + DECAY_ARRAY_SIZE) % DECAY_ARRAY_SIZE;

    i = first_spot;
    while(i != last_spot)
    {
        remove_this = (thisAgent->decay_timelist[i]).first_decay_element;
        while(remove_this != NIL)
        {
            remove_this->this_wme->decay_element = NIL;
            remove_this->this_wme->has_decay_element = FALSE;
            //%%%maybe should free up the memory pool here?... -MRJ
            remove_this = remove_this->next;
        }
        i = (i + 1) % DECAY_ARRAY_SIZE;
    }//while

}//decay_deinit

/* ============================================================================
   decay_activation_level

   This function is provided for external use.  Given a WME, this it
   calculates an approximate activation level of that WME as an
   integeer between 0 and (DECAY_ARRAY_SIZE - 1).  The higher the
   number the more activated the WME is.  Calculating a real valued
   activation level is expensive and usually unnecessary.  This
   function provides a nice compromise.

   If the given WME does not have a decay element, this function
   returns DECAY_INT_NO_ACTIVATION.

   Created:  09 March 2004
   ========================================================================= */
int decay_activation_level(agent *thisAgent, wme *w)
{
    int wme_pos;
    int curr_pos;

    if (!w->has_decay_element)
    {
        return DECAY_INT_NO_ACTIVATION;
    }

    wme_pos = w->decay_element->time_spot->position;
    curr_pos = (thisAgent->current_decay_timelist_element)->position;

    if (wme_pos >= curr_pos)
    {
        return wme_pos - curr_pos;
    }
    else
    {
        return DECAY_ARRAY_SIZE - curr_pos + wme_pos;
    }
    
}//decay_activation_level

/* ============================================================================
   decay_activation

   This function is provided for external use.  Given a WME, this it
   calculates an EXACT activation level of that WME as an
   floating point number.  The higher the number the more activated the WME is.

   If the given WME does not have a decay element, this function
   returns DECAY_FLOAT_NO_ACTIVATION.

   Created:  10 March 2006
   ========================================================================= */
float decay_activation(agent *thisAgent, wme *w)
{
    float sum = 0.0;
    int i;
    
    if (!w->has_decay_element)
    {
        return (float)DECAY_FLOAT_NO_ACTIVATION;
    }

    //Calculate and print the activation level
    for (i = 0; i <= (w->decay_element->history_count - 1); i++)
    {
        int n = w->decay_element->time_spot->time - w->decay_element->boost_history[i] + 1;
        if (n < DECAY_POWER_ARRAY_SIZE)
        {
            sum += (thisAgent->decay_power_array)[n];
        }
    }

    return sum;
    
}//decay_activation


/* ============================================================================
   decay_reference_wme()       *RECURSIVE*

   Given a WME, this function increments its reference count.  If the WME is not
   activated (because it has i-support) then this function traces the
   given WME's preference tree to find the set of all activated WMEs that must
   exist in order for the given WME to exist.  Each of these activated WMEs is
   given a reference.


   Created:  12 August 2003
   Updated:  09 June 2006 - Added loop detection
   ========================================================================= */
void decay_reference_wme(agent *thisAgent, wme *w, int depth = 0)
{
    preference *pref = w->preference;
    instantiation *inst;
    condition *c;

    //Avoid stack overflow
    if (depth > 10) return;
    
    /*
     * Step 1:  Check for cases where referencing the WME is easy.  This should
     *          happen the majority of the time.
     */
    
    //If the WME has a decay element we can just bump the reference
    //count and return
    if (w->has_decay_element)
    {
        w->decay_element->num_references++;
        return;
    }
    //Architectural WMEs without decay elements are ignored
    else if (pref == NIL)
    {
        return;
    }
    else if (pref->o_supported == TRUE)
    {
        /*
          It's possible that there is an o-supported wme out there which does
          not have a decay_element.  Possible causes for this are:
          1.  The wme was i-supported and is just now being o-supported
          2.  Decay was turned off, and turned back on, so the wme never got a
              chance to make the decay_element
          3.  Forgetting has been disabled so the WME's decay element was
              removed but the WME was not removed from working memory.
          
          Regardless of how this happened, we need to create a new decay element
          for this WME here.
        */

        //Add a decay element to this WME
        // MRJ: The 1 in the next call is not always right. It's a
        // rare case, but maybe look at it...
        decay_update_new_wme(thisAgent, w, 1);
        return;
    }
    //If i-support mode is 'none' then we can stop here
    else if ((thisAgent->sysparams)[WME_DECAY_I_SUPPORT_MODE_SYSPARAM] == DECAY_I_SUPPORT_MODE_NONE)
    {
        return;
    }

    /*
     * Step 2:  In this case we have an i-supported WME that has been
     *          referenced.  We need to find the supporting o-supported WMEs and
     *          reference them instead.
     *
     */

    inst = pref->inst;
    c = inst->top_of_instantiated_conditions;
    while(c != NIL)
    {
        //BUGBUG: How to handle negative conditions?  Ignore for now.
        if (c->type == POSITIVE_CONDITION)
        {
            // recurse %%%has c->bt.wme_ been deprecated??%%%
            decay_reference_wme(thisAgent, c->bt.wme_, depth + 1); 
        }//if
        c = c->next;
    }//while

}//decay_reference_wme


/* ============================================================================
   dcah_helper                       *RECURSIVE*

   This function recursively discovers the activated WMEs that led to
   the creation of a given WME.  When found, their boost histories are added
   to the history in the given decay element.

   This funciton returns the number of supporting WMEs found.

   %%%BUG:  Does I need to use a transitive closure check here?
   
   Arguments:
       w        - wme to examine
       el       - decay element to fill in the boost history of

   Created:  11 Mar 2004
   ========================================================================= */
int dcah_helper(wme *w, wme_decay_element *el, int depth = 0)
{
    preference *pref = w->preference;
    instantiation *inst;
    condition *cond;
    wme *cond_wme;
    int num_cond_wmes = 0;
    int i,j;

    if (pref == NIL) return 0;
    
    //Avoid stack overflow (This is a kludge, I know)
    if (depth > 10) return 0;
    
    inst = pref->inst;
    cond = inst->top_of_instantiated_conditions;
    while(cond != NIL)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
             cond_wme = cond->bt.wme_; // %%%has bt_info_struct.wme_ been deprecated?%%%
             if (cond_wme->has_decay_element)
             {
                 if (!cond_wme->decay_element->just_created)
                 {
                     i = DECAY_HISTORY_SIZE - 1;
                     for(j = cond_wme->decay_element->history_count - 1; j >= 0; j--)
                     {
                         el->boost_history[i]
                             += cond_wme->decay_element->boost_history[j];
                         
                         i--;
                     }
                     num_cond_wmes++;
                 }
             }//if
             else
             {
                 num_cond_wmes += dcah_helper(cond_wme, el, depth +1);
             }//else
        }//if
        
        cond = cond->next;
    }//while

    return num_cond_wmes;
    
}//dcah_helper

/* ============================================================================
   decay_calculate_average_history()

   This function examines the production instantiation that led to the
   creation of a WME.  The decay history of each WME in that
   instantiation is examined and averaged.  This new average boost
   history is inserted into the given decay element.

   If the WME is not o-supported or no conditions are found that meet
   the criteria (positive conditions on wmes that have a decay
   history) then an empty history is assigned.

   Arguments:
       w        - wme to examine
       el       - decay element to fill in the boost history of

   Created:  11 Mar 2004
   ========================================================================= */
void decay_calculate_average_history(wme *w, wme_decay_element *el)
{
    preference *pref = w->preference;
    int i;
    int num_cond_wmes = 0;      //Number of wmes in the preference
    
    el->history_count = 0;
    if (pref == NIL) return;

    for(i = 0; i < DECAY_HISTORY_SIZE; i++)
    {
        el->boost_history[i] = 0;
    }

    num_cond_wmes = dcah_helper(w, el);

    if (num_cond_wmes > 0)
    {
        //Calculate the average
        for(i = 0; i < DECAY_HISTORY_SIZE; i++)
        {
            el->boost_history[i] /= num_cond_wmes;
        }

        //Determine the actual length of the history
        for(i = DECAY_HISTORY_SIZE - 1; i >= 0; i--)
        {
            if (el->boost_history[i] > 0) el->history_count++;
        }

        //Compress the array values into the left hand side of the array
        for(i = 0; i < el->history_count; i++)
        {
            int gap = DECAY_HISTORY_SIZE - el->history_count;
            el->boost_history[i] = el->boost_history[i + gap];
        }
    }//if

}//decay_calculate_average_history()

/* ============================================================================
   decay_update_new_wme()

   This function adds a decay element to an existing WME.  It is called whenever
   a wme is discovered that does not have a decay element.
   (Usually this is at wme creation time.)

   Arguments:
       w        - wme to add the decay element to
       num_refs - how many times this wme has been referenced to date.  This
                  number must be greater than zero.

   Created:  23 May 2001
   ========================================================================= */
 
void decay_update_new_wme(agent *thisAgent, wme *w, int num_refs)
{
    wme_decay_element *temp_el;
    int bActivate;      // should this be an activated wme?

    start_timer(thisAgent, &(thisAgent->decay_tv));
    start_timer(thisAgent, &(thisAgent->decay_new_wme_tv));

    /*
     * Step 1: Verify that this WME meets the criteria for being an activated
     *         wme
     */
    bActivate = TRUE;
    switch((thisAgent->sysparams)[WME_DECAY_WME_CRITERIA_SYSPARAM])
    {
        case DECAY_WME_CRITERIA_ALL:
            break;
        case DECAY_WME_CRITERIA_O_ARCH:
            if ( (w->preference != NIL)
                 && (w->preference->o_supported != TRUE) )
            {
                bActivate = FALSE;
            }

            break;
        case DECAY_WME_CRITERIA_O_SUPPORT_ONLY:
            if ( (w->preference == NIL)
                 || (w->preference->o_supported != TRUE) )
            {
                bActivate = FALSE;
            }
            break;
    }//switch

    if (! bActivate)
    {
        //However, the creation of an i-supported WME may activate the WMEs that
        //led to its creation.
        if ( (w->preference != NIL) && (w->preference->o_supported != TRUE) )
        {
            switch((thisAgent->sysparams)[WME_DECAY_I_SUPPORT_MODE_SYSPARAM])
            {
                case DECAY_I_SUPPORT_MODE_NONE:
                case DECAY_I_SUPPORT_MODE_NO_CREATE:
                    break;
                case DECAY_I_SUPPORT_MODE_UNIFORM:
                    decay_reference_wme(thisAgent, w);
                    break;
            }
        }
        
        stop_timer(thisAgent, &(thisAgent->decay_tv), &(thisAgent->total_decay_time));
        stop_timer(thisAgent, &(thisAgent->decay_new_wme_tv), &(thisAgent->total_decay_new_wme_time));
        
        return;
    }//if
                
            


    //If the wme already has a decay element return.
    if (w->decay_element != NIL)
    {
        stop_timer(thisAgent, &(thisAgent->decay_tv), &(thisAgent->total_decay_time));
        stop_timer(thisAgent, &(thisAgent->decay_new_wme_tv), &(thisAgent->total_decay_new_wme_time));

        return;
    }

    
    /*
     * Step 2:  Allocate and initialize a new decay element for the WME
     */
    allocate_with_pool(thisAgent, &(thisAgent->decay_element_pool), &temp_el);
    temp_el->just_created = TRUE;
    temp_el->just_removed = FALSE;
    temp_el->history_count = 0;
    temp_el->this_wme = w;
    temp_el->num_references = num_refs;

    //Give the WME an initial history based upon the WMEs that were
    //tested to create it.
    decay_calculate_average_history(w, temp_el);

    //Initialize the activation log
    if ((thisAgent->sysparams)[WME_DECAY_LOGGING_SYSPARAM])
    {
        temp_el->log = (decay_log *)allocate_memory (thisAgent,
                                                     sizeof(decay_log),
                                                     MISCELLANEOUS_MEM_USAGE);
        temp_el->log->act_log = NIL;
        temp_el->log->size = 0;
        temp_el->log->start = -1; // flag value
    }
    else
    {
        temp_el->log = NIL;
    }

    //Insert at the current position in the decay timelist
    //It will be boosted out of this position at the next update
    temp_el->next = (thisAgent->current_decay_timelist_element)->first_decay_element;
    temp_el->previous = NIL;
    temp_el->time_spot = (thisAgent->current_decay_timelist_element);
    if(temp_el->next != NIL)
    {
        temp_el->next->previous = temp_el;
    }
    (thisAgent->current_decay_timelist_element)->first_decay_element = temp_el;

    //Attach it to the wme
    w->decay_element = temp_el;
    w->has_decay_element = TRUE;

    stop_timer(thisAgent, &(thisAgent->decay_tv), &(thisAgent->total_decay_time));
    stop_timer(thisAgent, &(thisAgent->decay_new_wme_tv), &(thisAgent->total_decay_new_wme_time));
    
    
}//decay_update_new_wme



/* ============================================================================
   activate_wmes_in_pref()

   This routine boosts the activation of all WMEs in a given preference

   Arguments:
       pref - the preference to examine

   NOTE:  As far as I can tell, this function never encounters a
          preference that meets the criteria for a reference increment.
          Nothing seems to get past: "if(w->value == pref->value)".  What is
          this code for? -:AMN: 
       
   Created:  04 Aug 2003
   ========================================================================= */
void activate_wmes_in_pref(agent *thisAgent, preference *pref)
{
    wme *w;
    
    // I have the recreated code here instead of a seperate function so that
    // all newly_created_insts are picked up.
    if ((pref->type != REJECT_PREFERENCE_TYPE) && 
        (pref->type != PROHIBIT_PREFERENCE_TYPE) &&
        (pref->slot != NIL))
    {
        w = pref->slot->wmes;
        while (w)
        {
            // id and attr should already match so just compare the value
            if(w->value == pref->value)
            {
                log_activation("\nNew Pref:    (%y ^%y %y) ",
                               w->id, w->attr, w->value);
                decay_reference_wme(thisAgent, w);
            }//if
            w = w->next;
        }//while
    }//if

}//activate_wmes_in_pref

/* ============================================================================
   activate_wmes_in_inst()

   This routine boosts the activation of all WMEs in a given production
   instantiation. 

   Arguments:
       inst - the instantiation to examine

   NOTE:  Currently this is only called by the chunker when a new chunk or
   justification is created.

   Created:  04 Aug 2003
   ========================================================================= */
void activate_wmes_in_inst(agent *thisAgent, instantiation *inst)
{
    condition *cond;
      
    for (cond=inst->top_of_instantiated_conditions;
         cond!=NIL;
         cond=cond->next)
    {
        if (cond->type==POSITIVE_CONDITION)
        {
            log_activation("\nNew Inst:    (%y ^%y %y) ",
                           cond->bt.wme_->id,
                           cond->bt.wme_->attr,
                           cond->bt.wme_->value);
            log_prod_name("%s", inst->prod->name->var.name);

            decay_reference_wme(thisAgent, cond->bt.wme_);  //%%%deprecated?%%%
        }
    }
    
}//activate_wmes_in_inst

/* ============================================================================
   decay_update_wmes_tested_in_prods()

   This function scans all the match set changes and updates the reference count
   for affected WMEs.

   Created:  23 May 2001
   ========================================================================= */

void decay_update_wmes_tested_in_prods(agent *thisAgent)
{

     ms_change *msc;
     token temp_token, *t;
     instantiation *inst;
     condition *cond;
  
     start_timer(thisAgent, &(thisAgent->decay_tv));
     start_timer(thisAgent, &(thisAgent->decay_lhs_tv));

     for (msc=(thisAgent->ms_o_assertions); msc!=NIL; msc=msc->next)
     {
         temp_token.parent = msc->tok;
         temp_token.w = msc->w;
         t = &temp_token;

         while (!(t == (thisAgent->dummy_top_token)))
         {
             if (t->w != NIL)
             {
                 log_activation("\nMS O Assert: (%y ^%y %y) ",
                                t->w->id, t->w->attr, t->w->value);
                 decay_reference_wme(thisAgent, t->w);
             }             
             t = t->parent;
         }//while
     }//for

     for (msc=(thisAgent->ms_i_assertions); msc!=NIL; msc=msc->next)
     {
         temp_token.parent = msc->tok;
         temp_token.w = msc->w;
         t = &temp_token;
	
         while (!(t == (thisAgent->dummy_top_token)))
         {
             if (t->w != NIL)
             {
                 log_activation("\nMS I Assert: (%y ^%y %y) ",
                                t->w->id, t->w->attr, t->w->value);
                 decay_reference_wme(thisAgent, t->w);
             }
             
             t = t->parent;
         }//while
     }//for

     //If instantiations do not persistently activate WMEs then there is no need
     //to decrement the reference count for retractions. :AMN: 12 Aug 2003
     if ((thisAgent->sysparams)[WME_DECAY_PERSISTENT_ACTIVATION_SYSPARAM])
     {
         for (msc=(thisAgent->ms_retractions); msc!=NIL; msc=msc->next)
         {
             inst = msc->inst;
             temp_token.w = msc->w;
             t = &temp_token;

#ifdef AMN_MONITOR
             //Why isn't this in the above two loops as well?  Because those
             //ms-change structs don't have an associated preference.
             if (inst->prod->type != MONITOR_PRODUCTION_TYPE)
#endif
             for (cond=inst->top_of_instantiated_conditions;
                  cond!=NIL;
                  cond=cond->next)
             {

                 //If a wme's existence caused an instance to cease to match (due to
                 //a negative condition in that instance) then we don't want to
                 //decrement the reference count on the WME because the WME's
                 //reference count was never incremented.
                 if (cond->type==POSITIVE_CONDITION)
                 {
                     if (cond->bt.wme_->decay_element != NIL) //%%%bt.wme_ deprecated??%%%
                     {
                         log_activation("\nMS  Retract: (%y ^%y %y) ",
                                            cond->bt.wme_->id,
                                            cond->bt.wme_->attr,
                                            cond->bt.wme_->value);
                         log_prod_name("%s", inst->prod->name->var.name);
                     
                         cond->bt.wme_->decay_element->num_references--; //%%%bt.wme_ deprecated??%%%
                     } 
                 }
             }//for
         }//for
     }//if
     
     stop_timer(thisAgent, &(thisAgent->decay_tv), &(thisAgent->total_decay_time));
     stop_timer(thisAgent, &(thisAgent->decay_lhs_tv), &(thisAgent->total_decay_lhs_time));
  
 }//decay_update_wmes_tested_in_prods


/* ============================================================================
   decay_update_wmes_in_retracted_inst

   This code is detecting production retractions that affect activated WMEs and
   decrementing their reference counts.

   Created:  23 May 2001
   ========================================================================= */
void decay_update_wmes_in_retracted_inst(agent *thisAgent, instantiation *inst)
{
    wme *w;
    preference *pref, *next;

    
    start_timer(thisAgent, &(thisAgent->decay_tv));
    start_timer(thisAgent, &(thisAgent->decay_rhs_tv));

    if ( ((thisAgent->sysparams)[WME_DECAY_SYSPARAM])
         && ((thisAgent->sysparams)[WME_DECAY_PERSISTENT_ACTIVATION_SYSPARAM]) )
    {

#ifdef AMN_MONITOR
        if (inst->prod->type != MONITOR_PRODUCTION_TYPE)
#endif
        for (pref=inst->preferences_generated; pref!=NIL; pref=next)
        {
            next = pref->inst_next;
            
            if ((pref->type != REJECT_PREFERENCE_TYPE) && 
                (pref->type != PROHIBIT_PREFERENCE_TYPE) &&
                (pref->o_supported) &&
                (pref->slot != NIL))
            {
                //found an o-supported pref
                w = pref->slot->wmes;

                while (w)
                {
                    // id and attr should already match...
                    if(w->value == pref->value)
                    {
                        //we got a match with an existing wme
                        if (w->decay_element != NIL)
                        {
                            log_activation("\nInst Retrac: (%y ^%y %y) ",
                                               w->id, w->attr, w->value);
                            log_prod_name("%s", inst->prod->name->var.name);
                            w->decay_element->num_references--;
                        }
                    }
                    w = w->next;
                } 
            }//if
        }//for
    }//if

    stop_timer(thisAgent, &(thisAgent->decay_tv), &(thisAgent->total_decay_time));
    stop_timer(thisAgent, &(thisAgent->decay_rhs_tv), &(thisAgent->total_decay_rhs_time));

}//decay_update_wmes_in_retracted_inst()

/* ============================================================================
   forget_wme_due_to_decay()

   This routine removes an activated WME from working memory and performs all
   the necessary cleanup related to that removal.  This function is intended
   to be called when the WME's activation level has dropped below the
   DECAY_ACTIVATION_CUTOFF (defined in soarkernel.h).

   NOTE: A different way to do this might be to assert a remove preference here
         so that the wme is removed during assert_preferences.
         
   Created:  05 August 2003
   ========================================================================= */
void forget_wme_due_to_decay(agent *thisAgent, wme *w)
{
    wme *w2;
    slot *s;
    Symbol *id;
    preference *p;
    
    //Check for the sysparam that allows actual WME removal.  If it's set to
    //FALSE then the WME is left undisturbed in working memory and only the
    //its decay information is removed.  A new decay element will be created
    //for the WME if it is subsequently referenced.
    if ( ! (thisAgent->sysparams)[WME_DECAY_ALLOW_FORGETTING_SYSPARAM])
    {
        decay_remove_element(thisAgent, w);
        return;
    }
  
    id = w->id;
      
    /* what lists will w be on?  acceptable preferences?? */
    for (s=id->id.slots; s!=NIL; s=s->next)
    {
        for (w2=s->wmes; w2!=NIL; w2=w2->next)
        {
            if (w==w2) break;
        }
          
        if (w2)
        {
            remove_from_dll (s->wmes, w, next, prev);
        }
          
        for (w2=s->acceptable_preference_wmes; w2!=NIL; w2=w2->next)
        {
            if (w==w2)
            {
                remove_from_dll (s->acceptable_preference_wmes, w, next, prev); 
                break;
            }
        }
          
        // %%%once a preference is made into a wme, is it removed from the
        // preference lists in that slot, or kept there?  If kept, it must
        // be removed here. -MRJ

        // %%%do we need to deallocate this pref, or is that taken care of
        // automatically? -MRJ
               
        for(p=s->all_preferences; p!=NIL; p = p->all_of_slot_next)
        {
            // if p matches this wme, remove p...
            if( (p->id == w->id)
                && (p->attr == w->attr)
                && (p->value == w->value))
            {
                remove_preference_from_tm(thisAgent, p);
            }
        }
    }//for
          
    /* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
    if ((thisAgent->operand2_mode))
    {
#endif
        if (w->gds)
        {
            if (w->gds->goal != NIL)
            {
                gds_invalid_so_remove_goal(thisAgent, w); 
            }
        }
#ifndef SOAR_8_ONLY
    }
#endif

    remove_wme_from_wm(thisAgent, w);

/* NOTE: decay_deactivate_element() and/or decay_remove_element() do not need to
   be called here. They will be called by the kernel when the wme struct is
   actually removed from WM or from RAM (respectively).  */
      
}//forget_wme_due_to_decay()

/* ============================================================================
   decay_reposition_wme()

   This function repositions a decay element in the decay timelist.

   Arguments:
       cur_decay_el - the decay element to reposition
       decay_spot - where to move it to

   Created:  05 August 2003
   ========================================================================= */
void decay_reposition_wme(agent *thisAgent,
                          wme_decay_element *cur_decay_el,
                          long decay_spot)
{
    // remove the current decay element whose decay spot we've just
    // calculated from the decay timelist in preparation for moving
    // it to its new location (see next code block)
    if(cur_decay_el->previous == NIL)
    {
        // it is first in the decay list so set the first element to
        // the next one (or NIL)
        cur_decay_el->time_spot->first_decay_element = cur_decay_el->next;
        // if there is a next element (now the first), set it's previous to NIL
        if(cur_decay_el->next != NIL)
        {
            cur_decay_el->next->previous = NIL;
        }
    }
    else
    {
        // it is not first (so will have a previous for sure)
        cur_decay_el->previous->next = cur_decay_el->next;
        if(cur_decay_el->next != NIL)
        {
            cur_decay_el->next->previous = cur_decay_el->previous;
        }
    }
    
    
    
    // Insert the current decay element in its new location
    cur_decay_el->next = (thisAgent->decay_timelist[decay_spot]).first_decay_element;
    cur_decay_el->previous = NIL;
    cur_decay_el->time_spot = &(thisAgent->decay_timelist[decay_spot]);
    
    // set up next element's previous pointer
    if(cur_decay_el->next != NIL)
    {
        cur_decay_el->next->previous = cur_decay_el;
    }
    
    // set up the first_decay_element for this time slot
    //(just insert it first since order doesn't matter)
    (thisAgent->decay_timelist[decay_spot]).first_decay_element = cur_decay_el;

}//decay_reposition_wme()

/* ============================================================================
   decay_print_boost_info()            *DEBUGGING*

   This function prints information about the results of a WME boost

   w         - the WME in question
   prev_spot - the element's previous spot in the decay timelist
   new_spot  - the element's new spot in the decay timelist

   Created:  26 August 2003
   ========================================================================= */
void decay_print_boost_info(agent *thisAgent,
                            wme *w,
                            long prev_spot,
                            long new_spot)
{
    int i;

    print_with_symbols(thisAgent,
                       "\nBoosting WME: (%y ^%y %y) boost history: (",
                       w->id, w->attr, w->value);

    for(i = 0; i < w->decay_element->history_count; i++)
    {
        print(thisAgent, "%ld,", w->decay_element->boost_history[i]);
    }

    print(thisAgent, ") #ref=%ld from %ld to %ld (%ld)",
          w->decay_element->num_references,
          prev_spot, new_spot, new_spot - prev_spot);
}//decay_print_boost_info

/* ============================================================================
   decay_log_boost

   This function logs a given activation level at a given cycle.

   Created:  02 September 2003
   ========================================================================= */
void decay_log_boost(agent *thisAgent,
                     wme_decay_element *el,
                     float act,
                     int cycle)
{
    int i;

    //The activation at the time of the first cycle is infinite.  To make
    //it easier to graph, this value is ignored.
    if (act > 1.0) return;
    
    //If the activation log array is too full, realloc it
    if (cycle - el->log->start >= el->log->size)
    {
        int new_size;
        float *new_act_log;
        
        new_size = el->log->size*2 + DECAY_ACTIVATION_LOG_SIZE;
        
        new_act_log = (float *)allocate_memory(thisAgent,
                                               sizeof(float)*new_size,
                                               MISCELLANEOUS_MEM_USAGE);

        //Is this is the first time this log has been used?
        if (el->log->start == -1)
        {
            //Record the start cycle
            el->log->start = cycle;
        }
        else
        {
            //Copy the old values to the new array
            for(i = 0; i < el->log->size; i++)
            {
                new_act_log[i] = el->log->act_log[i];
            }
        }

        //Free the old log
        if (el->log->act_log != NIL)
        {
            free_memory(thisAgent, el->log->act_log, MISCELLANEOUS_MEM_USAGE);
        }

        el->log->act_log = new_act_log;
        el->log->size = new_size;
    }//if

    el->log->act_log[cycle - el->log->start] = act;
    
}//decay_log_boost

/* ============================================================================
   decay_add_refs_to_history

   This function adds N refs at the previous cycle to the boost history
   of a given decay element.

   CAVEAT: Why previous cycle instead of current?  It's a kludge.
   This function is only called by decay_boost_wme() which is called
   at the end of the cycle but after the cycle count has been
   incremented.  So I compensate here.
   
   NOTE: that this code will allow multiple boosts for the same cycle
   (Ron Chong's code only allows one boost per cycle)

   Arguments:
       el       - decay element to fill in the boost history of
       num_refs - how many references to add

   Created:  11 Mar 2004
   ========================================================================= */
void decay_add_refs_to_history(agent *thisAgent,
                               wme_decay_element *el,
                               int num_refs)
{
    int i;
    int move_by;
    
    if (num_refs > DECAY_HISTORY_SIZE)
    {
        num_refs = DECAY_HISTORY_SIZE;
    }
    
    if ((el->history_count + num_refs) > DECAY_HISTORY_SIZE)
    {
        //Shift some references out of the array to make room for the new ones.
        move_by = el->history_count + num_refs - DECAY_HISTORY_SIZE;

        for (i = 0; i < (DECAY_HISTORY_SIZE - num_refs); i++)
        {
            el->boost_history[i] = el->boost_history[i + move_by];
        }
        
        for(i=(DECAY_HISTORY_SIZE - num_refs);
            i < DECAY_HISTORY_SIZE;
            i++)
        {
            el->boost_history[i] = (thisAgent->d_cycle_count) - 1;
        }
        
        el->history_count = DECAY_HISTORY_SIZE;

    }
    else
    {
        for(i=(el->history_count);
            i < (el->history_count + num_refs);
            i++)
        {
            el->boost_history[i] = (thisAgent->d_cycle_count) - 1;
        }
        el->history_count += num_refs;
    }
    
}//decay_add_refs_to_history

/* ============================================================================
   decay_boost_wme()

   This function calculates the new position in the decay timelist for a decay
   element when its associated WME is referenced.

   Return Value:  The new position for this wme in the decay timelist array

   Created:  05 August 2003
   ========================================================================= */
long decay_boost_wme(agent *thisAgent, wme_decay_element *cur_decay_el)
{
    long decay_spot;
    long time_iter;
    float sum, activation_level;
    int history_iter;

    /*
     * Step 1: Update the boost history
     *
     */
    //For new WMEs, we need to make sure the ref count is nonzero.
    if ( (cur_decay_el->just_created) && (cur_decay_el->num_references < 1) )
    {
        cur_decay_el->num_references = 1;
    }

    decay_add_refs_to_history(thisAgent, cur_decay_el, cur_decay_el->num_references);

    /*
     * Step 2:  Calculate the new position in the decay timelist
     */

    // start at the current time_spot for this wme, because the
    // new time spot can't be lower than the current.
    time_iter = cur_decay_el->time_spot->time; 
    
    if ((thisAgent->sysparams)[WME_DECAY_PRECISION_SYSPARAM]
        == DECAY_PRECISION_LOW)
    {
        time_iter += (thisAgent->decay_quick_boost)[cur_decay_el->history_count];
        time_iter += cur_decay_el->num_references;

        //%%%Log activation.  How??
    }
    else
    {
        do
        {
            sum = 0;

            for (history_iter = 0;
                 history_iter <= (cur_decay_el->history_count - 1);
                 history_iter++)
            {
                int n = time_iter - cur_decay_el->boost_history[history_iter] + 1;
                if (n < DECAY_POWER_ARRAY_SIZE)
                {
                    sum += (thisAgent->decay_power_array)[n];
                }
            }
        
            activation_level = (float) log((double) sum);

            //If the log is turned on, log the new activation level
            if ((thisAgent->sysparams)[WME_DECAY_LOGGING_SYSPARAM])
            {
                decay_log_boost(thisAgent, cur_decay_el, activation_level, time_iter);
            }
        
            time_iter++;
        } while(activation_level > DECAY_ACTIVATION_CUTOFF);
    }//else
    
    //time_iter is the cycle when the wme will be below the
    //decay threshold so remove the wme at the end of the cycle
    //just before that.
    decay_spot = time_iter - 1;
    
    // calculate what position it should go to
    if(decay_spot > ((thisAgent->current_decay_timelist_element)->time + MAX_DECAY))
    {
        decay_spot = ((thisAgent->current_decay_timelist_element)->position + MAX_DECAY) % DECAY_ARRAY_SIZE;
    }
    else
    {
        decay_spot = decay_spot % DECAY_ARRAY_SIZE;
    }

    /*
     * Step 0 revisited:  More special handling for just created WMEs
     */
    if (cur_decay_el->just_created)
    {
        cur_decay_el->just_created = FALSE;

        /* AMN: 25 May 2003 WMEs that are architectural (e.g., input-link,
           ^superstate nil) need to have their num_references value reset.
           Otherwise the system will behave as if the WMEs have been referenced
           at every cycle.
         */
        if (cur_decay_el->this_wme->preference == NULL)
        {
            cur_decay_el->num_references = 0;
        }
    }//if
    
    return decay_spot;

}//decay_boost_wme

/* ===================================================================
   print_most_activated_wmes   *DEBUGGING*

   This function prints the most activated WMEs in working memory.
   The user specifies how many tiers of activated WMEs to print.
   
   
   Created: 30 Apr 2003
   Updated: 06 Oct 2003 - moved from epmem.c to activate.c
   =================================================================== */
void decay_print_most_activated_wmes(agent *thisAgent, int n)
{
    decay_timelist_element *decay_list;
    int decay_pos;
    wme_decay_element *decay_element;
    int i;
    float sum = 0;
    int history_iter;
    char act_buf[512];

    print(thisAgent, "\nBEGIN ACTIVATED WME LIST\n");

    decay_list = (thisAgent->decay_timelist);
    decay_pos = (thisAgent->current_decay_timelist_element)->position;

    /* Traverse the decay array backwards in order to get the most
       activated wmes first */
    for(i = 0; i < n; i++)
    {
        char buf[32];
        sprintf(buf, "%.2d     ", i);
        
        decay_pos = decay_pos > 0 ? decay_pos - 1 : MAX_DECAY - 1;
        
        if (decay_list[decay_pos].first_decay_element != NULL)
        {
            decay_element = decay_list[decay_pos].first_decay_element;
            while (decay_element != NULL)
            {
                print(thisAgent, buf);

                sprintf(act_buf, "(%d: ", decay_element->this_wme->timetag);
                print(thisAgent, act_buf);
                print_with_symbols(thisAgent,
                                   "%y ^%y %y", 
                                   decay_element->this_wme->id,
                                   decay_element->this_wme->attr,
                                   decay_element->this_wme->value);
                sprintf(act_buf, ")      num refs: %d      activation: ",
                        decay_element->num_references);
                print(thisAgent, act_buf);


                //Calculate and print the activation level
                sum = 0;
                for (history_iter = 0; history_iter <= (decay_element->history_count - 1); history_iter++)
                {
                    int n = decay_element->time_spot->time- decay_element->boost_history[history_iter] + 1;
                    if (n < DECAY_POWER_ARRAY_SIZE)
                    {
                        sum += (thisAgent->decay_power_array)[n];
                    }
                }
                sprintf(act_buf, "%f\n", sum);
                print(thisAgent, act_buf);

                
                decay_element = decay_element->next;
            }
        }//if

        printf("\n");
    }//for

    print(thisAgent, "END OF ACTIVATED WME LIST");

}//print_most_activated_wmes



/* ============================================================================
   decay_move_and_remove_wmes(void)

   This function is called at the end of each cycle to boost WMEs that have been
   referenced this cycle and remove WMEs that have been forgotten due to decay.

   NOTE:  The bulk of the work done by the decay system is done in this routine
   (or one of its subroutines).

   Created:  23 May 2001
   ========================================================================= */
void decay_move_and_remove_wmes(agent *thisAgent)
{
    wme_decay_element *cur_decay_el, *next;
    int array_iter, array_position, start_position;
    long decay_spot;        // New position for the wme in the decay timelist

    start_timer(thisAgent, &(thisAgent->decay_tv));
    start_timer(thisAgent, &(thisAgent->decay_move_remove_tv));


    /*
     * Step 1:  Reposition all WMEs have have been created or referenced this
     *          cycle
     */

    // need to start at the last time in the decay timelist and work backwards...
    start_position = (thisAgent->current_decay_timelist_element)->position;

    //loop over all positions in the decay timelist
    for(array_iter=0; array_iter<DECAY_ARRAY_SIZE; array_iter++)
    {
        // Find position in the array for this iteration (starting 1 back from
        // current) 
        array_position = (start_position - array_iter + MAX_DECAY) % DECAY_ARRAY_SIZE;
        cur_decay_el = (thisAgent->decay_timelist[array_position]).first_decay_element;

        //loop over all decay elemnts at this position in the timelist
        while(cur_decay_el != NIL)
        {
            next = cur_decay_el->next;

            if((cur_decay_el->num_references > 0) || (cur_decay_el->just_created))
            {
                decay_spot = decay_boost_wme(thisAgent, cur_decay_el);
                decay_reposition_wme(thisAgent, cur_decay_el, decay_spot);

                //If we are *not* using persistent activation then the reference
                //count should be set to zero here to prevent subsequent
                //activation. :AMN: 12 Aug 2003
                if (! ((thisAgent->sysparams)[WME_DECAY_PERSISTENT_ACTIVATION_SYSPARAM]) )
                {
                    cur_decay_el->num_references = 0;
                }

                
            }//if

            cur_decay_el = next;
        }//while (loop over all decay elemnts at one position in the timelist)
    }//for (loop over all positions in the decay timelist)
 


    /*
     * Step 2: Removes all the WMEs that are still at the current spot in the
     *         decay timelist.  This is the actual forgetting mechanism
     *         associated with decay. 
     */
    cur_decay_el = (thisAgent->current_decay_timelist_element)->first_decay_element;
    while(cur_decay_el != NIL)
    {
        next = cur_decay_el->next; // save this pointer before it is deallocated
        forget_wme_due_to_decay(thisAgent, cur_decay_el->this_wme);
        cur_decay_el = next;
    }//while
  
    // Update working memory with all the WME removals that were done in the
    // loop above.  This has to be done before changing first_decay_element at
    // current time, otherwise it will cause pointer problems.
    do_buffered_wm_and_ownership_changes(thisAgent); 
  
    // update position that just had removals
    (thisAgent->current_decay_timelist_element)->time = 
        (thisAgent->current_decay_timelist_element)->time + DECAY_ARRAY_SIZE;
  
    // Mark the current position in the array as empty.  Note that if
    // remove_wme_from_wm() doesn't work right, this next line could cause
    // memory leaks
    (thisAgent->current_decay_timelist_element)->first_decay_element = NIL;

    // Update current position in the array.
    (thisAgent->current_decay_timelist_element) = 
        &((thisAgent->decay_timelist[((thisAgent->current_decay_timelist_element)->position + 1) % DECAY_ARRAY_SIZE]));

    //%%%UNCOMMENT TO DEBUG
    //decay_print_most_activated_wmes(MAX_DECAY);
    
    stop_timer(thisAgent, &(thisAgent->decay_tv), &(thisAgent->total_decay_time));
    stop_timer(thisAgent, &(thisAgent->decay_move_remove_tv), &(thisAgent->total_decay_move_remove_time));

}//decay_move_and_remove_wmes

/* ============================================================================
   decay_write_log()

   This function uses the decay log to calculate the activation level of
   a given wme at every cycle that it has existed.  This info is written
   to the current decay log file.

   Created:  02 September 2003
   ========================================================================= */
void decay_write_log(agent *thisAgent, wme *w)
{
    FILE *f;                    // log file handle 
    wme_decay_element *el;      // w->decay_element
    int i;

    // init vars
    el = w->decay_element;

    //Catch WMEs with no log info
    if ( (el->log == NULL) || (el->log->start == -1) )
    {
        return;
    }

    //Verify we have a valid log file handle and start using it
    f = (FILE *)((thisAgent->sysparams)[WME_DECAY_LOGGING_SYSPARAM]);
    if (f == 0) return;
    start_redirection_to_file(thisAgent, f);

    //Print the WME to the log file (including time tag)
    print(thisAgent, "(%ul: ", w->timetag);
    print_with_symbols(thisAgent, "%y ^%y %y)\t", w->id, w->attr, w->value);

    //Print a baseline activation for all cycles before this WME existed
    for(i = 1; i < el->log->start; i++)
    {
        fprintf(f, "%f\t", DECAY_ACTIVATION_CUTOFF);
    }

    //Print the activation for every previous cycle that this WME has existed
    for(i = el->log->start;
        i < el->time_spot->time;
        i++)
    {
        print(thisAgent, "%f\t", el->log->act_log[i - el->log->start]);
    }//for

    //Print the current activation level
    print(thisAgent, "%f\n", el->log->act_log[el->time_spot->time - el->log->start]);
    
    stop_redirection_to_file(thisAgent);        
    
}//decay_write_log

/* ============================================================================
   decay_deactivate_element()

   This routine marks a decay element as being attached to a wme struct that has
   been removed from working memory.  When the wme struct is actually
   deallocated then the decay_remove_element() routine is called.

   Created:  06 Oct 2003
   ========================================================================= */
void decay_deactivate_element(agent *thisAgent, wme *w)
{
    start_timer(thisAgent, &(thisAgent->decay_tv));
    start_timer(thisAgent, &(thisAgent->decay_deallocate_tv_2));
    start_timer(thisAgent, &(thisAgent->decay_deallocate_tv));

    //Make sure this wme has an element and that element has not already been
    //deactivated
    if ( (! w->has_decay_element) || (w->decay_element->just_removed) )
    {
        return;
    }
    
    //If the log is turned on, write the activation history to the logfile
    if ((thisAgent->sysparams)[WME_DECAY_LOGGING_SYSPARAM])
    {
        decay_write_log(thisAgent, w);
    }

    //Remove the decay element from the decay timelist
	if(w->decay_element->previous == NIL)
    {
        // if it is the first in a list, set that to the next
		w->decay_element->time_spot->first_decay_element = w->decay_element->next;
	}
    else
    {
        // otherwise remove the decay_element from the list it is on and free
        // memory
		w->decay_element->previous->next = w->decay_element->next;
    }

    if(w->decay_element->next != NIL)
    {
        // if the element has a next update prev -> next
		w->decay_element->next->previous = w->decay_element->previous;
    }

    w->decay_element->next = NIL;
    w->decay_element->previous = NIL;
    w->decay_element->just_removed = TRUE;
    
    stop_timer(thisAgent, &(thisAgent->decay_tv), &(thisAgent->total_decay_time));
    stop_timer(thisAgent, &(thisAgent->decay_deallocate_tv), &(thisAgent->total_decay_deallocate_time));
    stop_timer(thisAgent, &(thisAgent->decay_deallocate_tv_2), &(thisAgent->total_decay_deallocate_time_2));

}//decay_deactivate_element


/* ============================================================================
   decay_remove_element()

   This routine deallocates the decay element attached to a given WME.

   Created:  23 May 2001
   ========================================================================= */
void decay_remove_element(agent *thisAgent, wme *w)
{

    start_timer(thisAgent, &(thisAgent->decay_tv));
    start_timer(thisAgent, &(thisAgent->decay_deallocate_tv_2));
    start_timer(thisAgent, &(thisAgent->decay_deallocate_tv));

    //Make sure this wme has an element and that element has not already been
    //deactivated
    if ( (! w->has_decay_element) || (w->decay_element->just_removed) )
    {
        return;
    }

    //Deactivate the wme first
    decay_deactivate_element(thisAgent, w);

    //Remove the log struct if it exists
    if (w->decay_element->log != NIL)
    {
        if (w->decay_element->log->act_log != NIL)
        {
            free_memory(thisAgent, w->decay_element->log->act_log, MISCELLANEOUS_MEM_USAGE);
        }

        free_memory(thisAgent, w->decay_element->log, MISCELLANEOUS_MEM_USAGE);
    }
   
	free_with_pool (&(thisAgent->decay_element_pool), w->decay_element);

	w->has_decay_element = FALSE;
	w->decay_element = NIL;

    stop_timer(thisAgent, &(thisAgent->decay_tv), &(thisAgent->total_decay_time));
    stop_timer(thisAgent, &(thisAgent->decay_deallocate_tv), &(thisAgent->total_decay_deallocate_time));
    stop_timer(thisAgent, &(thisAgent->decay_deallocate_tv_2), &(thisAgent->total_decay_deallocate_time_2));

}//decay_remove_element


#endif //SOAR_WMEM_ACTIVATION
