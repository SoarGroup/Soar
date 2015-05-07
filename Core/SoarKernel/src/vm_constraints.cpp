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
#include "wmem.h"
#include "print.h"
#include "debug.h"

/*    Note: We do not need to deallocate the symbol key in any of the following code b/c we did not
 *          increase the refcount on those symbols.  We do need to deallocate the test b/c it
 *          was a copy of another test in another instantiation. */
void Variablization_Manager::variablize_relational_constraints()
{
    variablization* found_variablization = NULL;
    ::list* c;
    std::map< Symbol*, ::list* >* variablized_sti_constraints = new std::map< Symbol*, ::list* >;
    std::map< uint64_t, ::list* >* variablized_constant_constraints = new std::map< uint64_t, ::list* >;

    dprint_header(DT_LHS_VARIABLIZATION, PrintBefore, "Variablizing relational constraints.\n");
    dprint(DT_LHS_VARIABLIZATION, "(1) Variablizing relational constraints for short-term identifiers.\n");
//    dprint(DT_LHS_VARIABLIZATION, "%8"); // Prints all wmes with identities


    /* -- Replace sti constraints with variablized version and delete any ungrounded tests and
     *    any constraints whose symbol key has not been variablized during the equality
     *    variablization pass. -- */
    for (std::map< Symbol*, ::list* >::iterator it = sti_constraints->begin(); it != sti_constraints->end(); ++it)
    {

        dprint(DT_LHS_VARIABLIZATION, "Looking for variablization for equality symbol %y.\n", it->first);
        found_variablization = get_variablization(it->first);

        if (found_variablization)
        {
            dprint(DT_LHS_VARIABLIZATION, "...found variablization.  Variablizing constraint list.\n");

            variablize_cached_constraints_for_symbol(&(it->second));

            /* -- If at least one relational constraint remains in the list, add to variablized constraint
             *    list, using the variablized equality symbol -- */
            if (it->second)
            {
                dprint(DT_LHS_VARIABLIZATION, "...variablized constraints exist.  Copying to new constraint list.\n");
                (*variablized_sti_constraints)[found_variablization->variablized_symbol] = it->second;
            }
        }
        else
        {
            /* -- Delete entire constraint list for ungrounded identifier */
            dprint(DT_LHS_VARIABLIZATION, "...not variablizing constraint list b/c equality symbol not in chunk.  Deallocating tests.\n");
            c = it->second;
            while (c)
            {
                dprint(DT_LHS_VARIABLIZATION, "...deallocating test %t\n", static_cast<test>(c->first));
                deallocate_test(thisAgent, static_cast<test>(c->first));
                c = c->rest;
            }
            free_list(thisAgent, it->second);
            it->second = NULL;
        }
    }

    sti_constraints->clear();
    sti_constraints = variablized_sti_constraints;

    /* -- Replace constant constraints with variablized version.  Delete only constraints whose symbol key
     *    has not been variablized during the equality variablization pass. -- */
    dprint(DT_LHS_VARIABLIZATION, "(2) Variablizing relational constraints for constant symbols.\n");
    for (std::map< uint64_t, ::list* >::iterator it = constant_constraints->begin(); it != constant_constraints->end(); ++it)
    {

        dprint(DT_LHS_VARIABLIZATION, "Looking for variablization for o%u.\n", it->first);
        found_variablization = get_variablization(it->first);

        if (found_variablization)
        {
            dprint(DT_LHS_VARIABLIZATION, "...found variablization for o%u.  Variablizing constraint list.\n", it->first);
            variablize_cached_constraints_for_symbol(&(it->second));

            /* -- If at least one relational constraint remains in the list, add to variablized constraint
             *    list, using the variablized equality symbol -- */
            /* MToDo | Don't think this is possible for constants  Checking.  Remove */
            assert(it->second);
            if (it->second)
            {
                dprint(DT_LHS_VARIABLIZATION, "...variablized constraints exist.  Copying to new constraint list.\n");
//                (*variablized_constant_constraints)[found_variablization->grounding_id] = it->second;
            }
        }
        else
        {
            /* -- Delete entire constraint list for ungrounded identifier -- */
            dprint(DT_LHS_VARIABLIZATION, "...not variablizing constraint list b/c o_id not in positive test of chunk.  Deallocating tests.\n");
            c = it->second;
            while (c)
            {
                dprint(DT_LHS_VARIABLIZATION, "...deallocating test %t\n", static_cast<test>(c->first));
                deallocate_test(thisAgent, static_cast<test>(c->first));
                c = c->rest;
            }
            free_list(thisAgent, it->second);
            it->second = NULL;
        }
    }

    /* -- Replace constant constraints with variablized version. -- */
    constant_constraints->clear();
    constant_constraints = variablized_constant_constraints;

    dprint(DT_LHS_VARIABLIZATION, "Done variablizing relational constraints.\n");

}

