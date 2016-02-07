#ifndef CONDITION_H
#define CONDITION_H

#include "kernel.h"

/* -------------------------------------------------------------------
                             Conditions

   Conditions are used for two things:  (1) to represent the LHS of newly
   entered productions (new SP's or chunks); and (2) to represent the
   instantiated LHS in production instantiations.

   Fields in a condition:

      type:  indicates the type of condition:  either POSITIVE_CONDITION,
        NEGATIVE_CONDITION, or CONJUNCTIVE_NEGATION_CONDITION.

      already_in_tc:  (reserved for use by the cond_is_in_tc() stuff in
        production.c)

      next, prev:  used for a doubly-linked list of all conditions on the
        LHS, or all subconditions of an NCC.

      data.tests.id_test, data.tests.attr_test, data.tests.value_test:
        for positive and negative conditions, these are the three wme
        field tests for the condition.

      test_for_acceptable_preference:  for positive and negative conditions,
        this is true iff the condition tests for acceptable preference wmes.

      data.ncc.top, data.ncc.bottom:  for NCC's, these point to the top and
        bottom of the subconditions linked list.

      bt:  for top-level positive conditions in production instantiations,
        this structure gives information for that will be used in backtracing.

      reorder:  (reserved for use by the reorderer)
------------------------------------------------------------------- */

/* --- info on conditions used for backtracing (and by the rete) --- */
typedef struct bt_info_struct
{
    wme* wme_;                /* the actual wme that was matched */
    goal_stack_level level;   /* level (at firing time) of the id of the wme */
    preference* trace;        /* preference for BT, or NIL */

    ::list* CDPS;            /* list of substate evaluation prefs to backtrace through,
                              i.e. the context dependent preference set. */
    bt_info_struct() : wme_(NULL), level(0), trace(NULL), CDPS(NULL) {}
} bt_info;

/* --- info on conditions used only by the reorderer --- */
typedef struct reorder_info_struct
{
    ::list* vars_requiring_bindings;         /* used only during reordering */
    struct condition_struct* next_min_cost;  /* used only during reordering */
} reorder_info;

/* --- info on negated conjunctive conditions only --- */
typedef struct ncc_info_struct
{
    struct condition_struct* top;
    struct condition_struct* bottom;
} ncc_info;

/* --- finally, the structure of a condition --- */
typedef struct condition_struct
{
    ConditionType               type;
    bool                        already_in_tc;                    /* used only by cond_is_in_tc stuff */
    bool                        test_for_acceptable_preference;   /* for pos/neg conds only. Not NCCs */
    struct condition_struct*    next, *prev;

    union condition_main_data_union
    {
        struct
        {
            test                id_test;
            test                attr_test;
            test                value_test;
        } tests;                                /* for positive, negative cond's only */
        ncc_info                ncc;            /* for negative conjunctive conds only */
    } data;
    bt_info                     bt;             /* backtrace info for top-level positive cond's:
                                                   used by chunking and the rete */
    reorder_info                reorder;        /* used only during reordering */
    struct condition_struct*    counterpart;    /* only used during chunking */
} condition;

/* ------------------------ */
/* Utilities for conditions */
/* ------------------------ */

/* --- Deallocates a condition (including any NCC's and tests in it). */
void deallocate_condition(agent* thisAgent, condition* cond);

/* --- Deallocates a condition list (including any NCC's and tests in it). */
extern void deallocate_condition_list(agent* thisAgent, condition* cond_list);

/* --- Initializes substructures of the given condition to default values. --- */
extern void init_condition(condition* cond);

/* --- Returns a new copy of the given condition. --- */
extern condition* copy_condition(agent* thisAgent, condition* cond, bool pUnify_variablization_identity = false, bool pStripLiteralConjuncts = false);

/* --- Returns a new copy of the given condition without any relational tests --- */
condition* copy_condition_without_relational_constraints(agent* thisAgent, condition* cond);

/* --- Copies the given condition list, returning pointers to the
   top-most and bottom-most conditions in the new copy. --- */
extern void copy_condition_list(agent* thisAgent, condition* top_cond, condition** dest_top,
                         condition** dest_bottom, bool pUnify_variablization_identity = false, bool pStripLiteralConjuncts = false);

void add_bound_variables_in_condition(agent* thisAgent, condition* c, tc_number tc,
                                      ::list** var_list, bool add_LTIs = false);
void unmark_variables_and_free_list(agent* thisAgent, ::list* var_list);

/* --- Returns true iff the two conditions are identical. --- */
extern bool conditions_are_equal(condition* c1, condition* c2);

/* --- Returns a hash value for the given condition. --- */
extern uint32_t hash_condition(agent* thisAgent, condition* cond);

bool canonical_cond_greater(condition* c1, condition* c2);

#endif
