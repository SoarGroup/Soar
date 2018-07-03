#include "action_record.h"

#include "agent.h"
#include "condition.h"
#include "ebc.h"
#include "ebc_identity.h"
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

void simplify_identity_in_rhs_value(agent* thisAgent, rhs_value rv, bool isChunkInstantiation)
{
    if (!rv || (rhs_value_is_reteloc(rv)) || (rhs_value_is_unboundvar(rv))) return;

    if (rhs_value_is_funcall(rv))
    {
        cons* fl = rhs_value_to_funcall_list(rv);
        for (cons* c = fl->rest; c != NIL; c = c->rest)
        {
            simplify_identity_in_rhs_value(thisAgent, static_cast<char*>(c->first), isChunkInstantiation);
        }
        return;
    }

    rhs_symbol r = rhs_value_to_rhs_symbol(rv);
    if (r->identity)
    {
//        thisAgent->outputManager->printa_sf(thisAgent, "%u %u %u %u %u = ", r->identity, r->identity_set->idset_id, r->identity_set->super_join->idset_id, r->identity_set->clone_identity, r->identity_set->super_join->clone_identity);
        uint64_t lID = r->identity->get_identity();
        if (!lID) lID = r->identity->get_clone_identity();
        if (!lID) lID = r->inst_identity;
        r->inst_identity = lID;
        if (!isChunkInstantiation && (r->identity->idset_id != r->inst_identity))
        {
            r->identity_id_unjoined = r->identity->idset_id;
        }
//        thisAgent->outputManager->printa_sf(thisAgent, "%u -> %u\n", r->identity_unjoined, r->identity);
    } else r->inst_identity = LITERAL_VALUE;

    r->identity = NULL;
}

void simplify_identity_in_action(agent* thisAgent, action* pAction, bool isChunkInstantiation)
{

    simplify_identity_in_rhs_value(thisAgent, pAction->id, isChunkInstantiation);
    simplify_identity_in_rhs_value(thisAgent, pAction->attr, isChunkInstantiation);
    simplify_identity_in_rhs_value(thisAgent, pAction->value, isChunkInstantiation);
    if (preference_is_binary(pAction->preference_type))
        simplify_identity_in_rhs_value(thisAgent, pAction->referent, isChunkInstantiation);
}