void Variablization_Manager::variablize_cached_constraints_for_symbol(::list** constraint_list)
{
    variablization* found_variablization = NULL;
    cons* c, *c_next, *c_last;
    test t;
    bool success;

    /* MToDo | Remove.  Quick check to see if it is hanging here somehow. */
    int64_t debug_count = 0;

    c = (*constraint_list);
    c_last = NULL;
    while (c)
    {
        assert(++debug_count < 100);
        c_next = c->rest;
        t = static_cast<test>(c->first);
        // Should not be possible to be a conjunctive test
        assert(t->type != CONJUNCTIVE_TEST);
        /* MToDo | This check was needed when we added literal equality tests to cached constraints
         *         Probably won't need, but leaving in for now. Note that having it also caused
         *         a memory access error, but did not investigate. */
//        if (t->identity->original_var)
//        {
            success = variablize_test_by_lookup(&(t), false);
            if (!success)
            {
                /* -- STI identifier that is ungrounded.  Delete. -- */
                dprint(DT_LHS_VARIABLIZATION, "Deleting constraint b/c STI not in in chunk.\n");
                if (c_last)
                {
                    /* -- Not at the head of the list -- */
                    c_last->rest = c->rest;
                    free_cons(thisAgent, c);
                    c = c_last;
                }
                else
                {
                    /* -- At the head of the list -- */
                    (*constraint_list) = c->rest;
                    free_cons(thisAgent, c);
                    /* -- This will cause c_last to be set to NULL, indicating we're
                     *    at the head of the list -- */
                    c = NULL;
                }
                deallocate_test(thisAgent, t);
            }
//        } else {
//            dprint(DT_LHS_VARIABLIZATION, "Will not attempt to variablize cached constraint b/c no original variable.\n");
//        }
        c_last = c;
        c = c_next;
    }
}

void Variablization_Manager::clear_cached_constraints()
{
    for (std::map< Symbol*, ::list* >::iterator it = sti_constraints->begin(); it != sti_constraints->end(); ++it)
    {
        free_list(thisAgent, it->second);
    }
    sti_constraints->clear();

    for (std::map< uint64_t, ::list* >::iterator it = constant_constraints->begin(); it != constant_constraints->end(); ++it)
    {
        free_list(thisAgent, it->second);
    }
    constant_constraints->clear();

    for (std::list< constraint* >::iterator it = constraints->begin(); it != constraints->end(); ++it)
    {
        /* We intentionally used the tests in the conditions backtraced through instead of copying
         * them, so we don't need to deallocate the tests in the constraint. We just delete the
         * constraint struct that contains the two pointers.*/
        delete *it;
    }
    constraints->clear();
}

