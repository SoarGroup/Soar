/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* -------------------------------------------------------------------
                            production.h

   Fields in a production:

      name:  points to the name of the production (a symbol)

      documentation:  points to a string (a memory_block_for_string) giving
        user-provided documentation about the production, or NIL if the
        user didn't give any documentation for it.

      reference_count:  (see below)

      firing_count:  the number of times this production has ever fired
        since it was created.  (Note that this is not reset by an init-soar.)

      next, prev:  used for a doubly-linked list of productions of the same
        type (see below).  The list header is all_productions_of_type[].

      type: the type of the production:  USER_PRODUCTION_TYPE,
        DEFAULT_PRODUCTION_TYPE, CHUNK_PRODUCTION_TYPE, or
        JUSTIFICATION_PRODUCTION_TYPE.

      declared_support:  indicates whether the production was declared
        :o-support or :i-support.  This field is either UNDECLARED_SUPPORT,
        DECLARED_O_SUPPORT, or DECLARED_I_SUPPORT.

      trace_firings:  true iff a (pwatch) has been set on this production.

      p_node:  If the production is currently in the Rete, this points to
        the corresponding p_node in the Rete.  If the production is not in
        the Rete, this field is NIL.

      action_list:  singly-linked list of the RHS actions of the production.

      rhs_unbound_variables:  A (consed) list of variables used on the RHS
        that aren't bound on the LHS, in the order of their indices (for
        rhs_values).  For chunks, this is NIL, since we discard chunk
        variable names.

      instantiations:  header for a doubly-linked list of the instantiations
        of this production that are currently in the match set (i.e.,
        Rete-supported).

      OPERAND_which_assert_list: (need comment info from REW or RCHONG)

   Reference counts on productions:
      +1 if it's in production memory (i.e., hasn't been excised)
      +1 for each existing instantiation pointing to it
   We deallocate a production if its reference_count goes to 0.
------------------------------------------------------------------- */

#ifndef PRODUCTION_H
#define PRODUCTION_H

#include "portability.h"
#include "kernel.h"
#include <map>
#include <set>

typedef char* rhs_value;
typedef unsigned char byte;
typedef struct action_struct action;
typedef struct cons_struct cons;
typedef struct agent_struct agent;
typedef cons list;
typedef struct symbol_struct Symbol;
typedef struct preference_struct preference;
typedef struct wme_struct wme;
typedef signed short goal_stack_level;
typedef struct test_struct test_info;
typedef test_info* test;

typedef std::map< Symbol*, Symbol* > rl_symbol_map;
typedef std::set< rl_symbol_map > rl_symbol_map_set;

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
    byte type;
    bool already_in_tc;                    /* used only by cond_is_in_tc stuff */
    bool test_for_acceptable_preference;   /* for positive, negative cond's only */
    struct condition_struct* next, *prev;

    union condition_main_data_union
    {
        struct
        {
            test id_test;
            test attr_test;
            test value_test;
        } tests;                           /* for positive, negative cond's only */
        ncc_info ncc;                        /* for ncc's only */
    } data;

    bt_info bt;            /* for top-level positive cond's: used for BT and by the rete */
    reorder_info reorder;  /* used only during reordering */
} condition;

typedef struct production_struct
{
    Symbol* name;
    char* original_rule_name;
    char* documentation;        /* pointer to memory block, or NIL */
    char* filename;             /* name of source file, or NIL.  kjh CUSP(b11) */
    uint64_t reference_count;
    uint64_t firing_count;             /* how many times it's fired */
    struct production_struct* next, *prev;  /* used for dll */
    byte type;
    byte declared_support;
    bool trace_firings;                     /* used by pwatch */
    struct rete_node_struct* p_node;        /* NIL if it's not in the rete */
    action* action_list;                    /* RHS actions */
    ::list* rhs_unbound_variables;            /* RHS vars not bound on LHS */
    struct instantiation_struct* instantiations; /* dll of inst's in MS */
    int OPERAND_which_assert_list;          /* RCHONG: 10.11 */
    byte interrupt;                         /* SW: 7.31.03 */

    struct
    {
        bool interrupt_break : 1;
        bool already_fired : 1;         /* RPM test workaround for bug #139 */
        bool rl_rule : 1;                   /* if true, is a Soar-RL rule */
    };

    double rl_update_count;       /* number of (potentially fractional) updates to this rule */
    unsigned int rl_ref_count;    /* number of states referencing this rule in prev_op_rl_rules list */

    // Per-input memory parameters for delta bar delta algorithm
    double rl_delta_bar_delta_beta;
    double rl_delta_bar_delta_h;

    double rl_ecr;                // expected current reward (discounted reward)
    double rl_efr;                // expected future reward (discounted next state)

    condition* rl_template_conds;
    rl_symbol_map_set* rl_template_instantiations;

} production;