void simplify_identity_in_preference(agent* thisAgent, preference* pPref, bool isChunkInstantiation)
{
    uint64_t temp;
    if (pPref->identities.id)
    {
//        thisAgent->outputManager->printa_sf(thisAgent, "%u %u %u %u %u = ", pPref->identities.id, pPref->clone_identities.id, pPref->identity_sets.id->idset_id, pPref->identity_sets.id->super_join->idset_id, pPref->identity_sets.id->clone_identity, pPref->identity_sets.id->super_join->clone_identity);

        if (isChunkInstantiation)
        {
            pPref->inst_identities.id = pPref->chunk_inst_identities.id;
            pPref->chunk_inst_identities.id = LITERAL_VALUE;
        }
        else
        {
            pPref->inst_identities.id = pPref->identities.id->joined_identity->idset_id;
            if (pPref->identities.id->idset_id != pPref->inst_identities.id)
            {
                pPref->chunk_inst_identities.id = pPref->identities.id->idset_id;
            } else {
                pPref->chunk_inst_identities.id = LITERAL_VALUE;
            }
        }
//        thisAgent->outputManager->printa_sf(thisAgent, "%u %u\n", pPref->identities.id, pPref->clone_identities.id);
        set_pref_identity(thisAgent, pPref, ID_ELEMENT, NULL_IDENTITY_SET);
    }
    else if (pPref->chunk_inst_identities.id)
    {
        temp = pPref->inst_identities.id;
        pPref->inst_identities.id = pPref->chunk_inst_identities.id;
        if (isChunkInstantiation || (temp == pPref->inst_identities.id))
        {
            pPref->chunk_inst_identities.id = LITERAL_VALUE;
        } else {
            pPref->chunk_inst_identities.id = temp;
        }
    }
    if (pPref->identities.attr)
    {
        if (isChunkInstantiation)
        {
            pPref->inst_identities.attr = pPref->chunk_inst_identities.attr;
            pPref->chunk_inst_identities.attr = LITERAL_VALUE;
        }
        else
        {
            pPref->inst_identities.attr = pPref->identities.attr->joined_identity->idset_id;
            if (pPref->identities.attr->idset_id != pPref->inst_identities.attr)
            {
                pPref->chunk_inst_identities.attr = pPref->identities.attr->idset_id;
            } else {
                pPref->chunk_inst_identities.attr = LITERAL_VALUE;
            }
        }
        set_pref_identity(thisAgent, pPref, ATTR_ELEMENT, NULL_IDENTITY_SET);
    }
    else if (pPref->chunk_inst_identities.attr)
    {
        temp = pPref->inst_identities.attr;
        pPref->inst_identities.attr = pPref->chunk_inst_identities.attr;
        if (isChunkInstantiation || (temp == pPref->inst_identities.attr))
        {
            pPref->chunk_inst_identities.attr = LITERAL_VALUE;
        } else {
            pPref->chunk_inst_identities.attr = temp;
        }
    }

    if (pPref->identities.value)
    {
        if (isChunkInstantiation)
        {
            pPref->inst_identities.value = pPref->chunk_inst_identities.value;
            pPref->chunk_inst_identities.value = LITERAL_VALUE;
        }
        else
        {
            pPref->inst_identities.value = pPref->identities.value->joined_identity->idset_id;
            if (pPref->identities.value->idset_id != pPref->inst_identities.value)
            {
                pPref->chunk_inst_identities.value = pPref->identities.value->idset_id;
            } else {
                pPref->chunk_inst_identities.value = LITERAL_VALUE;
            }
        }
        set_pref_identity(thisAgent, pPref, VALUE_ELEMENT, NULL_IDENTITY_SET);
    }
    else if (pPref->chunk_inst_identities.value)
    {
        temp = pPref->inst_identities.value;
        pPref->inst_identities.value = pPref->chunk_inst_identities.value;
        if (isChunkInstantiation || (temp == pPref->inst_identities.value))
        {
            pPref->chunk_inst_identities.value = LITERAL_VALUE;
        } else {
            pPref->chunk_inst_identities.value = temp;
        }
    }

    if (preference_is_binary(pPref->type))
    {
        if (pPref->identities.referent)
        {
            pPref->inst_identities.referent = pPref->identities.referent->joined_identity->idset_id;
            if (!isChunkInstantiation && (pPref->identities.referent->idset_id != pPref->inst_identities.referent))
            {
                pPref->chunk_inst_identities.referent = pPref->identities.referent->idset_id;
            }
            set_pref_identity(thisAgent, pPref, REFERENT_ELEMENT, NULL_IDENTITY_SET);
        }
        else if (pPref->chunk_inst_identities.referent)
        {
            temp = pPref->inst_identities.referent;
            pPref->inst_identities.referent = pPref->chunk_inst_identities.referent;
            if (isChunkInstantiation || (temp == pPref->inst_identities.referent))
            {
                pPref->chunk_inst_identities.referent = LITERAL_VALUE;
            } else {
                pPref->chunk_inst_identities.referent = temp;
            }
        }
    }
    if (pPref->rhs_func_inst_identities.id) simplify_identity_in_rhs_value(thisAgent, pPref->rhs_func_inst_identities.id, isChunkInstantiation);
    if (pPref->rhs_func_inst_identities.attr) simplify_identity_in_rhs_value(thisAgent, pPref->rhs_func_inst_identities.attr, isChunkInstantiation);
    if (pPref->rhs_func_inst_identities.value) simplify_identity_in_rhs_value(thisAgent, pPref->rhs_func_inst_identities.value, isChunkInstantiation);
    if (pPref->rhs_func_inst_identities.referent) simplify_identity_in_rhs_value(thisAgent, pPref->rhs_func_inst_identities.referent, isChunkInstantiation);
}