void Variablization_Manager::cache_constraint(test equality_test, test relational_test)
{
    dprint(DT_CONSTRAINTS, "Adding relational constraint %t to %t.\n", relational_test, equality_test);
    ::list* new_list = NULL;
//    test copied_test = copy_test(thisAgent, relational_test);
    test copied_test = relational_test;

    if (equality_test->data.referent->is_sti())
    {
        std::map< Symbol*, ::list* >::iterator iter = (*sti_constraints).find(equality_test->data.referent);
        if (iter == sti_constraints->end())
        {
            push(thisAgent, (copied_test), new_list);
            (*sti_constraints)[equality_test->data.referent] = new_list;
            dprint(DT_CONSTRAINTS, "ADDED (*sti_constraints)[%y] + %t\n", equality_test->data.referent, copied_test);
        }
        else
        {
            new_list = (*sti_constraints)[equality_test->data.referent];
            push(thisAgent, (copied_test), new_list);
            (*sti_constraints)[equality_test->data.referent] = new_list;
            dprint(DT_CONSTRAINTS, "ADDED (*sti_constraints)[%y] + %t\n", equality_test->data.referent, copied_test);
        }
    }
    else
    {
        std::map< uint64_t, ::list* >::iterator iter = (*constant_constraints).find(equality_test->identity->o_id);
        if (iter == constant_constraints->end())
        {
            push(thisAgent, (copied_test), new_list);
            (*constant_constraints)[equality_test->identity->o_id] = new_list;
            dprint(DT_CONSTRAINTS, "ADDED (*constant_constraints)[o%u] + %t\n", equality_test->identity->o_id, copied_test);
        }
        else
        {
            new_list = (*constant_constraints)[equality_test->identity->o_id];
            push(thisAgent, (copied_test), new_list);
            (*constant_constraints)[equality_test->identity->o_id] = new_list;
            dprint(DT_CONSTRAINTS, "ADDED (*constant_constraints)[o%u] + %t\n", equality_test->identity->o_id, copied_test);
        }
    }
}

void Variablization_Manager::cache_constraints_in_test(test t)
{
    /* -- Only conjunctive tests can have relational tests here.  Otherwise,
     *    should be an equality test. -- */
    if (t->type != CONJUNCTIVE_TEST)
    {
        assert(t->type == EQUALITY_TEST);
        return;
    }

    test equality_test = NULL, referent_test, ctest;
    cons* c;
    for (c = t->data.conjunct_list; c != NIL; c = c->rest)
    {
        if (static_cast<test>(c->first)->type == EQUALITY_TEST)
        {
            equality_test = static_cast<test>(c->first);
            break;
        }
    }
    assert(equality_test);
    constraint* new_constraint = NULL;
    for (c = t->data.conjunct_list; c != NIL; c = c->rest)
    {
        ctest = static_cast<test>(c->first);
        switch (ctest->type)
        {
            case GREATER_TEST:
            case GREATER_OR_EQUAL_TEST:
            case LESS_TEST:
            case LESS_OR_EQUAL_TEST:
            case NOT_EQUAL_TEST:
            case SAME_TYPE_TEST:
            case DISJUNCTION_TEST:
                cache_constraint(equality_test, ctest);
                new_constraint = new constraint(equality_test, ctest);
                constraints->push_back(new_constraint);
                break;
            default:
                break;
        }
    }
}

void Variablization_Manager::cache_constraints_in_cond(condition* c)
{
    /* MToDo| Verify we don't need to do id element.  It should always be an equality test */
    //  assert(!c->data.tests.id_test || (c->data.tests.id_test->type == EQUALITY_TEST));
    dprint(DT_CONSTRAINTS, "Caching relational constraints in condition: %l\n", c);
    /* MToDo| Re-enable attribute constraint caching here.  Disabled just to simplify debugging for now */
    cache_constraints_in_test(c->data.tests.attr_test);
    cache_constraints_in_test(c->data.tests.value_test);
}

