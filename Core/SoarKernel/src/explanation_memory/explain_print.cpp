#include "ebc.h"

#include "action_record.h"
#include "agent.h"
#include "condition_record.h"
#include "condition.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "identity_record.h"
#include "instantiation_record.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production_record.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "symbol_manager.h"
#include "symbol.h"
#include "test.h"
#include "working_memory.h"

void Explanation_Memory::switch_to_explanation_trace(bool pEnableExplanationTrace)
{
    print_explanation_trace = pEnableExplanationTrace;
    if (!last_printed_id)
    {
        print_chunk_explanation();
    } else {
        print_instantiation_explanation_for_id(last_printed_id);
    }
}

void Explanation_Memory::print_formation_explanation()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "------------------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "The formation of chunk '%y' (c %u) \n", current_discussed_chunk->name, current_discussed_chunk->chunkID);
    outputManager->printa_sf(thisAgent, "------------------------------------------------------------------------------------\n\n");

    if (current_discussed_chunk->result_inst_records->size() > 0)
    {
        outputManager->printa_sf(thisAgent, "The following %d instantiations fired to produce results...\n\n------\n\n",
        current_discussed_chunk->result_inst_records->size() + 1);
    }

    outputManager->printa_sf(thisAgent, "Initial base instantiation (i %u) that fired when %y matched at level %d at time %u:\n\n",
        current_discussed_chunk->baseInstantiation->instantiationID,
        current_discussed_chunk->baseInstantiation->production_name,
        current_discussed_chunk->baseInstantiation->match_level,
        current_discussed_chunk->time_formed);

    print_instantiation_explanation(current_discussed_chunk->baseInstantiation, false);

    if (current_discussed_chunk->result_inst_records->size() > 0)
    {
        outputManager->printa_sf(thisAgent, "\n%d instantiation(s) that created extra results indirectly because they were connected to the results of the base instantiation:\n\n", (current_discussed_chunk->result_inst_records->size() - 1));
        for (auto it = current_discussed_chunk->result_inst_records->begin(); it != current_discussed_chunk->result_inst_records->end(); ++it)
        {
            print_instantiation_explanation((*it), false);
        }
    }
    outputManager->printa(thisAgent, "------\n");
    print_involved_instantiations();
    print_footer(true);
}

void Explanation_Memory::print_footer(bool pPrintDiscussedChunkCommands)
{
    outputManager->printa(thisAgent, "---------------------------------------------------------------------------------------------------------------------\n");
    outputManager->set_column_indent(0, 0);
    outputManager->set_column_indent(1, 16);
    outputManager->set_column_indent(2, 70);
    outputManager->set_column_indent(3, 83);
    if (print_explanation_trace)
    {
        outputManager->printa_sf(thisAgent, "- explain f %-Explain initial formation of chunk %-explain w %-Switch to working memory trace    -\n");
    } else {
        outputManager->printa_sf(thisAgent, "- explain f %-Explain initial formation of chunk %-explain e %-Switch to explanation trace       -\n");
    }
    outputManager->printa_sf(thisAgent, "- explain c %-Explain constraints required by problem-solving %-explain i %-Explain identity analysis         -\n");
    outputManager->printa_sf(thisAgent, "- explain s %-Print chunk statistics %-chunk stats %-Print overall chunk statistics    -\n");
    outputManager->printa(thisAgent, "---------------------------------------------------------------------------------------------------------------------\n");

}

bool Explanation_Memory::is_condition_related(condition_record* pCondRecord)
{
    //    if ((pCondRecord->condition_tests.id->eq_test->identity == current_explained_ids.id) ||
    //        (pCondRecord->condition_tests.attr->eq_test->identity == current_explained_ids.id) ||
    //        (pCondRecord->condition_tests.value->eq_test->identity == current_explained_ids.id))
    //    {
    //    }
    return false;
}

void Explanation_Memory::print_path_to_base(const inst_record_list* pPathToBase, bool pPrintFinal, const char* pFailedStr, const char* pHeaderStr)
{
    if (pPathToBase && (!pPathToBase->empty()))
    {
        int min_size = (pPrintFinal ? 1 : 2);
        if (pPathToBase->size() >= min_size)
        {
            if (pHeaderStr) outputManager->printa(thisAgent, pHeaderStr);
            bool notFirst = false;
            for (auto it = (pPathToBase)->rbegin(); it != (pPathToBase)->rend(); it++)
            {
                if (notFirst)
                {
                    thisAgent->outputManager->printa(thisAgent, " -> ");
                }
                thisAgent->outputManager->printa_sf(thisAgent, "i %u", (*it)->get_instantiationID());
                notFirst = true;
            }
        } else if (pFailedStr)
        {
            outputManager->printa(thisAgent, pFailedStr);
        }
    }
    outputManager->printa(thisAgent, "\n");
}

