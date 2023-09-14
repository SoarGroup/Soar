#ifndef PRODUCTION_H
#define PRODUCTION_H

#include "kernel.h"
#include "stl_typedefs.h"

#include <map>
#include <set>

typedef struct production_struct
{
    ProductionType                  type;
    Symbol*                         name;
    struct rete_node_struct*        p_node;                     /* NIL if it's not in the rete */
    char*                           original_rule_name;
    char*                           documentation;              /* pointer to memory block, or NIL */
    char*                           filename;                   /* name of source file, or NIL. */
    SupportType                     declared_support;
    action*                         action_list;                /* RHS actions */
    cons*                           rhs_unbound_variables;      /* RHS vars not bound on LHS */
    int                             OPERAND_which_assert_list;
    bool                            trace_firings;              /* used by pwatch */
    uint64_t                        reference_count;
    uint64_t                        firing_count;
    struct instantiation_struct*    instantiations;             /* dll of inst's in MS */
    struct production_struct        *next, *prev;

    uint64_t                        naming_depth;
    bool                            explain_its_chunks;
    bool                            save_for_justification_explanation;
    byte                            interrupt;
    uint64_t                        p_id;

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
    double rl_gql;                // second value for implementation of GQ(\lambda)

    condition* rl_template_conds;

    int      duplicate_chunks_this_cycle;
    uint64_t last_duplicate_dc;

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

/* --------------------------------------------------------------------
                      Transitive Closure Utilities
               Marking, Unmarking, and Collecting Symbols

   Get_new_tc_number() is called from lots of places.  Any time we need
   to mark a set of identifiers and/or variables, we get a new tc_number
   by calling this routine, then proceed to mark various ids or vars
   by setting the sym->tc_num or sym->tc_num fields.

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
                      cons** id_list, cons** var_list);
void add_cond_to_tc(agent* thisAgent, condition* c, tc_number tc,
                    cons** id_list, cons** var_list);
void add_action_to_tc(agent* thisAgent, action* a, tc_number tc,
                      cons** id_list, cons** var_list);
bool cond_is_in_tc(agent* thisAgent, condition* cond, tc_number tc);
bool action_is_in_tc(action* a, tc_number tc);

void add_all_variables_in_condition_list(agent* thisAgent, condition* cond_list,
        tc_number tc, cons** var_list);


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
ProdReorderFailureType reorder_and_validate_lhs_and_rhs(agent*                    thisAgent,
                                                        condition**               lhs_top,
                                                        action**                  rhs_top,
                                                        bool                      reorder_nccs,
                                                        matched_symbol_list*      ungrounded_syms = NULL,
                                                        bool                      add_ungrounded_lhs = false,
                                                        bool                      add_ungrounded_rhs = false
);

production* make_production(agent* thisAgent, ProductionType type,
                                   Symbol* name, char* original_rule_name,
                                   condition** lhs_top, action** rhs_top,
                                   bool reorder_nccs, preference* results);

void deallocate_production(agent* thisAgent, production* prod);
void excise_production(agent* thisAgent, production* prod, bool print_sharp_sign = true, bool cacheProdForExplainer = false);
void excise_all_productions_of_type(agent* thisAgent, byte type, bool print_sharp_sign, bool cacheProdForExplainer = false);
void excise_all_productions(agent* thisAgent, bool print_sharp_sign, bool cacheProdForExplainer = false);

inline void production_add_ref(production* p)
{
    (p)->reference_count++;
}

inline void production_remove_ref(agent* thisAgent, production* p)
{
    if (--(p->reference_count) == 0) deallocate_production(thisAgent, p);
}

#endif
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
