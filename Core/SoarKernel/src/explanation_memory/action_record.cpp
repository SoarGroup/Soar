#include "action_record.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "ebc.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production_record.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "symbol.h"
#include "test.h"
#include "working_memory.h"
#include "visualize.h"

void simplify_identity_in_rhs_value(agent* thisAgent, rhs_value rv)
{
    if (!rv || (rhs_value_is_reteloc(rv)) || (rhs_value_is_unboundvar(rv))) return;

    if (rhs_value_is_funcall(rv))
    {
        cons* fl = rhs_value_to_funcall_list(rv);
        for (cons* c = fl->rest; c != NIL; c = c->rest)
        {
            simplify_identity_in_rhs_value(thisAgent, static_cast<char*>(c->first));
        }
        return;
    }

    rhs_symbol r = rhs_value_to_rhs_symbol(rv);
    uint64_t lID = r->identity;
    IdentitySetSharedPtr lIDSet = r->identity_set_wp;

    if (lIDSet) r->identity = lIDSet->get_identity();
    else r->identity = LITERAL_VALUE;

    r->identity_set_wp = NULL;
//    r->identity_set_wp.reset();
}

void simplify_identity_in_action(agent* thisAgent, action* pAction)
{
    simplify_identity_in_rhs_value(thisAgent, pAction->id);
    simplify_identity_in_rhs_value(thisAgent, pAction->attr);
    simplify_identity_in_rhs_value(thisAgent, pAction->value);
    if (preference_is_binary(pAction->preference_type))
        simplify_identity_in_rhs_value(thisAgent, pAction->referent);
}


void simplify_identity_in_preference(agent* thisAgent, preference* pPref)
{
    if (pPref->identity_sets.id)
    {
        pPref->identities.id = pPref->identity_sets.id->super_join->idset_id;
        pPref->identity_sets.id = NULL_IDENTITY_SET;
    }
    if (pPref->identity_sets.attr)
    {
        pPref->identities.attr = pPref->identity_sets.attr->super_join->idset_id;
        pPref->identity_sets.attr = NULL_IDENTITY_SET;
    }
    if (pPref->identity_sets.value)
    {
        pPref->identities.value = pPref->identity_sets.value->super_join->idset_id;
        pPref->identity_sets.value = NULL_IDENTITY_SET;
    }
    if (preference_is_binary(pPref->type) && pPref->identity_sets.referent)
    {
        pPref->identities.referent = pPref->identity_sets.referent->super_join->idset_id;
        pPref->identity_sets.referent = NULL_IDENTITY_SET;
    }
}

void action_record::init(agent* myAgent, preference* pPref, action* pAction, uint64_t pActionID)
{
    thisAgent               = myAgent;
    actionID                = pActionID;
    instantiated_pref       = shallow_copy_preference(thisAgent, pPref);
    original_pref           = pPref;
    simplify_identity_in_preference(thisAgent, instantiated_pref);

    if (pAction)
    {
        variablized_action = copy_action(thisAgent, pAction);
        simplify_identity_in_action(thisAgent, variablized_action);
    } else {
        variablized_action = NULL;
    }
    identities_used = NULL;
    dprint(DT_EXPLAIN_CONDS, "   Created action record a%u for pref %p (%r ^%r %r %r), [act %a]", pActionID, pPref, pPref->rhs_funcs.id, pPref->rhs_funcs.attr, pPref->rhs_funcs.value, pPref->rhs_funcs.referent, pAction);
}

void action_record::clean_up()
{
    dprint(DT_EXPLAIN_CONDS, "   Deleting action record a%u for: %p\n", actionID, instantiated_pref);
    deallocate_preference(thisAgent, instantiated_pref, true);
    deallocate_action_list(thisAgent, variablized_action);
    if (identities_used)
    {
        delete identities_used;
    }
    dprint(DT_EXPLAIN_CONDS, "   Done deleting action record a%u\n", actionID);
}

void action_record::print_rhs_chunk_value(const rhs_value pRHS_value, const rhs_value pRHS_variablized_value, bool printActual)
{
    std::string tempString;
    bool identity_printed = false;

    if (!printActual)
    {
        if (pRHS_variablized_value)
        {
            if (rhs_value_is_symbol(pRHS_variablized_value)  || rhs_value_is_funcall(pRHS_variablized_value))
            {
                tempString = "";
                thisAgent->outputManager->set_print_test_format(false, true);
                thisAgent->outputManager->rhs_value_to_string(pRHS_variablized_value, tempString, NULL, NULL, true);
                if (!tempString.empty())
                {
                    thisAgent->outputManager->printa_sf(thisAgent, "%s", tempString.c_str());
                    identity_printed = true;
                }
            }
        }
    }
    if (printActual || !identity_printed)
    {
        tempString = "";
        thisAgent->outputManager->set_print_test_format(true, false);
        thisAgent->outputManager->rhs_value_to_string(pRHS_value, tempString);
        thisAgent->outputManager->printa_sf(thisAgent, "%s", tempString.c_str());
    }
    thisAgent->outputManager->clear_print_test_format();
}
void action_record::print_rhs_instantiation_value(const rhs_value pRHS_value, const rhs_value pPref_func, uint64_t pID, bool printActual)
{
    std::string tempString("");

    if (printActual)
    {
        thisAgent->outputManager->set_print_test_format(true, false);
        thisAgent->outputManager->rhs_value_to_string(pRHS_value, tempString);
        thisAgent->outputManager->printa_sf(thisAgent, "%s", tempString.c_str());
    } else {
        if (pPref_func) {
            thisAgent->outputManager->set_print_test_format(false, true);
            thisAgent->outputManager->rhs_value_to_string(pPref_func, tempString, NULL, NULL, true);
            thisAgent->outputManager->printa_sf(thisAgent, "%s", tempString.c_str());
        } else {
            if (!pID)
            {
                thisAgent->outputManager->set_print_test_format(true, false);
                thisAgent->outputManager->rhs_value_to_string(pRHS_value, tempString);
                thisAgent->outputManager->printa_sf(thisAgent, "%s", tempString.c_str());
            } else {
                thisAgent->outputManager->printa_sf(thisAgent, "[%u]", pID);
            }
        }
    }
    thisAgent->outputManager->clear_print_test_format();
}