void Explanation_Memory::print_chunk_actions(action_record_list* pActionRecords, production* pOriginalRule, production_record* pExcisedRule)
{

    if (pActionRecords->empty())
    {
        outputManager->printa(thisAgent, "No actions on right-hand-side\n");
    }
    else
    {
        action_record* lAction;
        condition* top = NULL, *bottom = NULL;
        action* rhs, *pRhs = NULL;
        int lActionCount = 0;
        thisAgent->outputManager->set_print_indents("");
        thisAgent->outputManager->set_print_test_format(true, false);
        if (print_explanation_trace)
        {
            /* We use pRhs to deallocate actions at end, and rhs to iterate through actions */
            if (!pOriginalRule || !pOriginalRule->p_node)
            {
                if (pExcisedRule)
                {
                    rhs = pExcisedRule->get_rhs();
                    assert(rhs);
                } else {
                    outputManager->printa_sf(thisAgent, "No rule for this instantiation found in RETE\n");
                    return;
                }
            } else {
                p_node_to_conditions_and_rhs(thisAgent, pOriginalRule->p_node, NIL, NIL, &top, &bottom, &rhs);
                pRhs = rhs;
            }
        }
        for (action_record_list::iterator it = pActionRecords->begin(); it != pActionRecords->end(); it++)
        {
            lAction = (*it);
            ++lActionCount;
            if (!print_explanation_trace)
            {
                outputManager->printa_sf(thisAgent, "%d:%-%p\n", lActionCount, lAction->instantiated_pref);
            } else {
                lAction->print_chunk_action(rhs, lActionCount);
                rhs = rhs->next;
            }
        }
        if (print_explanation_trace)
        {
            /* If top exists, we generated conditions here and must deallocate. */
            if (pRhs) deallocate_action_list(thisAgent, pRhs);
            if (top) deallocate_condition_list(thisAgent, top);
        }
        thisAgent->outputManager->clear_print_test_format();
    }

}
void Explanation_Memory::print_instantiation_actions(action_record_list* pActionRecords, production* pOriginalRule, action* pRhs)
{

    if (pActionRecords->empty())
    {
        outputManager->printa(thisAgent, "No actions on right-hand-side\n");
    }
    else
    {
        action_record* lAction;
        action* rhs;
        int lActionCount = 0;
        thisAgent->outputManager->set_print_indents("");
        thisAgent->outputManager->set_print_test_format(true, false);
        if (print_explanation_trace)
        {
            /* We use pRhs to deallocate actions at end, and rhs to iterate through actions */
            rhs = pRhs;
        }
        for (action_record_list::iterator it = pActionRecords->begin(); it != pActionRecords->end(); it++)
        {
            lAction = (*it);
            ++lActionCount;
            if (!print_explanation_trace)
            {
                outputManager->printa_sf(thisAgent, "%d:%-%p\n", lActionCount, lAction->instantiated_pref);
            } else {
                lAction->print_instantiation_action(rhs, lActionCount);
                rhs = rhs->next;
            }
        }
        if (print_explanation_trace)
        {
            deallocate_action_list(thisAgent, pRhs);
        }
        thisAgent->outputManager->clear_print_test_format();
    }

}