void action_record::init(agent* myAgent, preference* pPref, action* pAction, uint64_t pActionID, bool isChunkInstantiation)
{
    thisAgent               = myAgent;
    actionID                = pActionID;
    instantiated_pref       = shallow_copy_preference(thisAgent, pPref);
    original_pref           = pPref;

    simplify_identity_in_preference(thisAgent, instantiated_pref, isChunkInstantiation);

    if (pAction)
    {
        variablized_action = copy_action(thisAgent, pAction);
        simplify_identity_in_action(thisAgent, variablized_action, isChunkInstantiation);
    } else {
        variablized_action = NULL;
    }
    identities_used = NULL;
}

void action_record::clean_up()
{
    deallocate_preference(thisAgent, instantiated_pref, true);
    deallocate_action_list(thisAgent, variablized_action);
    if (identities_used)
    {
        delete identities_used;
    }
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
            thisAgent->outputManager->rhs_value_to_string(pPref_func, tempString, true, NULL, NULL, false);
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
        outputManager->printa_sf(thisAgent, "%d:%-%s", lActionCount,  tempString.c_str());
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
        print_rhs_instantiation_value(pAction->id, instantiated_pref->rhs_func_inst_identities.id, instantiated_pref->inst_identities.id, instantiated_pref->chunk_inst_identities.id, false);
        outputManager->printa(thisAgent, " ^");
        print_rhs_instantiation_value(pAction->attr, instantiated_pref->rhs_func_inst_identities.attr, instantiated_pref->inst_identities.attr, instantiated_pref->chunk_inst_identities.attr, false);
        outputManager->printa(thisAgent, " ");
        print_rhs_instantiation_value(pAction->value, instantiated_pref->rhs_func_inst_identities.value, instantiated_pref->inst_identities.value, instantiated_pref->chunk_inst_identities.value, false);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_instantiation_value(pAction->referent, instantiated_pref->rhs_func_inst_identities.referent, instantiated_pref->inst_identities.referent, instantiated_pref->chunk_inst_identities.referent, false);
        }

        outputManager->printa(thisAgent, ")");
    }
    outputManager->printa(thisAgent, "\n");
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
        outputManager->printa_sf(thisAgent, "%d:%-%s", lActionCount,  tempString.c_str());
    } else {
        outputManager->printa_sf(thisAgent, "%d:%-(", lActionCount);
        print_rhs_instantiation_value(pAction->id, NULL, instantiated_pref->inst_identities.id, instantiated_pref->chunk_inst_identities.id, true);
        outputManager->printa(thisAgent, " ^");
        print_rhs_instantiation_value(pAction->attr, NULL, instantiated_pref->inst_identities.attr, instantiated_pref->chunk_inst_identities.attr, true);
        outputManager->printa(thisAgent, " ");
        print_rhs_instantiation_value(pAction->value, NULL, instantiated_pref->inst_identities.value, instantiated_pref->chunk_inst_identities.value, true);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_instantiation_value(pAction->referent, NULL, instantiated_pref->inst_identities.referent, instantiated_pref->chunk_inst_identities.referent, true);
        }
        outputManager->printa_sf(thisAgent, ")%-(");
        print_rhs_instantiation_value(pAction->id, instantiated_pref->rhs_func_inst_identities.id, instantiated_pref->inst_identities.id, instantiated_pref->chunk_inst_identities.id, false);
        outputManager->printa(thisAgent, " ^");
        print_rhs_instantiation_value(pAction->attr, instantiated_pref->rhs_func_inst_identities.attr, instantiated_pref->inst_identities.attr, instantiated_pref->chunk_inst_identities.attr, false);
        outputManager->printa(thisAgent, " ");
        print_rhs_instantiation_value(pAction->value, instantiated_pref->rhs_func_inst_identities.value, instantiated_pref->inst_identities.value, instantiated_pref->chunk_inst_identities.value, false);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_instantiation_value(pAction->referent, instantiated_pref->rhs_func_inst_identities.referent, instantiated_pref->inst_identities.referent, instantiated_pref->chunk_inst_identities.referent, false);
        }
        outputManager->printa(thisAgent, ")");
    }
    outputManager->printa(thisAgent, "\n");
    tempString.clear();
}