void Variablization_Manager::install_cached_constraints_for_test(test* t)
{
    if (!t)
    {
        return;
    }

    cons* c;
    test eq_test, ct;
    Symbol* eq_symbol;
    variablization* found_variablization;

    eq_test = equality_test_found_in_test(*t);
    assert(eq_test);
    eq_symbol = eq_test->data.referent;
    dprint(DT_CONSTRAINTS, "Calling add_relational_constraints_for_test() for symbol %y(%u).\n", eq_symbol, eq_test->identity ? eq_test->identity->o_id : 0);
    if (!eq_test->identity || (eq_test->identity->o_id == 0))
    {
        dprint(DT_CONSTRAINTS, "...no identity, so must be STI.  Using symbol to look up.\n");
        found_variablization = get_variablization(eq_symbol);
        if (found_variablization)
        {
            dprint(DT_CONSTRAINTS, "...variablization found.  Variablized symbol = %y.\n", found_variablization->variablized_symbol);
            print_cached_constraints(DT_CONSTRAINTS);
            std::map< Symbol*, ::list* >::iterator iter = (*sti_constraints).find(eq_symbol);
            if (iter != (*sti_constraints).end())
            {
                dprint(DT_CONSTRAINTS, "...adding relational constraint list for symbol %y...\n", eq_symbol);
                c = iter->second;
                while (c)
                {
                    ct = static_cast<test>(c->first);
                    dprint(DT_CONSTRAINTS, "...adding %t\n", ct);
                    add_test(thisAgent, t, ct);
                    c = c->rest;
                }
                free_list(thisAgent, iter->second);
                (*sti_constraints).erase(iter->first);
                dprint(DT_CONSTRAINTS, "...final constrained test: %t\n", *t);
            }
            else
            {
                dprint(DT_CONSTRAINTS, "...no relational constraints found.\n");
            }
        }
        else
        {
            dprint(DT_CONSTRAINTS, "... was never variablized. Skipping...\n");
        }
    }
    else
    {
        dprint(DT_CONSTRAINTS, "...identity exists, so must be constant.  Using o_id to look up.\n");
        found_variablization = get_variablization(eq_test->identity->o_id);
        if (found_variablization)
        {
            dprint(DT_CONSTRAINTS, "...variablization found.  Variablized symbol = %y.\n", found_variablization->variablized_symbol);
            print_cached_constraints(DT_CONSTRAINTS);
            std::map< uint64_t, ::list* >::iterator iter = (*constant_constraints).find(eq_test->identity->o_id);
            if (iter != (*constant_constraints).end())
            {
                dprint(DT_CONSTRAINTS, "...adding relational constraint list for symbol %y...\n", eq_symbol);
                c = iter->second;
                while (c)
                {
                    ct = static_cast<test>(c->first);
                    dprint(DT_CONSTRAINTS, "...adding %t\n", ct);
                    add_test(thisAgent, t, ct);
                    c = c->rest;
                }
                free_list(thisAgent, iter->second);
                (*constant_constraints).erase(iter->first);
                dprint(DT_CONSTRAINTS, "...final constrained test: %t\n", *t);
            }
            else
            {
                dprint(DT_CONSTRAINTS, "...no relational constraints found.\n");
            }
        }
        else
        {
            dprint(DT_CONSTRAINTS, "... was never variablized. Skipping...\n");
        }
    }
}

void Variablization_Manager::install_cached_constraints(condition* cond)
{
    dprint_header(DT_CONSTRAINTS, PrintBefore, "install_relational_constraints called...\n");
    print_variablization_tables(DT_CONSTRAINTS);
    print_cached_constraints(DT_CONSTRAINTS);

    /* MToDo | Vast majority of constraints will be on value element.  Making this work with a pass for
     *         values followed by attributes could be faster. */

    while (cond && ((sti_constraints->size() > 0) || (constant_constraints->size() > 0)))
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            dprint(DT_CONSTRAINTS, "Adding for positive condition %l\n", cond);
            install_cached_constraints_for_test(&cond->data.tests.attr_test);
            install_cached_constraints_for_test(&cond->data.tests.value_test);
        }
        else
        {
            dprint(DT_CONSTRAINTS, (cond->type == NEGATIVE_CONDITION) ? "Skipping for negative condition %l\n" : "Skipping for negative conjunctive condition:\n%l", cond);
        }
        cond = cond->next;
    }
    dprint(DT_CONSTRAINTS,
        (!sti_constraints->size() && !constant_constraints->size()) ?
        "All constraints were added.  Final tables:\n" :
        "Some constraints could not be attached!!!  Final tables:\n");
    print_variablization_tables(DT_CONSTRAINTS);
    print_cached_constraints(DT_CONSTRAINTS);
    dprint_noprefix(DT_CONSTRAINTS, "%1", cond);
    dprint_header(DT_CONSTRAINTS, PrintAfter, "");
}

