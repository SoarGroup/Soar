#include "action_record.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "ebc.h"
#include "ebc_identity_set.h"
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
    if (r->identity_set)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%u %u %u %u %u = ", r->identity, r->identity_set->idset_id, r->identity_set->super_join->idset_id, r->identity_set->clone_identity, r->identity_set->super_join->clone_identity);
        uint64_t lID = r->identity_set->get_identity();
        if (!lID) lID = r->identity_set->get_clone_identity();
        if (!lID) lID = r->identity;
        r->identity = lID;
        if (r->identity_set->idset_id != r->identity)
        {
            r->identity_unjoined = r->identity_set->idset_id;
        }
        thisAgent->outputManager->printa_sf(thisAgent, "%u -> %u\n", r->identity_unjoined, r->identity);
    } else r->identity = LITERAL_VALUE;

    r->identity_set = NULL;
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
    uint64_t temp;
    if (pPref->identity_sets.id)
    {
//        thisAgent->outputManager->printa_sf(thisAgent, "%u %u %u %u %u = ", pPref->identities.id, pPref->clone_identities.id, pPref->identity_sets.id->idset_id, pPref->identity_sets.id->super_join->idset_id, pPref->identity_sets.id->clone_identity, pPref->identity_sets.id->super_join->clone_identity);
        pPref->identities.id = pPref->identity_sets.id->super_join->idset_id;
        if (pPref->identity_sets.id->idset_id != pPref->identities.id)
        {
            pPref->clone_identities.id = pPref->identity_sets.id->idset_id;
        } else {
            pPref->clone_identities.id = LITERAL_VALUE;
        }
//        thisAgent->outputManager->printa_sf(thisAgent, "%u %u\n", pPref->identities.id, pPref->clone_identities.id);
        set_pref_identity_set(thisAgent, pPref, ID_ELEMENT, NULL_IDENTITY_SET);
    }
    else if (pPref->clone_identities.id)
    {
        temp = pPref->identities.id;
        pPref->identities.id = pPref->clone_identities.id;
        if (temp == pPref->identities.id)
        {
            pPref->clone_identities.id = LITERAL_VALUE;
        } else {
            pPref->clone_identities.id = temp;
        }
    }
    if (pPref->identity_sets.attr)
    {
        pPref->identities.attr = pPref->identity_sets.attr->super_join->idset_id;
        if (pPref->identity_sets.attr->idset_id != pPref->identities.attr)
        {
            pPref->clone_identities.attr = pPref->identity_sets.attr->idset_id;
        } else {
            pPref->clone_identities.attr = LITERAL_VALUE;
        }
        set_pref_identity_set(thisAgent, pPref, ATTR_ELEMENT, NULL_IDENTITY_SET);
    }
    else if (pPref->clone_identities.attr)
    {
        temp = pPref->identities.attr;
        pPref->identities.attr = pPref->clone_identities.attr;
        if (temp == pPref->identities.attr)
        {
            pPref->clone_identities.attr = LITERAL_VALUE;
        } else {
            pPref->clone_identities.attr = temp;
        }
    }

    if (pPref->identity_sets.value)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%u %u %u %u %u = ", pPref->identities.value, pPref->clone_identities.value, pPref->identity_sets.value->idset_id, pPref->identity_sets.value->super_join->idset_id, pPref->identity_sets.value->clone_identity, pPref->identity_sets.value->super_join->clone_identity);
        break_if_id_matches(pPref->clone_identities.value, 40);
        pPref->identities.value = pPref->identity_sets.value->super_join->idset_id;
        if (pPref->identity_sets.value->idset_id != pPref->identities.value)
        {
            pPref->clone_identities.value = pPref->identity_sets.value->idset_id;
        } else {
            pPref->clone_identities.value = LITERAL_VALUE;
        }
        thisAgent->outputManager->printa_sf(thisAgent, "%u %u\n", pPref->identities.value, pPref->clone_identities.value);
        set_pref_identity_set(thisAgent, pPref, VALUE_ELEMENT, NULL_IDENTITY_SET);
    }
    else if (pPref->clone_identities.value)
    {
        temp = pPref->identities.value;
        pPref->identities.value = pPref->clone_identities.value;
        if (temp == pPref->identities.value)
        {
            pPref->clone_identities.value = LITERAL_VALUE;
        } else {
            pPref->clone_identities.value = temp;
        }
    }

    if (preference_is_binary(pPref->type))
    {
        if (pPref->identity_sets.referent)
        {
            pPref->identities.referent = pPref->identity_sets.referent->super_join->idset_id;
            if (pPref->identity_sets.referent->idset_id != pPref->identities.referent)
            {
                pPref->clone_identities.referent = pPref->identity_sets.referent->idset_id;
            }
            set_pref_identity_set(thisAgent, pPref, REFERENT_ELEMENT, NULL_IDENTITY_SET);
        }
        else if (pPref->clone_identities.referent)
        {
            temp = pPref->identities.referent;
            pPref->identities.referent = pPref->clone_identities.referent;
            if (temp == pPref->identities.referent)
            {
                pPref->clone_identities.referent = LITERAL_VALUE;
            } else {
                pPref->clone_identities.referent = temp;
            }
        }
    }
    if (pPref->rhs_funcs.id) simplify_identity_in_rhs_value(thisAgent, pPref->rhs_funcs.id);
    if (pPref->rhs_funcs.attr) simplify_identity_in_rhs_value(thisAgent, pPref->rhs_funcs.attr);
    if (pPref->rhs_funcs.value) simplify_identity_in_rhs_value(thisAgent, pPref->rhs_funcs.value);
    if (pPref->rhs_funcs.referent) simplify_identity_in_rhs_value(thisAgent, pPref->rhs_funcs.referent);
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
                thisAgent->outputManager->rhs_value_to_string(pRHS_variablized_value, tempString, true, NULL, NULL, true);
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
void action_record::print_rhs_instantiation_value(const rhs_value pRHS_value, const rhs_value pPref_func, uint64_t pID, uint64_t pIDClone, bool printActual)
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
            thisAgent->outputManager->rhs_value_to_string(pPref_func, tempString, true, NULL, NULL, true);
            thisAgent->outputManager->printa_sf(thisAgent, "[%s]", tempString.c_str());
        } else {
            if (!(pID || pIDClone))
            {
                thisAgent->outputManager->set_print_test_format(true, false);
                thisAgent->outputManager->rhs_value_to_string(pRHS_value, tempString);
                thisAgent->outputManager->printa_sf(thisAgent, "[%s]", tempString.c_str());
            } else if (pIDClone) {
                thisAgent->outputManager->printa_sf(thisAgent, "[%u->%u]", pIDClone, pID);
            } else {
                thisAgent->outputManager->printa_sf(thisAgent, "[%u]", pID);
            }
        }
    }
    thisAgent->outputManager->clear_print_test_format();
}