void action_record::viz_rhs_value(const rhs_value pRHS_value, const rhs_value pRHS_variablized_value, uint64_t pID, uint64_t pNodeID, char pTypeChar, WME_Field pField)
{
    std::string tempString;
    bool identity_printed = false;
    tempString = "";

    std::string highlight_str;
    if (pID)
    {
        highlight_str = " BGCOLOR=\"";
        highlight_str += thisAgent->visualizationManager->get_color_for_id(pID);
        highlight_str += "\" ";
    } else highlight_str = " ";

    thisAgent->visualizationManager->viz_table_element_start(pNodeID, pTypeChar, pField, false, highlight_str.c_str());

    thisAgent->outputManager->set_print_test_format(true, false);
    thisAgent->outputManager->rhs_value_to_string(pRHS_value, tempString);
    thisAgent->visualizationManager->graphviz_output += tempString;
    if (pRHS_variablized_value)
    {
        if (rhs_value_is_symbol(pRHS_variablized_value)  || rhs_value_is_funcall(pRHS_variablized_value))
        {
            tempString = "";
            thisAgent->outputManager->set_print_test_format(false, true);
            thisAgent->outputManager->rhs_value_to_string(pRHS_variablized_value, tempString, NULL, NULL, true);
            thisAgent->outputManager->set_print_test_format(true, false);
            if (!tempString.empty())
            {
                thisAgent->outputManager->printa_sf(thisAgent, "[%u]", pID);
                identity_printed = true;
            }
        }
    }
    if (!identity_printed && pID) {
        thisAgent->outputManager->printa_sf(thisAgent, "[%u]", pID);
    }

    thisAgent->visualizationManager->viz_table_element_end();
}

void action_record::viz_action_list(agent* thisAgent, action_record_list* pActionRecords, production* pOriginalRule, action* pRhs, production_record* pExcisedRule)
{
    if (pActionRecords->empty())
    {
        thisAgent->visualizationManager->viz_text_record("Empty RHS");
    }
    else
    {
        action_record* lAction;
        condition* top = NULL, *bottom = NULL;
        action* rhs;
        int lActionCount = 0;
        thisAgent->outputManager->set_print_indents("");
        thisAgent->outputManager->set_print_test_format(true, false);
        if (thisAgent->explanationMemory->print_explanation_trace)
        {
            /* We use pRhs to deallocate actions at end, and rhs to iterate through actions */
            if (pRhs)
            {
                rhs = pRhs;
            } else {
                if (!pOriginalRule || !pOriginalRule->p_node)
                {
                    if (pExcisedRule)
                    {
                        rhs = pExcisedRule->get_rhs();
                        assert(rhs);
                    } else {
                        thisAgent->visualizationManager->viz_record_start();
                        thisAgent->visualizationManager->viz_text_record("No RETE rule");
                        thisAgent->visualizationManager->viz_record_end();
                        return;
                    }
                } else {
                    p_node_to_conditions_and_rhs(thisAgent, pOriginalRule->p_node, NIL, NIL, &top, &bottom, &rhs);
                    pRhs = rhs;
                }
            }
        }
//        if (thisAgent->explanationMemory->print_explanation_trace)
//        {
//            int lNumRecords = rhs ? 1 : 0;
//            action* tempRHS = rhs;
//            while (tempRHS->next)
//            {
//                ++lNumRecords;
//                tempRHS = tempRHS->next;
//            }
//            while (rhs->next)
//            {
//                if (++lActionCount <= lNumRecords) thisAgent->visualizationManager->viz_endl();
//                lAction->viz_action(rhs);
//                rhs = rhs->next;
//            }
//        } else {
//            size_t lNumRecords = pActionRecords->size();
//            for (action_record_list::iterator it = pActionRecords->begin(); it != pActionRecords->end(); it++)
//            {
//                lAction = (*it);
//                if (++lActionCount <= lNumRecords) thisAgent->visualizationManager->viz_endl();
//                lAction->viz_preference();
//            }
//        }
        size_t lNumRecords = pActionRecords->size();
        for (action_record_list::iterator it = pActionRecords->begin(); it != pActionRecords->end(); it++)
        {
            lAction = (*it);
            ++lActionCount;
            if (lActionCount <= lNumRecords)
            {
                thisAgent->visualizationManager->viz_endl();
            }
            if (!thisAgent->explanationMemory->print_explanation_trace)
            {
                lAction->viz_preference();
            } else {
                lAction->viz_action(rhs);
                rhs = rhs->next;
            }
        }
        thisAgent->visualizationManager->graphviz_output += "\n";
        if (thisAgent->explanationMemory->print_explanation_trace)
        {
            /* If top exists, we generated conditions here and must deallocate. */
            if (pRhs) deallocate_action_list(thisAgent, pRhs);
            if (top) deallocate_condition_list(thisAgent, top);
        }
        thisAgent->outputManager->clear_print_test_format();
    }
}
void action_record::viz_preference()
{
    thisAgent->visualizationManager->viz_record_start();
    thisAgent->visualizationManager->viz_table_element_start(actionID, 'a', ID_ELEMENT);
    thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, "%y", instantiated_pref->id);
    thisAgent->visualizationManager->viz_table_element_end();
    thisAgent->visualizationManager->viz_table_element_start();
    thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, "%y", instantiated_pref->attr);
    thisAgent->visualizationManager->viz_table_element_end();

    if (preference_is_binary(instantiated_pref->type))
    {
        thisAgent->visualizationManager->viz_table_element_start();
        thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, "%y", instantiated_pref->value);
        thisAgent->visualizationManager->viz_table_element_end();
        thisAgent->visualizationManager->viz_table_element_start(actionID, 'a', NO_ELEMENT);
        thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, " %c %y", preference_to_char(instantiated_pref->type), instantiated_pref->referent);
        thisAgent->visualizationManager->viz_table_element_end();
    } else {
        thisAgent->visualizationManager->viz_table_element_start(actionID, 'a', VALUE_ELEMENT);
        thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, " %y %c", instantiated_pref->value, preference_to_char(instantiated_pref->type));
        thisAgent->visualizationManager->viz_table_element_end();
    }
    thisAgent->visualizationManager->viz_record_end();
}

