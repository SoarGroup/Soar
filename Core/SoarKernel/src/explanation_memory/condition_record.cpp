#include "condition_record.h"

#include "action_record.h"
#include "agent.h"
#include "condition.h"
#include "ebc.h"
#include "ebc_identity.h"
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

void simplify_identity_in_test(agent* thisAgent, test t, bool isChunkInstantiation)
{
    if (!t) return;

    switch (t->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case SMEM_LINK_UNARY_TEST:
        case SMEM_LINK_UNARY_NOT_TEST:
        case DISJUNCTION_TEST:
            break;
        case CONJUNCTIVE_TEST:
            for (cons* c = t->data.conjunct_list; c != NIL; c = c->rest)
                simplify_identity_in_test(thisAgent, static_cast<test>(c->first), isChunkInstantiation);
            break;
        default:
            if(isChunkInstantiation)
            {
                t->inst_identity = t->chunk_inst_identity;
                t->chunk_inst_identity = LITERAL_VALUE;
            } else {
                if (t->identity)
                {
                    t->inst_identity = t->identity->joined_identity->idset_id;
                    if (t->identity->idset_id != t->inst_identity)
                        t->chunk_inst_identity = t->identity->idset_id;
                    else
                        t->chunk_inst_identity = LITERAL_VALUE;
                } else {
                    t->chunk_inst_identity = LITERAL_VALUE;
                }
            }
            clear_test_identity(thisAgent, t);
            break;
    }
}

void condition_record::init(agent* myAgent, condition* pCond, uint64_t pCondID, instantiation_record* pInst, bool isChunkInstantiation)
{
    thisAgent = myAgent;
    conditionID = pCondID;
    type = pCond->type;
    parent_action = NULL;
    path_to_base = NULL;
    my_instantiation = NULL;

    condition_tests.id = copy_test(thisAgent, pCond->data.tests.id_test);
    condition_tests.attr = copy_test(thisAgent, pCond->data.tests.attr_test);
    condition_tests.value = copy_test(thisAgent, pCond->data.tests.value_test);
    simplify_identity_in_test(thisAgent, condition_tests.id, isChunkInstantiation);
    simplify_identity_in_test(thisAgent, condition_tests.attr, isChunkInstantiation);
    simplify_identity_in_test(thisAgent, condition_tests.value, isChunkInstantiation);
    test_for_acceptable_preference = pCond->test_for_acceptable_preference;

    set_matched_wme_for_cond(pCond);

    if (pCond->bt.level)
    {
        wme_level_at_firing = pCond->bt.level;
    } else if (condition_tests.id->eq_test->data.referent->is_sti())
    {
        wme_level_at_firing = condition_tests.id->eq_test->data.referent->id->level;
    } else {
        wme_level_at_firing = 0;
    }

    /* Cache the pref to make it easier to connect this condition to the action that created
     * the preference later. Tricky because NCs and NCCs have neither and architectural
     * may have neither */
    parent_instantiation = NULL;
    cached_wme = pCond->bt.wme_;

    if (pCond->bt.trace)
    {
        if (isChunkInstantiation)
        {
            assert(pCond->explain_inst);
            my_instantiation = thisAgent->explanationMemory->get_instantiation(pCond->explain_inst);
            assert(my_instantiation);

            cached_pref = pCond->bt.trace;
            parent_instantiation = thisAgent->explanationMemory->get_instantiation(pCond->bt.trace->inst);
        }
        else
        {
            cached_pref = pCond->bt.trace;
            parent_instantiation = thisAgent->explanationMemory->get_instantiation(pCond->bt.trace->inst);
            my_instantiation = pInst;
        }
    } else {
        cached_pref = NULL;
        if (pCond->explain_inst)
        {
            my_instantiation = thisAgent->explanationMemory->get_instantiation(pCond->explain_inst);
        } else  {
            my_instantiation = pInst;
        }
    }
}

void condition_record::clean_up()
{
    deallocate_test(thisAgent, condition_tests.id);
    deallocate_test(thisAgent, condition_tests.attr);
    deallocate_test(thisAgent, condition_tests.value);

    if (matched_wme.id) thisAgent->symbolManager->symbol_remove_ref(&matched_wme.id);
    if (matched_wme.attr) thisAgent->symbolManager->symbol_remove_ref(&matched_wme.attr);
    if (matched_wme.value) thisAgent->symbolManager->symbol_remove_ref(&matched_wme.value);

    if (path_to_base)
    {
        delete path_to_base;
    }
}

void condition_record::connect_to_action()
{
    if (parent_instantiation && cached_pref)
    {
        parent_action = parent_instantiation->find_rhs_action(cached_pref);
    }
}

void condition_record::viz_connect_to_action(goal_stack_level pMatchLevel, bool isChunkInstantiation)
{
    if (parent_instantiation && (wme_level_at_firing == pMatchLevel))
    {
        if (!isChunkInstantiation)
        {
            thisAgent->visualizationManager->viz_connect_action_to_cond(parent_instantiation->get_instantiationID(),
                parent_action->get_actionID(), my_instantiation->get_instantiationID(), conditionID);
        }
    }
}