void action_record::print_chunk_action(action* pAction, int lActionCount)
{
    std::string tempString;
    Output_Manager* outputManager = thisAgent->outputManager;

    if (pAction->type == FUNCALL_ACTION)
    {
        tempString = "";
        outputManager->rhs_value_to_string(pAction->value, tempString, NULL, NULL, true);
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
        print_rhs_instantiation_value(pAction->id, instantiated_pref->rhs_funcs.id, instantiated_pref->identities.id, false);
        outputManager->printa(thisAgent, " ^");
        print_rhs_instantiation_value(pAction->attr, instantiated_pref->rhs_funcs.attr, instantiated_pref->identities.attr, false);
        outputManager->printa(thisAgent, " ");
        print_rhs_instantiation_value(pAction->value, instantiated_pref->rhs_funcs.value, instantiated_pref->identities.value, false);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_instantiation_value(pAction->referent, NULL, instantiated_pref->identities.referent, false);
        }
//        print_rhs_chunk_value(pAction->id, (variablized_action ? variablized_action->id : NULL), false);
//        outputManager->printa(thisAgent, " ^");
//        print_rhs_chunk_value(pAction->attr, (variablized_action ? variablized_action->attr : NULL), false);
//        outputManager->printa(thisAgent, " ");
//        print_rhs_chunk_value(pAction->value, (variablized_action ? variablized_action->value : NULL), false);
//        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
//        if (pAction->referent)
//        {
//            print_rhs_chunk_value(pAction->referent, (variablized_action ? variablized_action->referent : NULL), false);
//        }
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
        outputManager->rhs_value_to_string(pAction->value, tempString, NULL, NULL, true);
        outputManager->printa_sf(thisAgent, "%d:%-%s%-%s", lActionCount,  tempString.c_str(), tempString.c_str());
    } else {
        outputManager->printa_sf(thisAgent, "%d:%-(", lActionCount);
        print_rhs_instantiation_value(pAction->id, NULL, instantiated_pref->identities.id, true);
        outputManager->printa(thisAgent, " ^");
        print_rhs_instantiation_value(pAction->attr, NULL, instantiated_pref->identities.attr, true);
        outputManager->printa(thisAgent, " ");
        print_rhs_instantiation_value(pAction->value, NULL, instantiated_pref->identities.value, true);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_instantiation_value(pAction->referent, NULL, instantiated_pref->identities.referent, true);
        }
        outputManager->printa_sf(thisAgent, ")%-(");
        print_rhs_instantiation_value(pAction->id, instantiated_pref->rhs_funcs.id, instantiated_pref->identities.id, false);
        outputManager->printa(thisAgent, " ^");
        print_rhs_instantiation_value(pAction->attr, instantiated_pref->rhs_funcs.attr, instantiated_pref->identities.attr, false);
        outputManager->printa(thisAgent, " ");
        print_rhs_instantiation_value(pAction->value, instantiated_pref->rhs_funcs.value, instantiated_pref->identities.value, false);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_instantiation_value(pAction->referent, NULL, instantiated_pref->identities.referent, false);
        }
        outputManager->printa(thisAgent, ")\n");
    }
    tempString.clear();
}

void Explanation_Memory::print_instantiation_explanation(instantiation_record* pInstRecord, bool printFooter)
{
    if (print_explanation_trace)
    {
        pInstRecord->print_for_explanation_trace(printFooter);
    } else {
        pInstRecord->print_for_wme_trace(printFooter);
    }
}

void Explanation_Memory::print_chunk_explanation()
{
    assert(current_discussed_chunk);

    if (print_explanation_trace)
    {
        current_discussed_chunk->print_for_explanation_trace();
    } else {
        current_discussed_chunk->print_for_wme_trace();
    }
}

void Explanation_Memory::print_explain_summary()
{
    outputManager->set_column_indent(0, 55);
    outputManager->set_column_indent(1, 54);
    outputManager->printa_sf(thisAgent,    "%f=======================================================\n");
    outputManager->printa(thisAgent,    "                       Explainer Summary\n");
    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa_sf(thisAgent, "Watch all chunk formations        %-%s\n", (m_all_enabled ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "Explain justifications            %-%s\n", (m_justifications_enabled ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "Number of specific rules watched  %-%d\n", num_rules_watched);

    /* Print specific watched rules and time interval when watch all disabled */
    if (!m_all_enabled)
    {
        /* Print last 10 rules watched*/
        outputManager->printa_sf(thisAgent, "Rules watched:");
        print_rules_watched(10);
    }

    outputManager->printa(thisAgent, "\n");
    outputManager->printa_sf(thisAgent, "Chunks available for discussion:");
    print_chunk_list(10);

    outputManager->printa(thisAgent, "\n");

    /* Print current chunk and last 10 chunks formed */
    outputManager->printa_sf(thisAgent, "Current chunk being discussed: %-%s",
        (current_discussed_chunk ? current_discussed_chunk->name->sc->name : "None" ));
    if (current_discussed_chunk)
    {
        outputManager->printa_sf(thisAgent, "(c %u)\n\n", current_discussed_chunk->chunkID);
    } else {
        outputManager->printa(thisAgent, "\n\n");
    }
    outputManager->printa(thisAgent, "Use 'explain chunk [ <chunk-name> | id ]' to discuss the formation of that chunk.\n");
    outputManager->printa_sf(thisAgent, "Use 'explain ?' to learn more about explain's sub-command and settings.\n");
}

void Explanation_Memory::print_all_watched_rules()
{
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 0);
    outputManager->set_column_indent(1, 4);
    outputManager->printa(thisAgent, "Rules watched:\n");
    print_rules_watched(0);
}


void Explanation_Memory::print_all_chunks()
{
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 0);
    outputManager->set_column_indent(1, 4);
    outputManager->printa(thisAgent, "Chunks available for discussion:\n");
    print_chunk_list(0);
}