void Variablization_Manager::propagate_constraint_identities(uint64_t pI_id)
{
    std::map< uint64_t, ::list* >::iterator it;
    test new_ct;
    cons* c;

    for (it = constant_constraints->begin(); it != constant_constraints->end(); ++it)
    {
        c = it->second;
        while (c)
        {
            new_ct = static_cast<test>(c->first);
            dprint(DT_CONSTRAINTS, "...updating identity for constraint %t [%g]\n", new_ct, new_ct);
            if (new_ct->identity->o_id)
            {
                thisAgent->variablizationManager->unify_identity(thisAgent, new_ct);
                /* At this point, we can also generate new o_ids for the chunk.  They currently have o_ids that came from the
                 * conditions of the rules backtraced through and any unifications that occurred.  pI_id should only be
                 * 0 in the case of reinforcement rules being created.  (I think they're different b/c rl is creating
                 * rules that do not currently match unlike chunks/justifications) */
                if (new_ct->identity->o_id && pI_id)
                {
                    dprint(DT_FIX_CONDITIONS, "Creating new o_ids and o_vars for chunk using o%u(%y) for i%u.\n", new_ct->identity->o_id, new_ct->identity->rule_symbol, pI_id);
                    //                        old_o_id = new_ct->identity->original_var_id;
                    thisAgent->variablizationManager->update_o_id_for_new_instantiation(&(new_ct->identity->rule_symbol), &(new_ct->identity->o_id), pI_id);
                    dprint(DT_FIX_CONDITIONS, "Test after ovar update is now %t [%g].\n", new_ct, new_ct);
                }
            }
            c = c->rest;
        }
    }
}

attachment_point* Variablization_Manager::get_attachment_point(uint64_t pO_id)
{
    std::map< uint64_t, attachment_point* >::iterator it = (*attachment_points).find(pO_id);
    if (it != (*attachment_points).end())
    {
        dprint(DT_CONSTRAINTS, "...found attachment point: %y(o%u) -> %s of %l\n",
            get_ovar_for_o_id(it->first), it->first, field_to_string(it->second->field), it->second->cond);

        return it->second;
    } else {
        dprint(DT_CONSTRAINTS, "...did not find attachment point for %y(o%u)!\n",
            get_ovar_for_o_id(it->first), it->first);
        print_o_id_update_map(DT_VM_MAPS);
    }
    return 0;
}

void Variablization_Manager::set_attachment_point(uint64_t pO_id, condition* pCond, WME_Field pField)
{
    std::map< uint64_t, attachment_point* >::iterator it = (*attachment_points).find(pO_id);
    if (it == (*attachment_points).end())
    {
        dprint(DT_CONSTRAINTS, "Skipping because existing attachment already exists: %y(o%u) -> %s of %l\n",
            get_ovar_for_o_id(it->first), it->first, field_to_string(it->second->field), it->second->cond);
        return;
    }

    dprint(DT_CONSTRAINTS, "Recording attachment point: %y(o%u) -> %s of %l\n",
        get_ovar_for_o_id(pO_id), pO_id, field_to_string(pField), pCond);
    (*attachment_points)[pO_id] = new attachment_point(pCond, pField);;
}

