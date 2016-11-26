#include "condition_record.h"

#include "action_record.h"
#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "instantiation_record.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "symbol_manager.h"
#include "symbol.h"
#include "test.h"
#include "working_memory.h"
#include "visualize.h"

void condition_record::init(agent* myAgent, condition* pCond, uint64_t pCondID)
{
    thisAgent = myAgent;
    conditionID = pCondID;
    type = pCond->type;
    parent_action = NULL;
    path_to_base = NULL;
    my_instantiation = NULL;

    dprint(DT_EXPLAIN_CONDS, "   Creating condition %u for %l.\n", conditionID, pCond);

    condition_tests.id = copy_test(thisAgent, pCond->data.tests.id_test);
    condition_tests.attr = copy_test(thisAgent, pCond->data.tests.attr_test);
    condition_tests.value = copy_test(thisAgent, pCond->data.tests.value_test);

    set_matched_wme_for_cond(pCond);

    if (pCond->bt.level)
    {
        wme_level_at_firing = pCond->bt.level;
    } else if (condition_tests.id->eq_test->data.referent->is_sti())
    {
        assert (condition_tests.id->eq_test->data.referent->id->level);
        wme_level_at_firing = condition_tests.id->eq_test->data.referent->id->level;
        dprint(DT_EXPLAIN_CONDS, "   No backtrace level found.  Setting condition level to id's current level.\n", wme_level_at_firing);
    } else {
        wme_level_at_firing = 0;
        dprint(DT_EXPLAIN_CONDS, "   No backtrace level or sti identifier found.  Setting condition level to 0.\n", wme_level_at_firing);
    }

    /* Cache the pref to make it easier to connect this condition to the action that created
     * the preference later. Tricky because NCs and NCCs have neither and architectural
     * may have neither */
    cached_pref = pCond->bt.trace;
    cached_wme = pCond->bt.wme_;
    if (pCond->bt.trace)
    {
        parent_instantiation = thisAgent->explanationMemory->get_instantiation(pCond->bt.trace->inst);
    } else {
        parent_instantiation = NULL;
    }
    dprint(DT_EXPLAIN_CONDS, "   Done creating condition %u.\n", conditionID);
}

void condition_record::clean_up()
{
    dprint(DT_EXPLAIN_CONDS, "   Deleting condition record c%u for: (%t ^%t %t)\n", conditionID, condition_tests.id, condition_tests.attr, condition_tests.value);

    deallocate_test(thisAgent, condition_tests.id);
    deallocate_test(thisAgent, condition_tests.attr);
    deallocate_test(thisAgent, condition_tests.value);

    dprint(DT_EXPLAIN_CONDS, "   Removing references for matched wme: (%y ^%y %y)\n", matched_wme.id, matched_wme.attr, matched_wme.value);
    if (matched_wme.id) thisAgent->symbolManager->symbol_remove_ref(&matched_wme.id);
    if (matched_wme.attr) thisAgent->symbolManager->symbol_remove_ref(&matched_wme.attr);
    if (matched_wme.value) thisAgent->symbolManager->symbol_remove_ref(&matched_wme.value);

    if (path_to_base)
    {
        delete path_to_base;
    }
    dprint(DT_EXPLAIN_CONDS, "   Done deleting condition record c%u\n", conditionID);
}

void condition_record::connect_to_action()
{
    if (parent_instantiation && cached_pref)
    {
        assert(cached_pref);
        parent_action = parent_instantiation->find_rhs_action(cached_pref);
        assert(parent_action);
        dprint(DT_EXPLAIN_CONNECT, "   Linked condition %u (%t ^%t %t) to a%u in i%u.\n", conditionID, condition_tests.id, condition_tests.attr, condition_tests.value, parent_action->get_actionID(), parent_instantiation->get_instantiationID());
    } else {
        dprint(DT_EXPLAIN_CONNECT, "   Did not link condition %u (%t ^%t %t) because no parent instantiation.\n", conditionID, condition_tests.id, condition_tests.attr, condition_tests.value);
    }
//    cached_pref = NULL;
}

void condition_record::viz_connect_to_action(goal_stack_level pMatchLevel)
{
    if (parent_instantiation && (wme_level_at_firing == pMatchLevel))
    {
        assert(parent_action);
        assert(my_instantiation);
        thisAgent->visualizationManager->viz_connect_action_to_cond(parent_instantiation->get_instantiationID(),
            parent_action->get_actionID(), my_instantiation->get_instantiationID(), conditionID);
    }
}

