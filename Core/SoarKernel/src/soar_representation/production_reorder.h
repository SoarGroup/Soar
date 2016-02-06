/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* =======================================================================
                                reorder.h
   Need to add comments here
======================================================================= */

#ifndef REORDER_H
#define REORDER_H

#include "kernel.h"

typedef struct saved_test_struct
{
    struct saved_test_struct* next;
    Symbol* var;
    test the_test;
} saved_test;

extern bool reorder_action_list(agent* thisAgent, action** action_list, tc_number lhs_tc);
extern bool reorder_lhs(agent* thisAgent, condition** lhs_top, bool reorder_nccs, bool collect_ungroundeds = false, symbol_list* ungrounded_syms = NULL);
extern void init_reorderer(agent* thisAgent);

/* this prototype moved here from osupport.cpp -ajc (5/3/02) */
extern list* collect_root_variables(agent* thisAgent, condition* cond_list, tc_number tc, bool allow_printing_warnings, bool collect_ungroundeds = false, symbol_list* ungrounded_syms = NULL);

#endif

