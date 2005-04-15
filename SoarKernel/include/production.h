/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

      trace_firings:  TRUE iff a (pwatch) has been set on this production.

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

#ifdef __cplusplus
extern "C"
{
#endif

#define UNDECLARED_SUPPORT 0
#define DECLARED_O_SUPPORT 1
#define DECLARED_I_SUPPORT 2

/* RCHONG: begin 10.11 */

#define PE_PRODS 0
#define IE_PRODS 1
#define NO_SAVED_PRODS -1

/* RCHONG: end 10.11 */

struct not_struct;

typedef char Bool;
typedef char * test;
typedef char * rhs_value;
typedef unsigned char byte;
typedef unsigned long tc_number;
typedef struct action_struct action;
typedef struct condition_struct condition;
typedef struct cons_struct cons;
typedef struct agent_struct agent;
typedef cons list;
typedef union symbol_union Symbol;

typedef struct production_struct {
  Symbol *name;
  char *documentation;        /* pointer to memory block, or NIL */
  char *filename;             /* name of source file, or NIL.  kjh CUSP(b11) */
  unsigned long reference_count;
  unsigned long firing_count;             /* how many times it's fired */
  struct production_struct *next, *prev;  /* used for dll */
  byte type;
  byte declared_support;
  Bool trace_firings;                     /* used by pwatch */
  struct rete_node_struct *p_node;        /* NIL if it's not in the rete */
  action *action_list;                    /* RHS actions */
  list *rhs_unbound_variables;            /* RHS vars not bound on LHS */
  struct instantiation_struct *instantiations; /* dll of inst's in MS */
  int OPERAND_which_assert_list;          /* RCHONG: 10.11 */
  byte interrupt;						  /* SW: 7.31.03 */
  bool already_fired;         /* RPM test workaround for bug #139 */
} production;

/* ========================================================================

   Various utility routines for manipulating productions and parts thereof.
   Also includes the reorderer and compile-time o-support calculations.

   Init_production_utilities() should be called before anything else here.
======================================================================== */

/* This structure is used to break ties in favor of non-multi-attributes */
typedef struct multi_attributes_struct {
  Symbol *symbol;
  long value;
  struct multi_attributes_struct *next;
} multi_attribute;

extern void init_production_utilities (agent* thisAgent);

/* ------------------------------------------ */
/* Utilities for symbols and lists of symbols */
/* ------------------------------------------ */

/* --- Looks at a symbol, returns appropriate first letter for a dummy
   variable or identifier to follow it.  Returns '*' if none found. --- */
extern char first_letter_from_symbol (Symbol *sym);

/* --- Takes a list of symbols and returns a copy of the same list,
   incrementing the reference count on each symbol in the list. --- */
extern list *copy_symbol_list_adding_references (agent* thisAgent, list *sym_list);

/* --- Frees a list of symbols, decrementing their reference counts. --- */
extern void deallocate_symbol_list_removing_references (agent* thisAgent, list *sym_list);

/* ------------------- */
/* Utilities for tests */
/* ------------------- */

extern void add_all_variables_in_action (agent* thisAgent, action *a, tc_number tc, 
					 list **var_list);
extern void add_bound_variables_in_test (agent* thisAgent, test t, tc_number tc, 
					 list **var_list);
extern void add_bound_variables_in_condition (agent* thisAgent, condition *c, tc_number tc, 
					      list **var_list);
extern void unmark_variables_and_free_list (agent* thisAgent, list *var_list);

/* --- Takes a test and returns a new copy of it. --- */
extern test copy_test (agent* thisAgent, test t);

/* --- Same as copy_test(), only it doesn't include goal or impasse tests
   in the new copy.  The caller should initialize the two flags to FALSE
   before calling this routine; it sets them to TRUE if it finds a goal
   or impasse test. --- */
extern test copy_test_removing_goal_impasse_tests
  (agent* thisAgent, test t, Bool *removed_goal, Bool *removed_impasse);

/* --- Deallocates a test. --- */
extern void deallocate_test (agent* thisAgent, test t);

/* --- Destructively modifies the first test (t) by adding the second
   one (add_me) to it (usually as a new conjunct).  The first test
   need not be a conjunctive test. --- */
extern void add_new_test_to_test (agent* thisAgent, test *t, test add_me); 

/* --- Same as above, only has no effect if the second test is already
   included in the first one. --- */
extern void add_new_test_to_test_if_not_already_there (agent* thisAgent, test *t, test add_me);

/* --- Returns TRUE iff the two tests are identical. --- */
extern Bool tests_are_equal (test t1, test t2);

/* --- Returns a hash value for the given test. --- */
extern unsigned long hash_test (agent* thisAgent, test t);

/* --- Returns TRUE iff the test contains an equality test for the given
   symbol.  If sym==NIL, returns TRUE iff the test contains any equality
   test. --- */
extern Bool test_includes_equality_test_for_symbol (test t, Symbol *sym);

/* --- Looks for goal or impasse tests (as directed by the two flag
   parameters) in the given test, and returns TRUE if one is found. --- */
extern Bool test_includes_goal_or_impasse_id_test (test t,
                                                   Bool look_for_goal,
                                                   Bool look_for_impasse);

/* --- Looks through a test, and returns a new copy of the first equality
   test it finds.  Signals an error if there is no equality test in the
   given test. --- */
extern test copy_of_equality_test_found_in_test (agent* thisAgent, test t);

/* --- Looks through a test, returns appropriate first letter for a dummy
   variable to follow it.  Returns '*' if none found. --- */
extern char first_letter_from_test (test t);

/* ------------------------ */
/* Utilities for conditions */
/* ------------------------ */

/* --- Deallocates a condition list (including any NCC's and tests in it). */
extern void deallocate_condition_list (agent* thisAgent, condition *cond_list);

/* --- Returns a new copy of the given condition. --- */
extern condition *copy_condition (agent* thisAgent, condition *cond);

/* --- Copies the given condition list, returning pointers to the
   top-most and bottom-most conditions in the new copy. --- */
extern void copy_condition_list (agent* thisAgent, condition *top_cond, condition **dest_top,
                                 condition **dest_bottom);

/* --- Returns TRUE iff the two conditions are identical. --- */
extern Bool conditions_are_equal (condition *c1, condition *c2);

/* --- Returns a hash value for the given condition. --- */
extern unsigned long hash_condition (agent* thisAgent, condition *cond);

/* ------------------------------------ */
/* Utilities for actions and RHS values */
/* ------------------------------------ */

/* --- Deallocates the given rhs_value. --- */
extern void deallocate_rhs_value (agent* thisAgent, rhs_value rv);

/* --- Returns a new copy of the given rhs_value. --- */
extern rhs_value copy_rhs_value (agent* thisAgent, rhs_value rv);

/* --- Deallocates the given action (singly-linked) list. --- */
extern void deallocate_action_list (agent* thisAgent, action *actions);

/* --- Looks through an rhs_value, returns appropriate first letter for a
   dummy variable to follow it.  Returns '*' if none found. --- */
extern char first_letter_from_rhs_value (rhs_value rv);

/* ------------------ */
/* Utilities for nots */
/* ------------------ */

/* --- Deallocates the given (singly-linked) list of Nots. --- */
extern void deallocate_list_of_nots (agent* thisAgent, not_struct *nots);

/* --------------------------------------------------------------------
                      Transitive Closure Utilities
               Marking, Unmarking, and Collecting Symbols

   Get_new_tc_number() is called from lots of places.  Any time we need
   to mark a set of identifiers and/or variables, we get a new tc_number
   by calling this routine, then proceed to mark various ids or vars
   by setting the sym->id.tc_num or sym->var.tc_num fields.

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

tc_number get_new_tc_number (agent* thisAgent);

extern void add_symbol_to_tc (agent* thisAgent, Symbol *sym, tc_number tc,
                              list **id_list, list **var_list);
extern void add_cond_to_tc (agent* thisAgent, condition *c, tc_number tc,
                            list **id_list, list **var_list);
extern void add_action_to_tc (agent* thisAgent, action *a, tc_number tc,
                              list **id_list, list **var_list);
extern Bool cond_is_in_tc (agent* thisAgent, condition *cond, tc_number tc);
extern Bool action_is_in_tc (action *a, tc_number tc);

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

extern void reset_variable_generator (agent* thisAgent, 
									  condition *conds_with_vars_to_avoid,
                                      action *actions_with_vars_to_avoid);
extern Symbol *generate_new_variable (agent* thisAgent, char *prefix);

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

extern production *make_production (agent* thisAgent, 
									byte type,
                                    Symbol *name,
                                    condition **lhs_top,
                                    condition **lhs_bottom,
                                    action **rhs_top,
                                    Bool reorder_nccs);
extern void deallocate_production (agent* thisAgent, production *prod);
extern void excise_production (agent* thisAgent, production *prod, Bool print_sharp_sign);
extern void excise_all_productions_of_type(agent* thisAgent,
                                           byte type,
                                           Bool print_sharp_sign);
extern void excise_all_productions(agent* thisAgent,       
                                   Bool print_sharp_sign);

extern Bool canonical_cond_greater(condition *c1, condition *c2);

#ifdef USE_MACROS

#define production_add_ref(p) { (p)->reference_count++; }
#define production_remove_ref(thisAgent, p) { \
  (p)->reference_count--; \
  if ((p)->reference_count == 0) \
    deallocate_production(thisAgent, p); }

#else

inline void production_add_ref(production * p)
{
  (p)->reference_count++;
}

inline void production_remove_ref(agent* thisAgent, production * p)
{
  (p)->reference_count--;
  if ((p)->reference_count == 0)
    deallocate_production(thisAgent, p);
}

#endif /* USE_MACROS */

#ifdef __cplusplus
}
#endif

#endif