/* ========================================================================

   Various utility routines for manipulating productions and parts thereof.
   Also includes the reorderer and compile-time o-support calculations.

   Init_production_utilities() should be called before anything else here.
======================================================================== */

/* This structure is used to break ties in favor of non-multi-attributes */
typedef struct multi_attributes_struct
{
    Symbol* symbol;
    int64_t value;
    struct multi_attributes_struct* next;
} multi_attribute;

void init_production_utilities(agent* thisAgent);

/* ------------------------ */
/* Utilities for conditions */
/* ------------------------ */

/* --- Deallocates a condition (including any NCC's and tests in it). */
void deallocate_condition(agent* thisAgent, condition* cond);

/* --- Deallocates a condition list (including any NCC's and tests in it). */
void deallocate_condition_list(agent* thisAgent, condition* cond_list);

/* --- Initializes substructures of the given condition to default values. --- */
extern void init_condition(condition* cond);

/* --- Returns a new copy of the given condition. --- */
condition* copy_condition(agent* thisAgent, condition* cond);

/* --- Returns a new copy of the given condition without any relational tests --- */
condition* copy_condition_without_relational_constraints(agent* thisAgent, condition* cond);

/* --- Copies the given condition list, returning pointers to the
   top-most and bottom-most conditions in the new copy. --- */
void copy_condition_list(agent* thisAgent, condition* top_cond, condition** dest_top,
                         condition** dest_bottom);

void add_bound_variables_in_condition(agent* thisAgent, condition* c, tc_number tc,
                                      ::list** var_list);
void unmark_variables_and_free_list(agent* thisAgent, ::list* var_list);

/* --- Returns true iff the two conditions are identical. --- */
bool conditions_are_equal(condition* c1, condition* c2);

/* --- Returns a hash value for the given condition. --- */
uint32_t hash_condition(agent* thisAgent, condition* cond);

bool canonical_cond_greater(condition* c1, condition* c2);

/* --------------------------------------------------------------------
                      Transitive Closure Utilities
               Marking, Unmarking, and Collecting Symbols

   Get_new_tc_number() is called from lots of places.  Any time we need
   to mark a set of identifiers and/or variables, we get a new tc_number
   by calling this routine, then proceed to mark various ids or vars
   by setting the sym->common.tc_num.

   Sometimes in addition to marking symbols using their tc_num fields,
   we also want to build up a list of the symbols we've marked.  So,
   the routines here take an "id_list" or "var_list" argument.  This
   argument should be NIL if no such list is desired.  If non-NIL, it
   should point to the header of the linked list being built.

       Transitive Closure Calculations for Conditions and Actions

   Usage:
     1. Set my_tc = get_new_tc_number() to start a new TC
     2. (optional) If you want linked lists of symbols in the TC, initialize
        id_list=NIL and var_list=NIL.
        If you're not using id_list and/or var_list, give NIL for "&id_list"
        and/or "&var_list" in the function calls below.
     3. (optional) setup any id's or var's that you want to include in the
        initial TC, by calling
           add_symbol_to_tc (sym, my_tc, &id_list, &var_list)
        (If not using id_list or var_list, you can just mark
         sym->{id,var}.tc_num = my_tc instead.)
     4. To do the work you want, use any of the following any number of times:
            add_cond_to_tc (cond, my_tc, &id_list, &var_list);
            add_action_to_tc (cond, my_tc, &id_list, &var_list);
            result = cond_is_in_tc (cond, my_tc);
            result = action_is_in_tc (action, my_tc);
     5. When finished, free the cons cells in id_list and var_list (but
        don't call symbol_remove_ref() on the symbols in them).

  Warning:  actions must not contain reteloc's or rhs unbound variables here.
-------------------------------------------------------------------- */