void Explanation_Memory::print_global_stats()
{
    outputManager->set_column_indent(0, 60);
    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "            Explanation-Based Chunking Statistics\n");
    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "Sub-states analyzed                            %-%u\n", stats.chunks_attempted);
    outputManager->printa_sf(thisAgent, "Rules learned                                  %-%u\n", stats.chunks_succeeded);
    outputManager->printa_sf(thisAgent, "Learning failures reverted to justifications   %-%u\n", stats.chunks_reverted);

    outputManager->printa_sf(thisAgent, "\nJustifications attempted                   %- %u\n", stats.justifications_attempted);
    outputManager->printa_sf(thisAgent, "Justifications successfully built          %-%u\n", stats.justifications_succeeded);

    outputManager->printa_sf(thisAgent, "\nInstantiations built                     %- %u\n", thisAgent->explanationBasedChunker->get_instantiation_count());
    outputManager->printa_sf(thisAgent, "Instantiations backtraced through          %-%u\n", stats.instantations_backtraced);
    outputManager->printa_sf(thisAgent, "Instantiations re-visited                  %-%u\n", stats.seen_instantations_backtraced);

    outputManager->printa_sf(thisAgent, "\nConditions merged                        %- %u\n", stats.merged_conditions);
    outputManager->printa_sf(thisAgent, "Constraints collected                      %-%u\n", stats.constraints_collected);
    outputManager->printa_sf(thisAgent, "Constraints attached                       %-%u\n", stats.constraints_attached);

    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "          Partially Operational Conditions and Actions\n");
    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "LHS with conditions unconnected to goal    %-%u\n", stats.lhs_unconnected);
    outputManager->printa_sf(thisAgent, "RHS with actions unconnected to goal       %-%u\n", stats.rhs_unconnected);

    outputManager->printa_sf(thisAgent, "Repair attempted but failed                %-%u\n", stats.repair_failed);
    outputManager->printa_sf(thisAgent, "Chunks reverted to justifications          %-%u\n", stats.chunks_reverted);

    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "                Experimental Justification Repair\n");
    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "Justifications needing repair              %-%u\n", stats.rhs_unconnected);
    outputManager->printa_sf(thisAgent, "Justifications repaired                    %-%u\n", stats.justifications_repaired);
    outputManager->printa_sf(thisAgent, "Justification repair failed yet added      %-%u\n", stats.ungrounded_justifications_added);
    outputManager->printa_sf(thisAgent, "Justification repair failed and ignored    %-%u\n", stats.ungrounded_justifications_ignored);

    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "             Potential Generality Issues Detected\n");
    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "Chunks repaired                            %-%u\n", stats.chunks_repaired);
    outputManager->printa_sf(thisAgent, "Conditions added during repair             %-%u\n", stats.grounding_conditions_added);

    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "            Potential Correctness Issues Detected\n");
    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "Used negated reasoning about substate      %-%u\n", stats.tested_local_negation);
    outputManager->printa_sf(thisAgent, "Chunk learned did not match WM             %-%u\n", stats.chunk_did_not_match);
    outputManager->printa_sf(thisAgent, "Justification learned did not match WM     %-%u\n", stats.justification_did_not_match);

    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "                     Learning Failures\n");
    outputManager->printa_sf(thisAgent, "=============================================================\n");
    outputManager->printa_sf(thisAgent, "Duplicate of existing rule                 %-%u\n", stats.duplicates);
    outputManager->printa_sf(thisAgent, "Could not repair invalid rule              %-%u\n", stats.repair_failed);
    outputManager->printa_sf(thisAgent, "Skipped because of max-chunks              %-%u\n", stats.max_chunks);
    outputManager->printa_sf(thisAgent, "Skipped because of max-dupes               %-%u\n", stats.max_dupes);
    outputManager->printa_sf(thisAgent, "Backtracing produced no conditions         %-%u\n", stats.no_grounds);
}