void action_record::print_chunk_action(action* pAction, int lActionCount)
{
    std::string tempString;
    Output_Manager* outputManager = thisAgent->outputManager;

    if (pAction->type == FUNCALL_ACTION)
    {
        tempString = "";
        outputManager->rhs_value_to_string(pAction->value, tempString, true, NULL, NULL, true);
        outputManager->printa_sf(thisAgent, "%d:%-%s%-%s", lActionCount,  tempString.c_str(), tempString.c_str());
    } else {
        outputManager->printa_sf(thisAgent, "%d:%-(", lActionCount);
        print_rhs_chunk_value(pAction->id, (variablized_action ? variablized_action->id : NULL), true);
        outputManager->printa(thisAgent, " ^");
        print_rhs_chunk_value(pAction->attr, (variablized_action ? variablized_action->attr : NULL), true);
        outputManager->printa(thisAgent, " ");
        print_rhs_chunk_value(pAction->value, (variablized_action ? variablized_action->value : NULL), true);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_chunk_value(pAction->referent, (variablized_action ? variablized_action->referent : NULL), true);
        }
        outputManager->printa_sf(thisAgent, ")%-(");
        print_rhs_instantiation_value(pAction->id, instantiated_pref->rhs_funcs.id, instantiated_pref->identities.id, instantiated_pref->clone_identities.id, false);
        outputManager->printa(thisAgent, " ^");
        print_rhs_instantiation_value(pAction->attr, instantiated_pref->rhs_funcs.attr, instantiated_pref->identities.attr, instantiated_pref->clone_identities.attr, false);
        outputManager->printa(thisAgent, " ");
        print_rhs_instantiation_value(pAction->value, instantiated_pref->rhs_funcs.value, instantiated_pref->identities.value, instantiated_pref->clone_identities.value, false);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_instantiation_value(pAction->referent, instantiated_pref->rhs_funcs.referent, instantiated_pref->identities.referent, instantiated_pref->clone_identities.referent, false);
        }

        outputManager->printa(thisAgent, ")\n");
    }
    tempString.clear();
}

