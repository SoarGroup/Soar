#include <portability.h>
/* -------------------------------------------------------------------
                              test.h
                          Utilities for tests
------------------------------------------------------------------- */
#ifndef TEST_H_
#define TEST_H_


#include "gdatastructs.h"

/* --- Descriptions of these functions can be found in the test.cpp --- */
char first_letter_from_test (test t);
inline Bool test_is_blank(test t){return (t == 0);}
inline Bool test_is_variable(agent* thisAgent, test t);
Bool tests_are_equal (test t1, test t2, bool neg);
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

/* --- Some functions related to tests that used to be in rete.cpp
 *     and the forward declarations they need. --- */

typedef struct node_varnames_struct node_varnames;
typedef struct condition_struct condition;
typedef struct wme_struct wme;
typedef struct rete_node_struct rete_node;
typedef struct rete_test_struct rete_test;
typedef char varnames;
typedef unsigned short rete_node_level;

void add_additional_tests_and_originals (agent *thisAgent, rete_node *node, wme *right_wme, condition *cond, node_varnames *nvn);
void add_hash_info_to_id_test (agent* thisAgent, condition *cond, byte field_num, rete_node_level levels_up);
void add_hash_info_to_original_id_test (agent* thisAgent, condition *cond, byte field_num, rete_node_level levels_up);
void add_rete_test_list_to_tests (agent* thisAgent, condition *cond, rete_test *rt);
void add_gensymmed_equality_test (agent* thisAgent, test *t, char first_letter);
void add_varnames_to_test (agent* thisAgent, varnames *vn, test *t, bool force_unique = false);
void add_all_variables_in_test (agent* thisAgent, test t, tc_number tc, list **var_list);
void add_bound_variables_in_test (agent* thisAgent, test t, tc_number tc, ::list **var_list);

#endif /* TEST_H_ */