void Variablization_Manager::find_attachment_point_for_test(test pTest, condition* pCond, WME_Field pField)
{
    test lTest = equality_test_found_in_test(pTest);
    if (lTest && lTest->identity->o_id)
    {
        set_attachment_point(lTest->identity->o_id, pCond, pField);
    }
}

void Variablization_Manager::find_attachment_points(condition* pCond)
{
    dprint_header(DT_CONSTRAINTS, PrintBefore, "Scanning conditions for constraint attachment points...\n%1", pCond);

    while (pCond)
    {
        if (pCond->type == POSITIVE_CONDITION)
        {
            dprint(DT_CONSTRAINTS, "Adding attachment points for positive condition %l\n", pCond);
            find_attachment_point_for_test(pCond->data.tests.attr_test, pCond, ATTR_ELEMENT);
            find_attachment_point_for_test(pCond->data.tests.value_test, pCond, VALUE_ELEMENT);
        }
        else
        {
            dprint(DT_CONSTRAINTS, (pCond->type == NEGATIVE_CONDITION) ?
                "Skipping for negative condition %l\n" :
                "Skipping for negative conjunctive condition:\n%l", pCond);
        }
        pCond = pCond->next;
    }
    dprint_header(DT_CONSTRAINTS, PrintAfter, "Done scanning conditions for attachment points.");
}

void Variablization_Manager::invert_relational_test(test* pEq_test, test* pRelational_test)
{
    assert(test_has_referent(*pEq_test));
    assert(test_has_referent(*pRelational_test));
    assert((*pEq_test)->type != EQUALITY_TEST);
    assert((*pRelational_test)->type != EQUALITY_TEST);

    TestType tt = (*pRelational_test)->type;
    if (tt == NOT_EQUAL_TEST)
    {
        (*pEq_test)->type = NOT_EQUAL_TEST;
    }
    else if (tt == LESS_TEST)
    {
        (*pEq_test)->type = GREATER_TEST;
    }
    else if (tt == GREATER_TEST)
    {
        (*pEq_test)->type = LESS_TEST;
    }
    else if (tt == LESS_OR_EQUAL_TEST)
    {
        (*pEq_test)->type = GREATER_OR_EQUAL_TEST;
    }
    else if (tt == GREATER_OR_EQUAL_TEST)
    {
        (*pEq_test)->type = LESS_OR_EQUAL_TEST;
    }
    else if (tt == SAME_TYPE_TEST)
    {
        (*pEq_test)->type = SAME_TYPE_TEST;
    }
    (*pRelational_test)->type = EQUALITY_TEST;

    test temp = *pEq_test;
    pEq_test = pRelational_test;
    (*pRelational_test) = temp;

}

void Variablization_Manager::attach_relational_test(test pEq_test, test pRelational_test, uint64_t pI_id)
{
    o_id_update_info* new_o_id_info = get_updated_o_id_info(pEq_test->identity->o_id);
    if (new_o_id_info)
    {
        assert(new_o_id_info->positive_cond);
        if (new_o_id_info->field == VALUE_ELEMENT)
        {
            add_test(thisAgent, &(new_o_id_info->positive_cond->data.tests.value_test), pRelational_test);
        } else if (new_o_id_info->field == ATTR_ELEMENT)
        {
            add_test(thisAgent, &(new_o_id_info->positive_cond->data.tests.attr_test), pRelational_test);
        } else
        {
            assert(false);
            add_test(thisAgent, &(new_o_id_info->positive_cond->data.tests.id_test), pRelational_test);
        }
//        dprint(DT_CONSTRAINTS, "...found test to attach to %t[%g]\n", attach_test, attach_test);
//        new_o_id_info = get_updated_o_id_info(pRelational_test->identity->o_id);
//        if (new_o_id_info)
//        {
//        }
//        thisAgent->variablizationManager->update_o_id_for_new_instantiation(&(new_ct->identity->rule_symbol), &(new_ct->identity->o_id), pI_id, t);
        return;
    }
    assert(false);
}