tc_number get_new_tc_number(agent* thisAgent);
void add_symbol_to_tc(agent* thisAgent, Symbol* sym, tc_number tc,
                      ::list** id_list, ::list** var_list);
void add_cond_to_tc(agent* thisAgent, condition* c, tc_number tc,
                    ::list** id_list, ::list** var_list);
void add_action_to_tc(agent* thisAgent, action* a, tc_number tc,
                      ::list** id_list, ::list** var_list);
bool cond_is_in_tc(agent* thisAgent, condition* cond, tc_number tc);
bool action_is_in_tc(action* a, tc_number tc);

/* --------------------------------------------------------------------
                         Variable Generator

   These routines are used for generating new variables.  The variables
   aren't necessarily "completely" new--they might occur in some existing
   production.  But we usually need to make sure the new variables don't
   overlap with those already used in a *certain* production--for instance,
   when variablizing a chunk, we don't want to introduce a new variable that
   conincides with the name of a variable already in an NCC in the chunk.

   To use these routines, first call reset_variable_generator(), giving
   it lists of conditions and actions whose variables should not be
   used.  Then call generate_new_variable() any number of times; each
   time, you give it a string to use as the prefix for the new variable's
   name.  The prefix string should not include the opening "<".
-------------------------------------------------------------------- */

void reset_variable_generator(agent* thisAgent,
                              condition* conds_with_vars_to_avoid,
                              action* actions_with_vars_to_avoid);
Symbol* generate_new_variable(agent* thisAgent, const char* prefix);

/* -------------------------------------------------------------------
                         Production Management

    For each type of production, we maintain a doubly-linked list of
    all productions of that type.  The headers of these dll's are
    stored in the array all_productions_of_type[].  Another array,
    num_productions_of_type[], keeps counts of how many productions
    there are of each type.

    Production_add_ref() and production_remove_ref() are macros for
    incrementing and decrementing the reference count on a production.
    Production_remove_ref() also deallocates the production if the
    count goes to 0.

    Make_production() does reordering, compile-time o-support calc's,
    and builds and returns a production structure for a new production.
    It does not enter the production into the Rete net, however.
    The "type" argument should be one of USER_PRODUCTION_TYPE, etc.
    The flag "reorder_nccs" tells whether to recursively reorder
    the subconditions of NCC's--this is not necessary for newly
    built chunks, as their NCC's are copies of other NCC's in SP's that
    have already been reordered.  If any error occurs, make_production()
    returns NIL.

    Deallocate_production() and excise_production() do just what they
    say.  Normally deallocate_production() should be invoked only via
    the production_remove_ref() macro.
------------------------------------------------------------------- */

production* make_production(agent* thisAgent,
                            byte type,
                            Symbol* name,
                            char* original_rule_name,
                            condition** lhs_top,
                            condition** lhs_bottom,
                            action** rhs_top,
                            bool reorder_nccs,
                            preference* results = NULL);

void deallocate_production(agent* thisAgent, production* prod);
void excise_production(agent* thisAgent, production* prod, bool print_sharp_sign);
void excise_all_productions_of_type(agent* thisAgent,
                                    byte type,
                                    bool print_sharp_sign);
void excise_all_productions(agent* thisAgent,
                            bool print_sharp_sign);

inline void production_add_ref(production* p)
{
    (p)->reference_count++;
}

inline void production_remove_ref(agent* thisAgent, production* p)
{
    (p)->reference_count--;
    if ((p)->reference_count == 0)
    {
        deallocate_production(thisAgent, p);
    }
}

#endif