void condition_record::update_condition(condition* pCond, instantiation_record* pInst, bool isChunkInstantiation)
{
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

void condition_record::viz_combo_test(test pTest, test pTestIdentity, uint64_t pNode_id, WME_Field pField, bool isNegative, bool printAcceptable, bool isSuper)
{
    cons* c, *c2;
    test c1_test, c2_test;
    GraphViz_Visualizer* visualizer = thisAgent->visualizationManager;
    std::string highlight_str;
    uint64_t lIdentityForColor = 0;

    if (pTestIdentity)
    {
        if ((pTest->type == CONJUNCTIVE_TEST) && pTestIdentity->eq_test->inst_identity)
        {
            if ((thisAgent->visualizationManager->settings->use_joined_identities->get_value() == on) || !pTestIdentity->eq_test->chunk_inst_identity)
                lIdentityForColor = pTestIdentity->eq_test->inst_identity;
            else
                lIdentityForColor = pTestIdentity->eq_test->chunk_inst_identity;
        } else  if (pTestIdentity && pTestIdentity->inst_identity)
        {
            if ((thisAgent->visualizationManager->settings->use_joined_identities->get_value() == on) || !pTestIdentity->chunk_inst_identity)
                lIdentityForColor = pTestIdentity->inst_identity;
            else
                lIdentityForColor = pTestIdentity->chunk_inst_identity;
        }
    }
    if (lIdentityForColor) highlight_str += visualizer->get_color_for_id(lIdentityForColor); else highlight_str = " ";

    if (pTest->type == CONJUNCTIVE_TEST)
    {
        visualizer->viz_table_element_conj_start((pField == ID_ELEMENT) ? pNode_id : 0, 'c', NO_ELEMENT, isSuper, highlight_str.c_str());
        if (pTestIdentity->type == CONJUNCTIVE_TEST)
        {
            c2 =  pTestIdentity->data.conjunct_list;
            c2_test = NULL;
        } else {
            c2 = NULL;
            c2_test = pTestIdentity;
        }
        /* The following requires that the two conjunctive tests have the same number of elements.  This is to
         * handle the case when the main test for an identifier element has an isa_state test but the identity
         * test does not. */
        for (c = pTest->data.conjunct_list; c != NIL; c = c->rest)
        {
            visualizer->viz_record_start();
            c1_test = static_cast<test>(c->first);
            if (c2)
            {
                c2_test = static_cast<test>(c2->first);
                viz_combo_test(c1_test, c2_test, pNode_id, NO_ELEMENT, false, printAcceptable, isSuper);
            } else {
                if (test_has_referent(c1_test) && c1_test->data.referent->is_variable())
                {
                    viz_combo_test(c1_test, c2_test, pNode_id, NO_ELEMENT, false, printAcceptable, isSuper);
                } else {
                    viz_combo_test(c1_test, NULL, pNode_id, NO_ELEMENT, false, printAcceptable, isSuper);
                }
            }
            visualizer->viz_record_end();
            visualizer->viz_endl();
            if (c2) c2 = c2->rest;
        }
        visualizer->viz_table_end();
        visualizer->viz_table_element_end();
        visualizer->viz_endl();
    } else {
        if ((pField == ID_ELEMENT) || (pField == VALUE_ELEMENT))
        {
            visualizer->viz_table_element_start(pNode_id, 'c', pField, isSuper, highlight_str.c_str());
        } else {
            visualizer->viz_table_element_start(0, ' ', ID_ELEMENT, isSuper, highlight_str.c_str());
        }
        if (pField == ATTR_ELEMENT)
        {
            if (isNegative)
            {
                visualizer->graphviz_output += "-^";
            } else {
                visualizer->graphviz_output += "^";
            }
        }
        if (pTestIdentity && (pTestIdentity->inst_identity || pTestIdentity->chunk_inst_identity))
        {
            if (pTestIdentity->chunk_inst_identity)
            {
                thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%t [%u->%u]", pTest, pTestIdentity->chunk_inst_identity, pTestIdentity->inst_identity);
            } else {
                thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%t [%u]", pTest, pTestIdentity->inst_identity);
            }
        } else {
            thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%t ", pTest);
        }
        if (printAcceptable) thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "+ ");
        visualizer->viz_table_element_end();
    }
}

