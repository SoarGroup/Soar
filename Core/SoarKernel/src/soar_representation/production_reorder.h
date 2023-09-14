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

#include "stl_typedefs.h"

typedef struct saved_test_struct
{
    struct saved_test_struct*   next;
    Symbol*                     var;
    uint64_t                    inst_identity;
    Identity*                   identity;
    test                        the_test;
} saved_test;

extern ProdReorderFailureType reorder_action_list(agent* thisAgent, action** action_list, tc_number lhs_tc, matched_symbol_list* ungrounded_syms, bool add_ungrounded = false);
extern ProdReorderFailureType reorder_lhs(agent* thisAgent, condition** lhs_top, bool reorder_nccs, matched_symbol_list* ungrounded_syms = NULL, bool add_ungrounded = false);
extern void init_reorderer(agent* thisAgent);

/* this prototype moved here from osupport.cpp -ajc (5/3/02) */
extern cons* collect_root_variables(agent* thisAgent, condition* cond_list, tc_number tc, matched_symbol_list* ungrounded_syms = NULL, bool add_ungrounded = false);

#endif