void condition_record::update_condition(condition* pCond, instantiation_record* pInst)
{
    //dprint(DT_EXPLAIN_UPDATE, "   Updating condition c%u for %l.\n", conditionID, pCond);
    if (!matched_wme.id)
    {
        set_matched_wme_for_cond(pCond);
    }
    cached_pref = pCond->bt.trace;
    cached_wme = pCond->bt.wme_;
    if (pCond->bt.trace)
    {
        parent_instantiation = thisAgent->explanationMemory->get_instantiation(pCond->bt.trace->inst);
    } else {
        parent_instantiation = NULL;
    }
    parent_action = NULL;
    if (path_to_base) {
        delete path_to_base;
    }
    path_to_base = NULL;
}

void condition_record::set_matched_wme_for_cond(condition* pCond)
{
    /* bt info wme doesn't seem to always exist (maybe just for terminal nodes), so
     * we use actual tests if we know it's a literal condition because identifier is STI */
    if (condition_tests.id->eq_test->data.referent->is_sti() &&
        !condition_tests.attr->eq_test->data.referent->is_variable() &&
        !condition_tests.attr->eq_test->data.referent->is_variable())
    {
        matched_wme = { condition_tests.id->eq_test->data.referent, condition_tests.attr->eq_test->data.referent, condition_tests.value->eq_test->data.referent };
        thisAgent->symbolManager->symbol_add_ref(matched_wme.id);
        thisAgent->symbolManager->symbol_add_ref(matched_wme.attr);
        thisAgent->symbolManager->symbol_add_ref(matched_wme.value);
    } else {
        if (pCond->bt.wme_)
        {
            matched_wme = { pCond->bt.wme_->id, pCond->bt.wme_->attr, pCond->bt.wme_->value };
            thisAgent->symbolManager->symbol_add_ref(matched_wme.id);
            thisAgent->symbolManager->symbol_add_ref(matched_wme.attr);
            thisAgent->symbolManager->symbol_add_ref(matched_wme.value);
        } else {
            matched_wme.id = matched_wme.attr = matched_wme.value = NULL;
        }
    }
}

bool test_contains_identity_in_set(agent* thisAgent, test t, const id_set* pIDSet)
{
    cons* c;

    switch (t->type)
    {
        case EQUALITY_TEST:
            if (t->identity)
            {
                id_set::const_iterator it;
                it = pIDSet->find(t->identity);
                if (it != pIDSet->end())
                {
                    return true;
                }
            }

            return false;
            break;
        case CONJUNCTIVE_TEST:
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                if (test_contains_identity_in_set(thisAgent, static_cast<test>(c->first), pIDSet))
                {
                    return true;
                }
            }
            return false;
            break;

        default:  /* relational tests other than equality */
            return false;
    }
}

bool condition_record::contains_identity_from_set(const id_set* pIDSet)
{
    bool returnVal = (test_contains_identity_in_set(thisAgent, condition_tests.value, pIDSet) ||
        test_contains_identity_in_set(thisAgent, condition_tests.id, pIDSet) ||
        test_contains_identity_in_set(thisAgent, condition_tests.attr, pIDSet));

    dprint(DT_EXPLAIN_PATHS, "condition_record::contains_identity_from_set returning %s.\n", returnVal ? "TRUE" : "FALSE");

    return returnVal;
}

void condition_record::create_identity_paths(const inst_record_list* pInstPath)
{
    if (path_to_base)
    {
        dprint(DT_EXPLAIN_PATHS, "      Condition already has a path to base.  Skipping (%t ^%t %t).\n", condition_tests.id, condition_tests.attr, condition_tests.value);
        return;
    } else
    {
        assert(!path_to_base);
        path_to_base = new inst_record_list();
        (*path_to_base) = (*pInstPath);
        dprint(DT_EXPLAIN_PATHS, "      Condition record copied path_to_base %d = %d.\n", path_to_base->size(), pInstPath->size());
    }

}

void condition_record::viz_combo_test(test pTest, test pTestIdentity, uint64_t pNode_id, bool printInitialPort, bool printFinalPort, bool isAttribute, bool isNegative, bool printIdentity)
{
    cons* c, *c2;
    GraphViz_Visualizer* visualizer = thisAgent->visualizationManager;

    if (pTest->type == CONJUNCTIVE_TEST)
    {
        visualizer->viz_table_element_conj_start(printInitialPort ? pNode_id : 0, 'c', false);
        for (c = pTest->data.conjunct_list, c2 = pTestIdentity->data.conjunct_list; c != NIL; c = c->rest, c2 = c2->rest)
        {
            assert(c2);
            visualizer->viz_record_start();
            viz_combo_test(static_cast<test>(c->first), static_cast<test>(c2->first), pNode_id, false, false, false, false, printIdentity);
            visualizer->viz_record_end();
            visualizer->viz_endl();
        }
        visualizer->viz_table_end();
        visualizer->viz_table_element_end();
        visualizer->viz_endl();
    } else {
        if (printInitialPort || printFinalPort)
        {
            visualizer->viz_table_element_start(pNode_id, 'c', printInitialPort);
        } else {
            visualizer->viz_table_element_start();
        }
        if (isAttribute)
        {
            if (isNegative)
            {
                visualizer->graphviz_output += "-^";
            } else {
                visualizer->graphviz_output += "^";
            }
        }
        if (printIdentity && pTestIdentity->identity)
        {
            thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%t (%u) ", pTest, pTestIdentity->identity);
        } else {
            thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%t ", pTest);
        }
        visualizer->viz_table_element_end();
    }
}