void Explanation_Memory::print_chunk_stats() {

    assert(current_discussed_chunk);
    outputManager->set_column_indent(0, 60);
    outputManager->printa_sf(thisAgent, "===========================================================\n");
    outputManager->printa_sf(thisAgent, "%fStatistics for '%y' (c %u):\n",                         current_discussed_chunk->name, current_discussed_chunk->chunkID);
    outputManager->printa_sf(thisAgent, "===========================================================\n");
    outputManager->printa_sf(thisAgent, "Number of conditions           %-%u\n",          current_discussed_chunk->conditions->size());
    outputManager->printa_sf(thisAgent, "Number of actions              %-%u\n",          current_discussed_chunk->actions->size());
    outputManager->printa_sf(thisAgent, "Base instantiation             %-i %u (%y)\n",    current_discussed_chunk->baseInstantiation->instantiationID, current_discussed_chunk->baseInstantiation->production_name);
    if (current_discussed_chunk->result_inst_records->size() > 0)
    {
        outputManager->printa_sf(thisAgent, "Extra result instantiations: " );
        for (auto it = current_discussed_chunk->result_inst_records->begin(); it != current_discussed_chunk->result_inst_records->end(); ++it)
        {
            outputManager->printa_sf(thisAgent, "%-i %u (%y)\n", (*it)->instantiationID, (*it)->production_name);
        }
    }
    outputManager->printa_sf(thisAgent, "\n===========================================================\n");
    outputManager->printa_sf(thisAgent, "                 Generality and Correctness\n");
    outputManager->printa_sf(thisAgent, "===========================================================\n");
    outputManager->printa(thisAgent, "\n");
    outputManager->printa_sf(thisAgent, "Tested negation in local substate          %-%s\n", (current_discussed_chunk->stats.tested_local_negation ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "Rule learned did not match WM              %-%s\n", (current_discussed_chunk->stats.did_not_match_wm ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "Repaired unconnected identifiers           %-%s\n", ((current_discussed_chunk->stats.num_grounding_conditions_added > 0) ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "- LHS conditions not connected to goal   %-%s\n", (current_discussed_chunk->stats.lhs_unconnected ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "- RHS conditions not connected to goal   %-%s\n", (current_discussed_chunk->stats.rhs_unconnected ? "Yes" : "No"));
    if (current_discussed_chunk->stats.num_grounding_conditions_added > 0)
    {
        outputManager->printa_sf(thisAgent, "- Repaired conditions added                  %-%u\n", current_discussed_chunk->stats.num_grounding_conditions_added);
    } else {
        outputManager->printa_sf(thisAgent, "- Tried to repair but could not        %-%s\n", (current_discussed_chunk->stats.repair_failed ? "Yes" : "No"));
        outputManager->printa_sf(thisAgent, "- Reverted to justification            %-%s\n", (current_discussed_chunk->stats.reverted ? "Yes" : "No"));
    }
    outputManager->printa_sf(thisAgent, "\n===========================================================\n");
    outputManager->printa_sf(thisAgent, "                      Work Performed\n");
    outputManager->printa_sf(thisAgent, "===========================================================\n");
    outputManager->printa_sf(thisAgent, "Instantiations backtraced through          %-%u\n", current_discussed_chunk->stats.instantations_backtraced);
    outputManager->printa_sf(thisAgent, "Instantiations skipped                     %-%u\n", current_discussed_chunk->stats.seen_instantations_backtraced);
    outputManager->printa_sf(thisAgent, "Constraints collected                      %-%u\n", current_discussed_chunk->stats.constraints_collected);
    outputManager->printa_sf(thisAgent, "Constraints attached                       %-%u\n", current_discussed_chunk->stats.constraints_attached);
    outputManager->printa_sf(thisAgent, "Duplicates chunks later created            %-%u\n", current_discussed_chunk->stats.duplicates);
    outputManager->printa_sf(thisAgent, "Conditions merged                          %-%u\n", current_discussed_chunk->stats.merged_conditions);
}

void Explanation_Memory::print_chunk_list(short pNumToPrint)
{
    short lNumPrinted = 0;
    for (std::unordered_map< Symbol*, chunk_record* >::iterator it = (*chunks).begin(); it != (*chunks).end(); ++it)
    {
        Symbol* d1 = it->first;
        chunk_record* d2 = it->second;
        outputManager->printa_sf(thisAgent, "%-%-%y (c%u)\n", it->first, it->second->chunkID);
        if (pNumToPrint && (++lNumPrinted >= pNumToPrint))
        {
            break;
        }
    }
    if (pNumToPrint && ((*chunks).size() > pNumToPrint))
    {
        outputManager->printa_sf(thisAgent, "\n* Note:  Only printed the first %d chunks.  Type 'explain list' to see the other %d chunks.\n", pNumToPrint, ( (*chunks).size() - pNumToPrint));
    }
}

bool Explanation_Memory::print_watched_rules_of_type(agent* thisAgent, unsigned int productionType, short &pNumToPrint)
{
    short lNumPrinted = 0;
    bool lThereWasMore = false;

    for (production* prod = thisAgent->all_productions_of_type[productionType]; prod != NIL; prod = prod->next)
    {
        assert(prod->name);
        if (prod->explain_its_chunks)
        {
            outputManager->printa_sf(thisAgent, "%-%-%y\n", prod->name);
            if (pNumToPrint && (++lNumPrinted >= pNumToPrint))
            {
                if (prod->next)
                {
                    lThereWasMore = true;
                }
                break;
            }
        }
    }
    if (pNumToPrint)
    {
        pNumToPrint -= lNumPrinted;
    }
    assert (pNumToPrint >= 0);
    return lThereWasMore;
}

void Explanation_Memory::print_rules_watched(short pNumToPrint)
{
    short lNumLeftToPrint = pNumToPrint;
    bool lThereWasMore = false;

    lThereWasMore = print_watched_rules_of_type(thisAgent, USER_PRODUCTION_TYPE, lNumLeftToPrint);
    if (!lThereWasMore)
    {
        lThereWasMore = print_watched_rules_of_type(thisAgent, CHUNK_PRODUCTION_TYPE, lNumLeftToPrint);
    }
    if (!lThereWasMore)
    {
        lThereWasMore = print_watched_rules_of_type(thisAgent, JUSTIFICATION_PRODUCTION_TYPE, lNumLeftToPrint);
    }
    if (!lThereWasMore)
    {
        lThereWasMore = print_watched_rules_of_type(thisAgent, DEFAULT_PRODUCTION_TYPE, lNumLeftToPrint);
    }
    if (!lThereWasMore)
    {
        lThereWasMore = print_watched_rules_of_type(thisAgent, TEMPLATE_PRODUCTION_TYPE, lNumLeftToPrint);
    }
    if (lThereWasMore)
    {
        outputManager->printa_sf(thisAgent, "\n* Note:  Only printed the first %d rules.  Type 'explain watch' to see the other %d rules.\n", pNumToPrint, lNumLeftToPrint);
    }
}

void Explanation_Memory::print_condition_explanation(uint64_t pCondID)
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "Printing explanation of condition %u in relation to chunk %y.\n", pCondID, current_discussed_chunk->name);
}

void Explanation_Memory::print_identity_set_explanation()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "=========================================================================\n");
    outputManager->printa_sf(thisAgent, "-             Variablization Identity to Identity Set Mappings          -\n");
    outputManager->printa_sf(thisAgent, "=========================================================================\n");
    current_discussed_chunk->identity_analysis.print_mappings();
}

