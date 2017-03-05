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
#include "stl_typedefs.h"

template <typename T> inline void allocate_cons(agent* thisAgent, T* dest_cons_pointer);

/* -- test_info stores information about a test.  If nil, the test is
 *    considered blank.
 *
 *    The original_test pointer stores the test that was defined when the
 *    production was read in by the parser.  The values are filled in by the
 *    rete when reconstructing a production.  It is used to fill out the
 *    identity structure then removed.
 *
 *    The eq_test pointer is used to cache the main equality test for an
 *    element in a condition so that we do not have to continually re-scan
 *
 *    Note that conjunctive tests always have a NULL identity.  Each
 *    constituent test of the conjunctive test has its own identity */

typedef struct test_struct
{
    TestType        type;                  /* see definitions in enums.h */
    union test_info_union
    {
        Symbol*     referent;         /* for relational tests */
        cons*       disjunction_list;   /* for disjunction tests */
        cons*       conjunct_list;      /* for conjunctive tests */
    } data;
    test_struct*    eq_test;
    uint64_t        identity;
    uint64_t        clone_identity;
    IdentitySetSharedPtr   identity_set;
} test_info;

/* --- Note that the test typedef is a *pointer* to a test struct. A test is
 *     considered blank when that pointer is nil. --- */
typedef test_info* test;

/* --- Descriptions of these functions can be found in the test.cpp --- */
char first_letter_from_test(test t);
bool tests_are_equal(test t1, test t2, bool neg);
bool tests_identical(test t1, test t2, bool considerIdentity = false);
bool test_includes_goal_or_impasse_id_test(test t, bool look_for_goal, bool look_for_impasse);
test copy_of_equality_test_found_in_test(agent* thisAgent, test t);
test find_eq_test(test t);

test make_test(agent* thisAgent, Symbol* sym, TestType test_type);
uint32_t hash_test(agent* thisAgent, test t);
void deallocate_test(agent* thisAgent, test t);

test copy_test(agent* thisAgent, test t, bool pUseUnifiedIdentitySet = false, bool pStripLiteralConjuncts = false, bool remove_state_impasse = false, bool* removed_goal = NULL, bool* removed_impasse = NULL);

bool add_test(agent* thisAgent, test* dest_address, test new_test, bool merge_disjunctions = false);
void add_test_if_not_already_there(agent* thisAgent, test* t, test new_test, bool neg, bool merge_disjunctions = false);

cons* delete_test_from_conjunct(agent* thisAgent, test* t, cons* pDeleteItem);

/* --- Some functions related to tests that used to be in rete.cpp */

void add_hash_info_to_id_test(agent* thisAgent, condition* cond, byte field_num, rete_node_level levels_up);
void add_identity_to_original_id_test(agent* thisAgent, condition* cond, byte field_num, rete_node_level levels_up);
void add_rete_test_list_to_tests(agent* thisAgent, condition* cond, rete_test* rt);
void add_gensymmed_equality_test(agent* thisAgent, test* t, char first_letter);
void add_all_variables_in_test(agent* thisAgent, test t, tc_number tc, cons** var_list);
void add_bound_variables_in_test(agent* thisAgent, test t, tc_number tc, cons** var_list);
void add_bound_variable_with_identity(agent* thisAgent, Symbol* pSym, Symbol* pMatchedSym, uint64_t pIdentity, tc_number tc, matched_symbol_list* var_list);
void copy_non_identical_tests(agent* thisAgent, test* t, test add_me, bool considerIdentity = false);

inline bool test_has_referent(test t)
{
    return ((t->type != CONJUNCTIVE_TEST) && (t->type != GOAL_ID_TEST) &&
            (t->type != IMPASSE_ID_TEST) && (t->type != DISJUNCTION_TEST) &&
            (t->type != SMEM_LINK_UNARY_TEST) && (t->type != SMEM_LINK_UNARY_NOT_TEST));
};

inline bool test_can_be_transitive_constraint(test t)
{
    return ((t->type != EQUALITY_TEST) && (t->type != CONJUNCTIVE_TEST) &&
            (t->type != GOAL_ID_TEST) && (t->type != IMPASSE_ID_TEST));
};

#endif /* TEST_H_ */
