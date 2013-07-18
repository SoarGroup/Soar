#include <portability.h>
/* -------------------------------------------------------------------
                              test.h
                          Utilities for tests
------------------------------------------------------------------- */
#ifndef TEST_H_
#define TEST_H_


//#include "gdatastructs.h"
#include "kernel.h"

/* -------------------------------------------------------------------
                              Tests

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
template <typename T> inline void allocate_cons(agent* thisAgent, T * dest_cons_pointer);

/* --- Test struct stores information about all test types, including
 *     equality tests.  If nil, the test is considered blank.
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
    Symbol *referent;         /* for relational tests */
    ::list *disjunction_list;   /* for disjunction tests */
    ::list *conjunct_list;      /* for conjunctive tests */
  } data;
  test_struct *original_test;

} test_info;

/* --- Note that the test typedef is a *pointer* to a test struct. A test is
 *     considered blank when that pointer is nil. --- */
typedef test_info * test;


/* --- Descriptions of these functions can be found in the test.cpp --- */
inline bool test_is_blank(test t){return (t == 0);}
inline bool test_is_variable(agent* thisAgent, test t);
inline test make_blank_test() {return static_cast<test>(0);}

char first_letter_from_test (test t);
bool tests_are_equal (test t1, test t2, bool neg);
bool test_includes_equality_test_for_symbol (test t, Symbol *sym);
bool test_includes_goal_or_impasse_id_test (test t, bool look_for_goal, bool look_for_impasse);
test copy_of_equality_test_found_in_test (agent* thisAgent, test t);

test make_test(agent* thisAgent, Symbol * sym, TestType test_type);
uint32_t hash_test (agent* thisAgent, test t);
void deallocate_test (agent* thisAgent, test t, long indent=0);

test copy_test (agent* thisAgent, test t);
test copy_test_removing_goal_impasse_tests (agent* thisAgent, test t, bool *removed_goal, bool *removed_impasse);


#ifdef DEBUG_TRACE_ADD_TEST_TO_TEST
void add_new_test_to_test_func (agent* thisAgent, test *t, test add_me, test add_me_original=NULL);
#define add_new_test_to_test(thisAgent, t, add_me, add_me_original) \
        do { \
                printf("Debug | add_new_test_to_test called from %s\n", __func__); \
                add_new_test_to_test_func(thisAgent, t, add_me, add_me_original); \
        } while (0)
#else
void add_new_test_to_test (agent* thisAgent, test *t, test add_me, test add_me_original=NULL);
#endif
void add_new_test_to_test_if_not_already_there (agent* thisAgent, test *t, test add_me, bool neg);

/* --- Some functions related to tests that used to be in rete.cpp */

void add_additional_tests_and_originals (agent *thisAgent, rete_node *node, wme *right_wme, condition *cond, node_varnames *nvn);
void add_hash_info_to_id_test (agent* thisAgent, condition *cond, byte field_num, rete_node_level levels_up);
void add_hash_info_to_original_id_test (agent* thisAgent, condition *cond, byte field_num, rete_node_level levels_up);
void add_rete_test_list_to_tests (agent* thisAgent, condition *cond, rete_test *rt);
void add_gensymmed_equality_test (agent* thisAgent, test *t, char first_letter);
void add_varnames_to_test (agent* thisAgent, varnames *vn, test *t, bool force_unique = false);
void add_all_variables_in_test (agent* thisAgent, test t, tc_number tc, list **var_list);
void add_bound_variables_in_test (agent* thisAgent, test t, tc_number tc, ::list **var_list);

//extern const char *test_type_to_string(byte test_type);
extern void print_test (agent* thisAgent, test t, const char *indent_string = "        ", const char *conj_indent_string = "+ ");
extern void print_test_brief (agent* thisAgent, test t, bool trailing_space=true);

#endif /* TEST_H_ */