void Variablization_Manager::prune_redundant_constraints()
{
    dprint(DT_CONSTRAINTS, "Pruning redundant constraints from set of size %u.\n", static_cast<uint64_t>(constraints->size()));
    for (std::list< constraint* >::iterator iter = constraints->begin(); iter != constraints->end();)
    {
        if ((*iter)->constraint_test->tc_num == tc_num_found)
        {
            iter = constraints->erase(iter);
        }
        else
        {
            ++iter;
        }
    }
    dprint(DT_CONSTRAINTS, "Final pruned constraints is a set of size %u.\n", static_cast<uint64_t>(constraints->size()));
}

void Variablization_Manager::propagate_additional_constraints(condition* cond, uint64_t pI_id)
{
    constraint* lConstraint = NULL;
    test eq_copy = NULL, constraint_test = NULL;

    dprint_header(DT_CONSTRAINTS, PrintBefore, "Propagating additional constraints...\n");

    prune_redundant_constraints();
    if (constraints->empty())
    {
        dprint_header(DT_CONSTRAINTS, PrintAfter, "All constraints already in chunk conditions.  Done propagating additional constraints.\n");
        return;
    }

    return;
    find_attachment_points(cond);

    for (std::list< constraint* >::iterator iter = constraints->begin(); iter != constraints->end(); ++iter)
    {
        lConstraint = *iter;
        if (lConstraint->constraint_test->tc_num != tc_num_found)
        {
            constraint_test = copy_test(thisAgent, lConstraint->constraint_test);
            eq_copy = copy_test(thisAgent, lConstraint->eq_test);
            if (eq_copy->identity->o_id)
            {
                thisAgent->variablizationManager->unify_identity(thisAgent, eq_copy);
            }
            if (eq_copy->identity->o_id)
            {
                thisAgent->variablizationManager->unify_identity(thisAgent, eq_copy);
            }

//            new_test = copy_test(thisAgent, lConstraint->constraint_test, true, pI_id);
//            eq_copy = copy_test(thisAgent, lConstraint->eq_test, true, pI_id);
//            update_o_id_for_new_instantiation(&(constraint_test->identity->rule_symbol), &(constraint_test->identity->o_id), pI_id, NULL, true);
//            update_o_id_for_new_instantiation(&(eq_copy->identity->rule_symbol), &(eq_copy->identity->o_id), pI_id, NULL, true);
            dprint(DT_CONSTRAINTS, "...unattached test found: %t[%g] %t[%g]\n", eq_copy, eq_copy, constraint_test, constraint_test);

            if (eq_copy->identity->o_id)
            {
                /* Attach to a positive chunk condition test of eq_test */
                dprint(DT_CONSTRAINTS, "...equality test has an identity, so attaching.\n");
                attach_relational_test(eq_copy, constraint_test, pI_id);
            } else {
                if (constraint_test->identity->o_id)
                {
                    /* Original identity it was attached to was literalized, but the relational
                     * referent was not, so make complement and add to a positive chunk
                     * condition test for the referent */
                    dprint(DT_CONSTRAINTS, "...equality test is a literal but referent has identity, so attaching complement to referent.\n");
                    invert_relational_test(&eq_copy, &constraint_test);
                    attach_relational_test(eq_copy, constraint_test, pI_id);

                } else {
                    // Both tests are literals.  Delete.
                    dprint(DT_CONSTRAINTS, "...both tests are literals.  Oh my god.\n");
                    deallocate_test(thisAgent, constraint_test);
                    // Is this possible?
                    assert(false);
                }
            }
            /* eq_test no longer needed so deallocate.  relational test now attached */
            deallocate_test(thisAgent, eq_copy);
        } else {
            // Skipping test because marked as already in a chunk condition
            dprint(DT_CONSTRAINTS, "Skipping test b/c marked as already in chunk %g %g...\n", lConstraint->eq_test, lConstraint->constraint_test);
        }
    }
    dprint_header(DT_CONSTRAINTS, PrintAfter, "Done propagating additional constraints.\n");
}
