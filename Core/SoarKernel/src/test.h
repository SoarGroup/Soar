#include "portability.h"
/* -------------------------------------------------------------------
                              test.h

   Tests in conditions can be blank tests (null), tests for equality
   with a symbol, relational tests with a referent symbol, disjunctive
   tests between a list of constant symbols or a conjunction
   of multiple tests of any arbitrary type (except another conjunctive
   test).

   Note: This test structure and file was introduced in Soar 9.4 to
         support chunking of other symbol types and adding all test
         types to chunks.  The previous system differed significantly.
         - MMA 2013
------------------------------------------------------------------- */

#ifndef TEST_H_
#define TEST_H_

#include "kernel.h"
#include "symtab.h"

/* -- Forward declarations --- */
typedef struct node_varnames_struct node_varnames;
typedef struct condition_struct condition;
typedef struct wme_struct wme;
typedef struct rete_node_struct rete_node;
typedef struct rete_test_struct rete_test;
typedef char varnames;
typedef unsigned short rete_node_level;
typedef struct cons_struct cons;
typedef cons list;
typedef struct agent_struct agent;
typedef struct symbol_struct Symbol;
typedef signed short goal_stack_level;
typedef struct action_struct action;
template <typename T> inline void allocate_cons(agent* thisAgent, T* dest_cons_pointer);

/* -------------------------------------------------------------------
                              Tests

   Tests in conditions can be blank (null) tests, tests for equality
   with a variable or constant, or more complicated tests (such as
   not-equal tests, conjunctive tests, etc.).  We use some bit twiddling
   here to minimize space.  We use just a pointer to represent any kind
   of test.  For blank tests, this is the NIL pointer.  For equality tests,
   it points to the symbol referent of the test.  For other kinds of tests,
   bit 0 of the pointer is set to 1, and the pointer (minus 1) points to
   a complex_test structure.  (A field in the complex_test structure
   further indicates the type of the test.)
------------------------------------------------------------------- */

typedef char* test;
typedef struct complex_test_struct
{
    byte type;                  /* see definitions below */
    union test_info_union
    {
        Symbol* referent;         /* for relational tests */
        ::list* disjunction_list;   /* for disjunction tests */
        ::list* conjunct_list;      /* for conjunctive tests */
    } data;
} complex_test;

/* ------------------- */
/* Utilities for tests */
/* ------------------- */

extern void add_all_variables_in_action(agent* thisAgent, action* a, tc_number tc,
                                        ::list** var_list);
extern void add_bound_variables_in_test(agent* thisAgent, test t, tc_number tc,
                                        ::list** var_list);
extern void add_bound_variables_in_condition(agent* thisAgent, condition* c, tc_number tc,
        ::list** var_list);
extern void unmark_variables_and_free_list(agent* thisAgent, ::list* var_list);

/* --- Takes a test and returns a new copy of it. --- */
extern test copy_test(agent* thisAgent, test t);

/* --- Same as copy_test(), only it doesn't include goal or impasse tests
   in the new copy.  The caller should initialize the two flags to false
   before calling this routine; it sets them to true if it finds a goal
   or impasse test. --- */
extern test copy_test_removing_goal_impasse_tests
(agent* thisAgent, test t, bool* removed_goal, bool* removed_impasse);

/* --- Deallocates a test. --- */
extern void deallocate_test(agent* thisAgent, test t);

/* --- Destructively modifies the first test (t) by adding the second
   one (add_me) to it (usually as a new conjunct).  The first test
   need not be a conjunctive test. --- */
extern void add_new_test_to_test(agent* thisAgent, test* t, test add_me);

/* --- Same as above, only has no effect if the second test is already
   included in the first one. --- */
extern void add_new_test_to_test_if_not_already_there(agent* thisAgent, test* t, test add_me, bool neg);

/* --- Returns true iff the two tests are identical.
   If neg is true, ignores order of members in conjunctive tests
   and assumes variables are all equal. --- */
extern bool tests_are_equal(test t1, test t2, bool neg);

/* --- Returns a hash value for the given test. --- */
extern uint32_t hash_test(agent* thisAgent, test t);

/* --- Returns true iff the test contains an equality test for the given
   symbol.  If sym==NIL, returns true iff the test contains any equality
   test. --- */
extern bool test_includes_equality_test_for_symbol(test t, Symbol* sym);

/* --- Looks for goal or impasse tests (as directed by the two flag
   parameters) in the given test, and returns true if one is found. --- */
extern bool test_includes_goal_or_impasse_id_test(test t,
        bool look_for_goal,
        bool look_for_impasse);

/* --- Looks through a test, and returns a new copy of the first equality
   test it finds.  Signals an error if there is no equality test in the
   given test. --- */
extern test copy_of_equality_test_found_in_test(agent* thisAgent, test t);

/* --- Looks through a test, returns appropriate first letter for a dummy
   variable to follow it.  Returns '*' if none found. --- */
extern char first_letter_from_test(test t);

inline bool test_is_blank_test(test t)
{
    return (t == NIL);
}

inline bool test_is_complex_test(test t)
{
    return (char)(
               reinterpret_cast<uint64_t>(t)
               & 1);
}

#ifdef _MSC_VER
#pragma warning (default : 4311)
#endif

inline bool test_is_blank_or_equality_test(test t)
{
    return (!test_is_complex_test(t));
}

inline test make_blank_test()
{
    return static_cast<test>(NIL);
}

inline test make_equality_test(Symbol* sym)
{
    (sym)->reference_count++;
#ifdef DEBUG_SYMBOL_REFCOUNTS
    char buf[64];
    OutputDebugString(symbol_to_string(0, (sym), false, buf, 64));
    OutputDebugString(":+ ");
    OutputDebugString(_itoa((sym)->reference_count, buf, 10));
    OutputDebugString("\n");
#endif // DEBUG_SYMBOL_REFCOUNTS
    return reinterpret_cast<test>(sym);
}

inline test make_equality_test_without_adding_reference(Symbol* sym)
{
    return reinterpret_cast<test>(sym);
}

inline test make_blank_or_equality_test(Symbol* sym_or_nil)
{
    return ((sym_or_nil) ? (make_equality_test(sym_or_nil)) : make_blank_test());
}

inline char* make_test_from_complex_test(complex_test* ct)
{
    return reinterpret_cast<test>(ct) + 1;
}

inline Symbol* referent_of_equality_test(test t)
{
    return reinterpret_cast<Symbol*>(t);
}

inline complex_test* complex_test_from_test(test t)
{
    return reinterpret_cast<complex_test*>(t - 1);
}

inline void quickly_deallocate_test(agent* thisAgent, test t)
{
    if (! test_is_blank_test(t))
    {
        if (test_is_blank_or_equality_test(t))
        {
            symbol_remove_ref(thisAgent, referent_of_equality_test(t));
        }
        else
        {
            deallocate_test(thisAgent, t);
        }
    }
}

#endif
