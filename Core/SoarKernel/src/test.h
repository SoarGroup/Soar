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
typedef uint64_t tc_number;
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
 *    Note that conjunctive tests always have a NULL original_test.  Each
 *    constituent test of the conjunctive test contains links to its original
 *    test already --*/

typedef struct test_struct
{
    TestType        type;                  /* see definitions in enums.h */
    union test_info_union
    {
        Symbol*        referent;         /* for relational tests */
        ::list*        disjunction_list;   /* for disjunction tests */
        ::list*        conjunct_list;      /* for conjunctive tests */
    } data;
    test_struct*     eq_test;
    tc_number        tc_num;
    uint64_t         identity;
    test_struct() : type(NUM_TEST_TYPES), eq_test(NULL), tc_num(0), identity(0)
    {
        data.referent = NULL;
    }
} test_info;

/* --- Note that the test typedef is a *pointer* to a test struct. A test is
 *     considered blank when that pointer is nil. --- */
typedef test_info* test;

/* ----------------------------------------------------------------
   Returns true iff the test contains a test for a variable
   symbol.  Assumes test is not a conjunctive one and does not
   try to search them.
---------------------------------------------------------------- */
inline bool test_has_referent(test t)
{
    return ((t->type != DISJUNCTION_TEST) && (t->type != GOAL_ID_TEST) &&
            (t->type != IMPASSE_ID_TEST) && (t->type != CONJUNCTIVE_TEST));
};


/* --- Descriptions of these functions can be found in the test.cpp --- */
char first_letter_from_test(test t);
bool tests_are_equal(test t1, test t2, bool neg);
bool tests_identical(test t1, test t2, bool considerIdentity = false);
bool test_includes_equality_test_for_symbol(test t, Symbol* sym);
bool test_includes_goal_or_impasse_id_test(test t, bool look_for_goal, bool look_for_impasse);
bool test_is_variable(agent* thisAgent, test t);
test copy_of_equality_test_found_in_test(agent* thisAgent, test t);
void cache_eq_test(test t);
test equality_test_found_in_test(test t);
test equality_var_test_found_in_test(test t);
test find_equality_test_preferring_vars(test t);
test find_original_equality_test_preferring_vars(test t);

test make_test(agent* thisAgent, Symbol* sym, TestType test_type);
uint32_t hash_test(agent* thisAgent, test t);
void deallocate_test(agent* thisAgent, test t);

test copy_test(agent* thisAgent, test t, bool pUnify_variablization_identity = false);
test copy_test_removing_goal_impasse_tests(agent* thisAgent, test t, bool* removed_goal, bool* removed_impasse);
test copy_test_without_relationals(agent* thisAgent, test t);

void add_test(agent* thisAgent, test* dest_address, test new_test);
void add_test_if_not_already_there(agent* thisAgent, test* t, test new_test, bool neg);

::list* delete_test_from_conjunct(agent* thisAgent, test* t, ::list* pDeleteItem);

/* --- Some functions related to tests that used to be in rete.cpp */

void add_constraints_and_identities(agent* thisAgent, rete_node* node, condition* cond, wme* w, node_varnames* nvn, uint64_t pI_id, AddAdditionalTestsMode additional_tests);
//void set_identity_for_rule_variable(agent* thisAgent, test t, Symbol* pRuleSym, uint64_t pI_id);
void add_hash_info_to_id_test(agent* thisAgent, condition* cond, byte field_num, rete_node_level levels_up);
void add_hash_info_to_original_id_test(agent* thisAgent, condition* cond, byte field_num, rete_node_level levels_up);
void add_rete_test_list_to_tests(agent* thisAgent, condition* cond, rete_test* rt);
void add_gensymmed_equality_test(agent* thisAgent, test* t, char first_letter);
void add_all_variables_in_test(agent* thisAgent, test t, tc_number tc, list** var_list);
void add_bound_variables_in_test(agent* thisAgent, test t, tc_number tc, ::list** var_list);
void copy_non_identical_tests(agent* thisAgent, test* t, test add_me, bool considerIdentity = false);
/* UITODO| Make this method of Test */
const char* test_type_to_string(byte test_type);

#endif /* TEST_H_ */