void condition_record::viz_matched_test(test pTest, Symbol* pMatchedWME, uint64_t pNode_id, bool printInitialPort, bool printFinalPort, bool isAttribute, bool isNegative, bool printIdentity)
{
    cons* c;
    GraphViz_Visualizer* visualizer = thisAgent->visualizationManager;

    if (pTest->type == CONJUNCTIVE_TEST)
    {
        visualizer->viz_table_element_conj_start(printInitialPort ? pNode_id : 0, 'c', false);
        for (c = pTest->data.conjunct_list; c != NIL; c = c->rest)
        {
            visualizer->viz_record_start();
            viz_matched_test(static_cast<test>(c->first), pMatchedWME, pNode_id, false, false, false, false, printIdentity);
            visualizer->viz_record_end();
            visualizer->viz_endl();
        }
        visualizer->viz_table_end();
        visualizer->viz_table_element_end();
        visualizer->viz_endl();
    } else {
        if (printInitialPort || printFinalPort)
        {
            visualizer->viz_table_element_start(pNode_id, 'c', printInitialPort);
        } else {
            visualizer->viz_table_element_start();
        }
        if (isAttribute)
        {
            if (isNegative)
            {
                visualizer->graphviz_output += "-^";
            } else {
                visualizer->graphviz_output += "^";
            }
        }
        if (printIdentity || !pMatchedWME || (pTest->type != EQUALITY_TEST))
        {
            if (pTest->identity)
            {
                thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%t (%u) ", pTest, pTest->identity);
            } else {
                thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%t ", pTest);
            }
        } else {
            thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%y ", pMatchedWME);
        }
        visualizer->viz_table_element_end();
    }
}

/* This is only called for instantiated conditions in the wme trace.
 *
 * Note:  This may cause a bad vizgraph if attribute of NC is a conjunct.  The minus
 *        sign would be outside the brackets of the nested records for the conjunct. */
void condition_record::visualize_for_wm_trace()
{
    test id_test_without_goal_test ;

    thisAgent->visualizationManager->viz_record_start();
    id_test_without_goal_test = copy_test(thisAgent, condition_tests.id, false, false, true);
    viz_matched_test(id_test_without_goal_test, NULL, conditionID, true, false, false, false, false);
    deallocate_test(thisAgent, id_test_without_goal_test);
    viz_matched_test(condition_tests.attr, NULL, conditionID, false, false, true, (type == NEGATIVE_CONDITION), false);
    viz_matched_test(condition_tests.value, NULL, conditionID, false, true, false, false, false);
    thisAgent->visualizationManager->viz_record_end();
}

void condition_record::visualize_for_chunk()
{
    test id_test_without_goal_test ;

    thisAgent->visualizationManager->viz_record_start();
    id_test_without_goal_test = copy_test(thisAgent, condition_tests.id, false, false, true);
    viz_matched_test(id_test_without_goal_test, matched_wme.id, conditionID, true, false, false, false, thisAgent->explanationMemory->print_explanation_trace);
    deallocate_test(thisAgent, id_test_without_goal_test);
    viz_matched_test(condition_tests.attr, matched_wme.attr, conditionID, false, false, true, (type == NEGATIVE_CONDITION), thisAgent->explanationMemory->print_explanation_trace);
    viz_matched_test(condition_tests.value, matched_wme.value, conditionID, false, true, false, false, thisAgent->explanationMemory->print_explanation_trace);
    thisAgent->visualizationManager->viz_record_end();
}

void condition_record::visualize_for_explanation_trace(condition* pCond)
{
    test id_test_without_goal_test = copy_test(thisAgent, pCond->data.tests.id_test, false, false, true);
    test id_test_without_goal_test2 = copy_test(thisAgent, condition_tests.id, false, false, true);

    thisAgent->visualizationManager->viz_record_start();
    viz_combo_test(id_test_without_goal_test, id_test_without_goal_test2, conditionID, true, false, false, false, true);
    deallocate_test(thisAgent, id_test_without_goal_test);
    deallocate_test(thisAgent, id_test_without_goal_test2);
    viz_combo_test(pCond->data.tests.attr_test, condition_tests.attr, conditionID, false, false, true, (type == NEGATIVE_CONDITION), true);
    viz_combo_test(pCond->data.tests.value_test, condition_tests.value, conditionID, false, true, false, false, true);
    thisAgent->visualizationManager->viz_record_end();
}