void action_record::viz_rhs_value(const rhs_value pRHS_value, const rhs_value pRHS_variablized_value, const rhs_value pRHS_func, uint64_t pID, uint64_t pIDClone, uint64_t pNodeID, char pTypeChar, WME_Field pField)
{
    std::string tempString;
    bool identity_printed = false;
    tempString = "";
    std::string highlight_str;
    if ((thisAgent->visualizationManager->settings->use_joined_identities->get_value() == on) || !pIDClone)
        highlight_str = thisAgent->visualizationManager->get_color_for_id(pID);
    else
        highlight_str = thisAgent->visualizationManager->get_color_for_id(pIDClone);

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
                if (rhs)
                {
                    while (rhs && (rhs->type == FUNCALL_ACTION))
                    {
                        lAction->viz_action(rhs);
                        rhs = rhs->next;
                    }
                    lAction->viz_action(rhs);
                    rhs = rhs->next;
                    while (rhs && (rhs->type == FUNCALL_ACTION))
                    {
                        lAction->viz_action(rhs);
                        rhs = rhs->next;
                    }
                } else {
                    /* For deep copy */
                    lAction->viz_preference();
                }
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
        thisAgent->visualizationManager->viz_table_element_start(actionID, 'a', ID_ELEMENT, false, "COLSPAN=\"3\" ");
        tempString = "";
        thisAgent->outputManager->rhs_value_to_string(pAction->value, tempString);
        thisAgent->visualizationManager->graphviz_output += tempString;
        thisAgent->visualizationManager->viz_table_element_end();
        thisAgent->visualizationManager->viz_record_end();
    } else {
        thisAgent->visualizationManager->viz_record_start();

        viz_rhs_value(pAction->id, (variablized_action ? variablized_action->id : NULL), instantiated_pref->rhs_func_inst_identities.id, instantiated_pref->inst_identities.id, instantiated_pref->chunk_inst_identities.id, actionID, 'a', ID_ELEMENT);
        viz_rhs_value(pAction->attr, (variablized_action ? variablized_action->attr : NULL), instantiated_pref->rhs_func_inst_identities.attr, instantiated_pref->inst_identities.attr, instantiated_pref->chunk_inst_identities.attr);

        if (pAction->referent)
        {
            viz_rhs_value(pAction->value, (variablized_action ? variablized_action->value : NULL), instantiated_pref->rhs_func_inst_identities.value, instantiated_pref->inst_identities.value, instantiated_pref->chunk_inst_identities.value);
            thisAgent->visualizationManager->graphviz_output += preference_to_char(pAction->preference_type);
            viz_rhs_value(pAction->referent, (variablized_action ? variablized_action->referent : NULL), instantiated_pref->rhs_func_inst_identities.referent, instantiated_pref->inst_identities.referent, instantiated_pref->chunk_inst_identities.referent, actionID, 'a', VALUE_ELEMENT);
        } else {
            viz_rhs_value(pAction->value, (variablized_action ? variablized_action->value : NULL), instantiated_pref->rhs_func_inst_identities.value, instantiated_pref->inst_identities.value, instantiated_pref->chunk_inst_identities.value, actionID, 'a', VALUE_ELEMENT);
            thisAgent->visualizationManager->graphviz_output += ' ';
            thisAgent->visualizationManager->graphviz_output += preference_to_char(pAction->preference_type);
        }

        thisAgent->visualizationManager->viz_record_end();
    }
    tempString.clear();
}
