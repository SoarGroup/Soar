#include <portability.h>
/* -------------------------------------------------------------------
                              test.h
                          Utilities for tests
------------------------------------------------------------------- */
#ifndef TEST_H_
#define TEST_H_

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

/* --- Types of tests (can't be 255 -- see rete.cpp) --- */

enum TestType {
         NOT_EQUAL_TEST = 1,          /* various relational tests */
         LESS_TEST = 2,
         GREATER_TEST = 3,
         LESS_OR_EQUAL_TEST = 4,
         GREATER_OR_EQUAL_TEST = 5,
         SAME_TYPE_TEST = 6,
         DISJUNCTION_TEST = 7,        /* item must be one of a list of constants */
         CONJUNCTIVE_TEST = 8,        /* item must pass each of a list of tests */
         GOAL_ID_TEST = 9,            /* item must be a goal identifier */
         IMPASSE_ID_TEST = 10,        /* item must be an impasse identifier */
         EQUALITY_TEST = 11,
         BLANK_TEST = 12,
         INVALID_TEST = 13
};

#define NUM_TEST_TYPES 11           /* Note: count does not include blank tests*/

/* --- Test struct stores information about all test types, including
 *     equality tests (Soar 9.3.2 and below did not).
 *
 *     The original_test pointer stores the test that was defined when the
 *     production was read in by the parser.  The values are filled in by the
 *     rete when reconstructing a production.  It is used by the chunker to
 *     determine when to variablize constant symbols. - MMA 2013
 *
 *     ---*/

typedef struct test_struct {
  TestType type;                  /* see definitions below */
  union test_info_union {
    Symbol *referent;           /* for relational tests */
    ::list *disjunction_list;   /* for disjunction tests */
    ::list *conjunct_list;      /* for conjunctive tests */
  } data;
  test_struct *original_test;
} test_info;

/* --- Note that the test typedef is a *pointer* to a test struct. A test is
 *     considered blank when that pointer is nil. --- */
typedef test_info * test;

/* --- Descriptions of these functions can be found in the test.cpp --- */

inline Bool test_is_blank(test t){return (t == 0);}
inline Bool test_is_variable(agent* thisAgent, test t);
Bool tests_are_equal (test t1, test t2, bool neg);
Bool tests_are_equal_with_bindings (agent* thisAgent, test t1, test test2, list **bindings);
Bool test_includes_equality_test_for_symbol (test t, Symbol *sym);
Bool test_includes_goal_or_impasse_id_test (test t, Bool look_for_goal, Bool look_for_impasse);
test copy_of_equality_test_found_in_test (agent* thisAgent, test t);

inline test make_blank_test() {return static_cast<test>(0);}
inline test make_test_without_refcount(agent* thisAgent, Symbol * sym, TestType test_type);
inline test make_test(agent* thisAgent, Symbol * sym, TestType test_type);
inline uint32_t hash_test (agent* thisAgent, test t);
void deallocate_test (agent* thisAgent, test t);

test copy_test (agent* thisAgent, test t);
test copy_test_removing_goal_impasse_tests (agent* thisAgent, test t, Bool *removed_goal, Bool *removed_impasse);

void add_new_test_to_test (agent* thisAgent, test *t, test add_me, test add_me_original=NULL);
void add_new_test_to_test_if_not_already_there (agent* thisAgent, test *t, test add_me, bool neg);

void add_all_variables_in_test (agent* thisAgent, test t, tc_number tc, list **var_list);
void add_bound_variables_in_test (agent* thisAgent, test t, tc_number tc, ::list **var_list);

char first_letter_from_test (test t);

#endif /* TEST_H_ */