void action_record::viz_action(action* pAction)
{
    std::string tempString;

    if (pAction->type == FUNCALL_ACTION)
    {
        thisAgent->visualizationManager->viz_record_start();
        thisAgent->visualizationManager->viz_table_element_start(actionID, 'a', ID_ELEMENT);
        thisAgent->visualizationManager->graphviz_output += "RHS Funcall";
        thisAgent->visualizationManager->viz_table_element_end();
        thisAgent->visualizationManager->viz_table_element_start(actionID, 'a', VALUE_ELEMENT);
        tempString = "";
        thisAgent->outputManager->rhs_value_to_string(pAction->value, tempString);
        thisAgent->visualizationManager->graphviz_output += tempString;
        thisAgent->visualizationManager->viz_table_element_end();
        thisAgent->visualizationManager->viz_record_end();
    } else {
        thisAgent->visualizationManager->viz_record_start();
        viz_rhs_value(pAction->id, (variablized_action ? variablized_action->id : NULL), instantiated_pref->identities.id, actionID, 'a', ID_ELEMENT);
        viz_rhs_value(pAction->attr, (variablized_action ? variablized_action->attr : NULL), instantiated_pref->identities.attr);
        if (pAction->referent)
        {
            viz_rhs_value(pAction->value, (variablized_action ? variablized_action->value : NULL), instantiated_pref->identities.value);
            thisAgent->visualizationManager->graphviz_output += preference_to_char(pAction->preference_type);
            viz_rhs_value(pAction->referent, (variablized_action ? variablized_action->referent : NULL), instantiated_pref->identities.referent, actionID, 'a', VALUE_ELEMENT);
        } else {
            viz_rhs_value(pAction->value, (variablized_action ? variablized_action->value : NULL), instantiated_pref->identities.value, actionID, 'a', VALUE_ELEMENT);
            thisAgent->visualizationManager->graphviz_output += ' ';
            thisAgent->visualizationManager->graphviz_output += preference_to_char(pAction->preference_type);
        }
        thisAgent->visualizationManager->viz_record_end();
    }
    tempString.clear();
}
//viz_rhs_value(pAction->id, (variablized_action ? rhs_value_true_null(variablized_action->id) : NULL), instantiated_pref->o_ids.id);
//viz_rhs_value(pAction->attr, (variablized_action ? rhs_value_true_null(variablized_action->attr) : NULL), instantiated_pref->o_ids.attr);
//viz_rhs_value(pAction->value, (variablized_action ? rhs_value_true_null(variablized_action->value) : NULL), instantiated_pref->o_ids.value);
//viz_rhs_value(pAction->referent, (variablized_action ? rhs_value_true_null(variablized_action->referent) : NULL), instantiated_pref->o_ids.referent);
//viz_rhs_value(pAction->value, (variablized_action ? rhs_value_true_null(variablized_action->value) : NULL), instantiated_pref->o_ids.value);
