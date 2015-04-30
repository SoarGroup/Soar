/*
 * variablization_manager_constraints.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "variablization_manager.h"
#include "agent.h"
#include "instantiations.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "debug.h"

void Variablization_Manager::install_literal_constraints_for_test(test* t)
{
    if (!t)
    {
        return;
    }

    Symbol* t_symbol;
    uint64_t lO_id;

    variablization *found_variablization;

    if ((*t)->type == CONJUNCTIVE_TEST)
    {
        dprint(DT_LITERALIZATION, "Installing literal constraints for conjunctive test %t...\n", *t);
        cons* c;
        for (c = (*t)->data.conjunct_list; c != NIL; c = c->rest)
        {
            test ctest = static_cast<test>(c->first);
            install_literal_constraints_for_test(&ctest);
            c->first = ctest;
        }
        return;
    }
    if (!test_has_referent(*t)) return;

    t_symbol = (*t)->data.referent;
    lO_id = (*t)->identity->original_var_id;
    dprint(DT_LITERALIZATION, "Installing literal constraints for test %y in test %t.\n", t_symbol, *t);
    if (lO_id == 0)
    {
        dprint(DT_LITERALIZATION, "...no identity, so must be STI or literal relational test.  Skipping.\n");
    }
    else
    {
        dprint(DT_LITERALIZATION, "...identity exists, so must be constant.  Using g_id to look up.\n");
        /* MToDo | The following may speed things up since the variablization
         *         table should be much smaller than the literal constraint table */
//        found_variablization = get_variablization(t_gid);
//        if (found_variablization)
//        {
//            dprint(DT_LITERALIZATION, "...grounding id %u was variablized, looking for literal constraint...\n", t_gid);
            std::map< uint64_t, test >::iterator iter = (*literal_constraints).find(lO_id);
            if (iter != (*literal_constraints).end())
            {
                dprint(DT_LITERALIZATION, "...literal constraint found for g%u: %t. Replacing with literal...\n", lO_id, iter->second);
                deallocate_test(thisAgent, *t);
                test temp = iter->second;
                *t = copy_test(thisAgent, iter->second);
                cache_eq_test(*t);
//                if ((*t)->identity->original_var)
//                {
//                    symbol_remove_ref(thisAgent, (*t)->identity->original_var);
//                    (*t)->identity->original_var = NULL;
//                }
//                /* MToDo | May not need to copy literal test to original test since probably not used after this point*/
//                if ((*t)->original_test)
//                {
//                    deallocate_test(thisAgent, (*t)->original_test);
//                    (*t)->original_test = copy_test(thisAgent, *t);
//                }
                found_variablization = get_variablization(lO_id);
                if (found_variablization)
                {
                    dprint(DT_LITERALIZATION, "...grounding id %u was variablized, marking as literal constraint for RHS...\n", lO_id);
                    found_variablization->variablized_symbol->tc_num = tc_num_literalized;
                }
                dprint(DT_LITERALIZATION, "...final test: %t\n", *t);
            }
            else
            {
                dprint(DT_LITERALIZATION, "...no literal constraints found.\n");
            }
//        }
//        else
//        {
//            dprint(DT_LITERALIZATION, "... was never variablized. Skipping...\n");
//        }
    }
}

void Variablization_Manager::install_literal_constraints(condition* pCond)
{
    dprint_header(DT_LITERALIZATION, PrintBoth, "install_literal_constraints called...!!!!!!!!!!!!!!!!!!!!!!\n");
    print_variablization_tables(DT_LITERALIZATION);
    print_cached_constraints(DT_LITERALIZATION);

    /* MToDo | Vast majority of constraints will be on value element.  Making this work with a pass for
     *         values followed by attributes could be faster. */

    if (literal_constraints->size() > 0)
    {
        while (pCond)
        {
            if (pCond->type == POSITIVE_CONDITION)
            {
                dprint(DT_LITERALIZATION, "Adding for positive condition %l\n", pCond);
                install_literal_constraints_for_test(&pCond->data.tests.attr_test);
                install_literal_constraints_for_test(&pCond->data.tests.value_test);
                dprint(DT_LITERALIZATION, "Resulting in condition %l.\n", pCond);
            } else if (pCond->type == NEGATIVE_CONDITION) {
//                dprint(DT_LITERALIZATION, (pCond->type == NEGATIVE_CONDITION) ? "Skipping for negative condition %l\n" : "Skipping for negative conjunctive condition:\n%l", pCond);
                dprint(DT_LITERALIZATION, "Adding for negative condition: %l\n", pCond);
                install_literal_constraints_for_test(&pCond->data.tests.attr_test);
                install_literal_constraints_for_test(&pCond->data.tests.value_test);
            } else if (pCond->type == CONJUNCTIVE_NEGATION_CONDITION) {
                dprint(DT_LITERALIZATION, "Adding for NCC with recursive call: %l\n", pCond);
                install_literal_constraints(pCond->data.ncc.top);
            }
            pCond = pCond->next;
        }
    }
    dprint(DT_LITERALIZATION, "install_literal_constraints resulted in final condition list %1.\n", pCond);
    dprint_header(DT_LITERALIZATION, PrintAfter, "");
    print_variablization_tables(DT_LITERALIZATION);
    print_cached_constraints(DT_LITERALIZATION);
}