void Explanation_Memory::print_constraints_enforced()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "Constraints enforced during formation of chunk %y.\n\nNot yet implemented.\n", current_discussed_chunk->name);
}


void Explanation_Memory::print_involved_instantiations()
{
    // Attempt to sort that wasn't compiling and didn't have time to figure out
    //    struct cmp_iID
    //        {
    //            bool operator () (const instantiation_record& a, const instantiation_record& b)
    //            {
    //                  return (a.instantiationID <= b.instantiationID);
    //            }
    //        };
    //    std::set< instantiation_record*, cmp_iID > sorted_set;
    ////    { std::begin((*instantiations_for_current_chunk)), std::end((*instantiations_for_current_chunk)) };
    //    std::copy(std::begin(instantiations_for_current_chunk), std::end(instantiations_for_current_chunk), std::inserter(sorted_set));
    assert(current_discussed_chunk);

    outputManager->printa_sf(thisAgent, "This chunk summarizes the problem-solving involved in the following %d rule firings:\n\n", current_discussed_chunk->backtraced_inst_records->size());

    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end();++it)
    {
        outputManager->printa_sf(thisAgent, "   i %u (%y)\n", (*it)->instantiationID, (*it)->production_name);
    }
    outputManager->printa(thisAgent, "\n");
}
