/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* utilities.h */

#ifndef UTILITIES_H
#define UTILITIES_H

#include <list>
#include "stl_support.h"

////////////////////////////////
// Returns a list of wmes that share a specified id
// The tc number of the id is checked against the tc number passed in
// If they match, then NULL is returned
// Otherwise the tc number of the id is set to the specified tc number
// To guarantee any existing wmes will be returned, use get_new_tc_num()
//  for the tc parameter
////////////////////////////////
extern SoarSTLWMEPoolList* get_augs_of_id(agent* thisAgent, Symbol * id, tc_number tc);

/*
*	This procedure parses a string to determine if it is a
*      lexeme for an identifier or context variable.
* 
*      Many interface routines take identifiers as arguments.  
*      These ids can be given as normal ids, or as special variables 
*      such as <s> for the current state, etc.  This routine reads 
*      (without consuming it) an identifier or context variable, 
*      and returns a pointer (Symbol *) to the id.  (In the case of 
*      context variables, the instantiated variable is returned.  If 
*      any error occurs (e.g., no such id, no instantiation of the 
*      variable), an error message is printed and NIL is returned.
*
* Results:
*	Pointer to a symbol for the variable or NIL.
*
* Side effects:
*	None.
*
===============================
*/
extern bool read_id_or_context_var_from_string (agent* agnt, const char * the_lexeme, Symbol * * result_id);
extern void get_lexeme_from_string (agent* agnt, const char * the_lexeme);
extern void get_context_var_info ( agent* agnt, Symbol **dest_goal, Symbol **dest_attr_of_slot, Symbol **dest_current_value);
extern Symbol *read_identifier_or_context_variable (agent* agnt);

/* ---------------------------------------------------------------------
                       Timer Utility Routines

   These are utility routines for using timers.  We use (struct timeval)'s
   (defined in a system include file) for keeping track of the cumulative
   time spent in one part of the system or another.  Reset_timer()
   clears a timer to 0.  Start_timer() and stop_timer() are used for
   timing an interval of code--the usage is:
   
     start_timer (&timeval_to_record_the_start_time_in); 
     ... other code here ...
     stop_timer (&timeval_to_record_the_start_time_in,
                 &timeval_holding_accumulated_time_for_this_code);

   Finally, timer_value() returns the accumulated value of a timer
   (in seconds).
--------------------------------------------------------------------- */

extern double timer_value (struct timeval *tv);
extern void reset_timer (struct timeval *tv_to_reset);
#ifndef NO_TIMING_STUFF
extern void start_timer (agent* thisAgent, struct timeval *tv_for_recording_start_time);
extern void stop_timer (agent* thisAgent,
                        struct timeval *tv_with_recorded_start_time,
                        struct timeval *tv_with_accumulated_time);
#else // !NO_TIMING_STUFF
#define start_timer(X,Y)
#define stop_timer(X,Y,Z)
#endif // !NO_TIMING_STUFF

#ifdef REAL_TIME_BEHAVIOR
/* RMJ */
extern void init_real_time (agent* thisAgent);
extern struct timeval *current_real_time;
#endif // REAL_TIME_BEHAVIOR

#ifdef ATTENTION_LAPSE
/* RMJ */
extern void wake_from_attention_lapse ();
extern void init_attention_lapse ();
extern void start_attention_lapse (long duration);
#endif // ATTENTION_LAPSE

// formerly in misc.h:
//////////////////////////////////////////////////////////
// String functions
//////////////////////////////////////////////////////////

// Determine if a string represents a natural number (i.e. all numbers)
extern bool is_natural_number( std::string *str );
extern bool is_natural_number( const char *str );

//////////////////////////////////////////////////////////
// Map functions
//////////////////////////////////////////////////////////

// get a list of all keys of a map
template <class X, class Y> std::vector<X> *map_keys( std::map<X,Y> *my_map )
{
	typename std::vector<X> *return_val = new std::vector<X>();
	typename std::map<X,Y>::iterator b, e;
	
	e = my_map->end();
	
	for ( b = my_map->begin(); b != e; b++ )
		return_val->push_back( b->first );
	
	return return_val;
}

// determine if a key is being used
template <class X, class Y> bool is_set( std::map<X,Y> *my_map, X *key )
{
	return ( my_map->find( *key ) != my_map->end() );
}

//////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////

// get a numeric value from a symbol
extern double get_number_from_symbol( Symbol *sym );

#endif //UTILITIES_H