void action_record::print_instantiation_action(action* pAction, int lActionCount)
{
    std::string tempString;
    Output_Manager* outputManager = thisAgent->outputManager;

    if (pAction->type == FUNCALL_ACTION)
    {
        tempString = "";
        outputManager->rhs_value_to_string(pAction->value, tempString, true, NULL, NULL, true);
        outputManager->printa_sf(thisAgent, "%d:%-%s%-%s", lActionCount,  tempString.c_str(), tempString.c_str());
    } else {
        outputManager->printa_sf(thisAgent, "%d:%-(", lActionCount);
        print_rhs_instantiation_value(pAction->id, NULL, instantiated_pref->identities.id, instantiated_pref->clone_identities.id, true);
        outputManager->printa(thisAgent, " ^");
        print_rhs_instantiation_value(pAction->attr, NULL, instantiated_pref->identities.attr, instantiated_pref->clone_identities.attr, true);
        outputManager->printa(thisAgent, " ");
        print_rhs_instantiation_value(pAction->value, NULL, instantiated_pref->identities.value, instantiated_pref->clone_identities.value, true);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_instantiation_value(pAction->referent, NULL, instantiated_pref->identities.referent, instantiated_pref->clone_identities.referent, true);
        }
        outputManager->printa_sf(thisAgent, ")%-(");
        print_rhs_instantiation_value(pAction->id, instantiated_pref->rhs_funcs.id, instantiated_pref->identities.id, instantiated_pref->clone_identities.id, false);
        outputManager->printa(thisAgent, " ^");
        print_rhs_instantiation_value(pAction->attr, instantiated_pref->rhs_funcs.attr, instantiated_pref->identities.attr, instantiated_pref->clone_identities.attr, false);
        outputManager->printa(thisAgent, " ");
        print_rhs_instantiation_value(pAction->value, instantiated_pref->rhs_funcs.value, instantiated_pref->identities.value, instantiated_pref->clone_identities.value, false);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_instantiation_value(pAction->referent, instantiated_pref->rhs_funcs.referent, instantiated_pref->identities.referent, instantiated_pref->clone_identities.referent, false);
        }
        outputManager->printa(thisAgent, ")\n");
    }
    tempString.clear();
}

void action_record::viz_rhs_value(const rhs_value pRHS_value, const rhs_value pRHS_variablized_value, const rhs_value pRHS_func, uint64_t pID, uint64_t pIDClone, uint64_t pNodeID, char pTypeChar, WME_Field pField)
{
    std::string tempString;
    bool identity_printed = false;
    tempString = "";

    std::string highlight_str = thisAgent->visualizationManager->get_color_for_id(pID);;
    thisAgent->visualizationManager->viz_table_element_start(pNodeID, pTypeChar, pField, false, highlight_str.c_str());

    thisAgent->outputManager->set_print_test_format(true, false);
    thisAgent->outputManager->rhs_value_to_string(pRHS_value, tempString);
    thisAgent->visualizationManager->graphviz_output += tempString;

    if ((pRHS_variablized_value && rhs_value_is_symbol(pRHS_variablized_value)) || pRHS_func)
    {
        tempString = "";
        thisAgent->outputManager->set_print_test_format(false, true);
        thisAgent->outputManager->rhs_value_to_string(pRHS_func ? pRHS_func : pRHS_variablized_value, tempString, true, NULL, NULL, true);
        thisAgent->outputManager->set_print_test_format(true, false);
        if (!tempString.empty())
        {
            thisAgent->visualizationManager->graphviz_output += " [";
            thisAgent->visualizationManager->graphviz_output += tempString;
            thisAgent->visualizationManager->graphviz_output += " ]";
            identity_printed = true;
        }
    }
    if (!identity_printed && (pID || pIDClone)) {
        if (pIDClone)
        {
            thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, " [%u->%u]", pIDClone, pID);
        } else {
            thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, " [%u]", pID);
        }
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
        thisAgent->visualizationManager->viz_table_element_start(actionID, 'a', VALUE_ELEMENT);
        tempString = "";
        thisAgent->outputManager->rhs_value_to_string(pAction->value, tempString);
        thisAgent->visualizationManager->graphviz_output += tempString;
        thisAgent->visualizationManager->viz_table_element_end();
        thisAgent->visualizationManager->viz_record_end();
    } else {
        thisAgent->visualizationManager->viz_record_start();

        viz_rhs_value(pAction->id, (variablized_action ? variablized_action->id : NULL), instantiated_pref->rhs_funcs.id, instantiated_pref->identities.id, instantiated_pref->clone_identities.id, actionID, 'a', ID_ELEMENT);
        viz_rhs_value(pAction->attr, (variablized_action ? variablized_action->attr : NULL), instantiated_pref->rhs_funcs.attr, instantiated_pref->identities.attr, instantiated_pref->clone_identities.attr);

        if (pAction->referent)
        {
            viz_rhs_value(pAction->value, (variablized_action ? variablized_action->value : NULL), instantiated_pref->rhs_funcs.value, instantiated_pref->identities.value, instantiated_pref->clone_identities.value);
            thisAgent->visualizationManager->graphviz_output += preference_to_char(pAction->preference_type);
            viz_rhs_value(pAction->referent, (variablized_action ? variablized_action->referent : NULL), instantiated_pref->rhs_funcs.referent, instantiated_pref->identities.referent, instantiated_pref->clone_identities.referent, actionID, 'a', VALUE_ELEMENT);
        } else {
            viz_rhs_value(pAction->value, (variablized_action ? variablized_action->value : NULL), instantiated_pref->rhs_funcs.value, instantiated_pref->identities.value, instantiated_pref->clone_identities.value, actionID, 'a', VALUE_ELEMENT);
            thisAgent->visualizationManager->graphviz_output += ' ';
            thisAgent->visualizationManager->graphviz_output += preference_to_char(pAction->preference_type);
        }

        thisAgent->visualizationManager->viz_record_end();
    }
    tempString.clear();
}
