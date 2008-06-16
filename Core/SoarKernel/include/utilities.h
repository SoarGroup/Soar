/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

#endif //UTILITIES_H