void condition_record::viz_matched_test(test pTest, Symbol* pMatchedWME, uint64_t pNode_id, WME_Field pField, bool isNegative, bool printIdentity, bool printAcceptable, bool isSuper)
{
    cons* c;
    GraphViz_Visualizer* visualizer = thisAgent->visualizationManager;
    std::string highlight_str;
    uint64_t lIdentityForColor = 0;

    if (pTest->eq_test && pTest->eq_test->inst_identity)
    {
        if ((pTest->type == CONJUNCTIVE_TEST) && pTest->eq_test->inst_identity)
        {
            if ((thisAgent->visualizationManager->settings->use_joined_identities->get_value() == on) || !pTest->eq_test->chunk_inst_identity)
                lIdentityForColor = pTest->eq_test->inst_identity;
            else
                lIdentityForColor = pTest->eq_test->chunk_inst_identity;
        } else if (pTest->inst_identity)
        {
            if ((thisAgent->visualizationManager->settings->use_joined_identities->get_value() == on) || !pTest->chunk_inst_identity)
                lIdentityForColor = pTest->inst_identity;
            else
                lIdentityForColor = pTest->chunk_inst_identity;
        }
    }
    if (lIdentityForColor) highlight_str += visualizer->get_color_for_id(lIdentityForColor); else highlight_str = " ";

    if (pTest->type == CONJUNCTIVE_TEST)
    {
        visualizer->viz_table_element_conj_start((pField == ID_ELEMENT) ? pNode_id : 0, 'c', VALUE_ELEMENT, isSuper, highlight_str.c_str());
        for (c = pTest->data.conjunct_list; c != NIL; c = c->rest)
        {
            visualizer->viz_record_start();
            viz_matched_test(static_cast<test>(c->first), pMatchedWME, pNode_id, NO_ELEMENT, false, printIdentity, printAcceptable, isSuper);
            visualizer->viz_record_end();
            visualizer->viz_endl();
        }
        visualizer->viz_table_end();
        visualizer->viz_table_element_end();
        visualizer->viz_endl();
    } else {
        if ((pField == ID_ELEMENT) || (pField == VALUE_ELEMENT))
        {
            visualizer->viz_table_element_start(pNode_id, 'c', pField, isSuper, highlight_str.c_str());
        } else {
            visualizer->viz_table_element_start(0, ' ', NO_ELEMENT, isSuper, highlight_str.c_str());
        }
        if (pField == ATTR_ELEMENT)
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
            if (pTest->inst_identity || pTest->chunk_inst_identity)
            {
                if (pTest->chunk_inst_identity)
                {
                    thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%t [%u->%u]", pTest, pTest->chunk_inst_identity, pTest->inst_identity);
                } else {
                    thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%t [%u]", pTest, pTest->inst_identity);
                }
            } else {
                thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%t ", pTest);
            }
        } else {
            thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "%y ", pMatchedWME);
        }
        if (printAcceptable) thisAgent->outputManager->sprinta_sf(thisAgent, visualizer->graphviz_output, "+ ");
        visualizer->viz_table_element_end();
    }
}

/* This is only called for instantiated conditions in the wme trace.
 *
 * Note:  This may cause a bad vizgraph if attribute of NC is a conjunct.  The minus
 *        sign would be outside the brackets of the nested records for the conjunct. */
void condition_record::visualize_for_wm_trace(goal_stack_level match_level)
{
    test id_test_without_goal_test ;
    bool isSuper = (match_level > 0) && (wme_level_at_firing < match_level);

    thisAgent->visualizationManager->viz_record_start();
    id_test_without_goal_test = copy_test(thisAgent, condition_tests.id, false, false, true);
    viz_matched_test(id_test_without_goal_test, NULL, conditionID, ID_ELEMENT, false, false, false, isSuper);
    deallocate_test(thisAgent, id_test_without_goal_test);
    viz_matched_test(condition_tests.attr, NULL, conditionID, ATTR_ELEMENT, (type == NEGATIVE_CONDITION), false, false, isSuper);
    viz_matched_test(condition_tests.value, NULL, conditionID, VALUE_ELEMENT, false, false, test_for_acceptable_preference, isSuper);
    thisAgent->visualizationManager->viz_record_end();
}

void condition_record::visualize_for_chunk()
{
    thisAgent->visualizationManager->viz_record_start();
    viz_matched_test(condition_tests.id, matched_wme.id, conditionID, ID_ELEMENT, false, thisAgent->explanationMemory->print_explanation_trace, false, false);
    viz_matched_test(condition_tests.attr, matched_wme.attr, conditionID, ATTR_ELEMENT, (type == NEGATIVE_CONDITION), thisAgent->explanationMemory->print_explanation_trace, false, false);
    viz_matched_test(condition_tests.value, matched_wme.value, conditionID, VALUE_ELEMENT, false, thisAgent->explanationMemory->print_explanation_trace, test_for_acceptable_preference, false);
    thisAgent->visualizationManager->viz_record_end();
}

void condition_record::visualize_for_explanation_trace(condition* pCond, goal_stack_level match_level)
{
    bool isSuper = (match_level > 0) && (wme_level_at_firing < match_level);
    thisAgent->visualizationManager->viz_record_start();
    viz_combo_test(pCond->data.tests.id_test, condition_tests.id, conditionID, ID_ELEMENT, false, false, isSuper);
    viz_combo_test(pCond->data.tests.attr_test, condition_tests.attr, conditionID, ATTR_ELEMENT, (type == NEGATIVE_CONDITION), false, isSuper);
    viz_combo_test(pCond->data.tests.value_test, condition_tests.value, conditionID, VALUE_ELEMENT, false, test_for_acceptable_preference, isSuper);
    thisAgent->visualizationManager->viz_record_end();
}

