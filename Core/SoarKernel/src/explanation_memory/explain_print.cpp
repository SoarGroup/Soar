#include "ebc.h"

#include "action_record.h"
#include "agent.h"
#include "condition_record.h"
#include "condition.h"
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
        static_cast<int64_t>(current_discussed_chunk->result_inst_records->size() + 1));
    }

    outputManager->printa_sf(thisAgent, "Initial base instantiation (i %u) that fired when %y matched at level %d at time %u:\n\n",
        current_discussed_chunk->baseInstantiation->instantiationID,
        current_discussed_chunk->baseInstantiation->production_name,
        static_cast<int64_t>(current_discussed_chunk->baseInstantiation->match_level),
        current_discussed_chunk->time_formed);

    print_instantiation_explanation(current_discussed_chunk->baseInstantiation, false);

    if (current_discussed_chunk->result_inst_records->size() > 0)
    {
        outputManager->printa_sf(thisAgent, "\n%d instantiation(s) that created extra results indirectly because they were connected to the results of the base instantiation:\n\n", static_cast<int64_t>((current_discussed_chunk->result_inst_records->size() - 1)));
        for (auto it = current_discussed_chunk->result_inst_records->begin(); it != current_discussed_chunk->result_inst_records->end(); ++it)
        {
            print_instantiation_explanation((*it), false);
        }
    }
    outputManager->printa(thisAgent, "\n");
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
                outputManager->printa_sf(thisAgent, "%d:%-%p\n", static_cast<int64_t>(lActionCount), lAction->instantiated_pref);
            } else {
                while (rhs && (rhs->type == FUNCALL_ACTION))
                {
                    lAction->print_chunk_action(rhs, lActionCount);
                    ++lActionCount;
                    rhs = rhs->next;
                }
                lAction->print_chunk_action(rhs, lActionCount);
                rhs = rhs->next;
                while (rhs && (rhs->type == FUNCALL_ACTION))
                {
                    lAction->print_chunk_action(rhs, lActionCount);
                    ++lActionCount;
                    rhs = rhs->next;
                }
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
        action* rhs, *lThisAction;
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
                outputManager->printa_sf(thisAgent, "%d:%-%p\n", static_cast<int64_t>(lActionCount), lAction->instantiated_pref);
            } else {
                while (rhs && (rhs->type == FUNCALL_ACTION))
                {
                    lAction->print_instantiation_action(rhs, lActionCount);
                    ++lActionCount;
                    rhs = rhs->next;
                }
                lAction->print_instantiation_action(rhs, lActionCount);
                rhs = rhs->next;
                while (rhs && (rhs->type == FUNCALL_ACTION))
                {
                    lAction->print_instantiation_action(rhs, lActionCount);
                    ++lActionCount;
                    rhs = rhs->next;
                }
            }
        }
        if (print_explanation_trace)
        {
            deallocate_action_list(thisAgent, pRhs);
        }
        thisAgent->outputManager->clear_print_test_format();
    }
}

void Explanation_Memory::print_instantiation_explanation(instantiation_record* pInstRecord, bool printFooter)
{
    if (print_explanation_trace)
    {
        pInstRecord->print_for_explanation_trace(false, printFooter);
    } else {
        pInstRecord->print_for_wme_trace(false, printFooter);
    }
}

void Explanation_Memory::print_chunk_explanation()
{
    assert(current_discussed_chunk);

    if (print_explanation_trace)
    {
        current_discussed_chunk->chunkInstantiation->print_for_explanation_trace(true, false);
    } else {
        current_discussed_chunk->chunkInstantiation->print_for_wme_trace(true, false);
    }
//    print_chunk_stats(current_discussed_chunk, false);
//    outputManager->printa(thisAgent, "\n");
//    thisAgent->explanationMemory->print_footer(true);
}

void Explanation_Memory::print_explain_summary()
{
    outputManager->set_column_indent(0, 55);
    outputManager->printa_sf(thisAgent, "%e=======================================================\n");
    outputManager->printa(thisAgent,      "                   Explainer Summary\n");
    outputManager->printa(thisAgent,      "=======================================================\n");
    outputManager->printa_sf(thisAgent,   "Watch all chunk formations        %-%s\n", (m_all_enabled ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent,   "Explain justifications            %-%s\n", (m_justifications_enabled ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent,   "Number of specific rules watched  %-%d\n", static_cast<int64_t>(num_rules_watched));

    /* Print specific watched rules and time interval when watch all disabled */
    if (!m_all_enabled)
    {
        /* Print last 10 rules watched*/
        outputManager->printa_sf(thisAgent, "Rules watched:");
        print_rules_watched(10);
    }
    outputManager->printa(thisAgent, "\n");

    /* Print current chunk and last 10 chunks formed */
    if (current_discussed_chunk)
    {
        outputManager->printa_sf(thisAgent, "Current rule being explained: %-%s (c %u)\n\n", current_discussed_chunk->name->sc->name, current_discussed_chunk->chunkID);
    } else {
        outputManager->printa(thisAgent, "No rule is currently being explained.\n");
    }
    list_explained_rules(10, true, false);
    outputManager->printa(thisAgent, "\n");
    list_explained_rules(10, false, false);
    outputManager->printa(thisAgent, "\n");

    outputManager->printa(thisAgent,    "-------------------------------------------------------\n\n");
    outputManager->printa(thisAgent, "Use 'explain chunk [ <chunk-name> | id ]' to explore the formation of that chunk.\n");
    outputManager->printa_sf(thisAgent, "Use 'explain ?' to learn more about explain's sub-command and settings.\n");
}

void Explanation_Memory::list_watched_rules()
{
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 4);
    outputManager->printa(thisAgent, "Rules watched:\n");
    print_rules_watched(0);
}


void Explanation_Memory::list_rules_of_type(bool pChunks)
{
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 4);
    list_explained_rules(0, pChunks, true);
}

void Explanation_Memory::print_EBC_stats()
{
    outputManager->set_column_indent(0, 72);
    outputManager->printa_sf(thisAgent, "===========================================================================\n");
    outputManager->printa_sf(thisAgent, "                  Explanation-Based Chunking Statistics\n");
    outputManager->printa_sf(thisAgent, "===========================================================================\n");
    outputManager->printa_sf(thisAgent, "Rules learned                                          %-%u\n", stats.chunks_succeeded);
    outputManager->printa_sf(thisAgent, "Justifications learned                                 %-%u\n", stats.justifications_succeeded);
    outputManager->printa_sf(thisAgent, "\n---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "                               Work Performed\n");
    outputManager->printa_sf(thisAgent, "---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "Sub-states analyzed                                    %-%u\n", stats.chunks_attempted);
    outputManager->printa_sf(thisAgent, "Number of rules fired in substates analyzed            %-%u\n", thisAgent->explanationBasedChunker->get_instantiation_count());
    outputManager->printa_sf(thisAgent, "Number of rule firings analyzed during backtracing     %-%u\n", stats.instantations_backtraced);
    outputManager->printa_sf(thisAgent, "\nConditions merged                                    %- %u\n", stats.merged_conditions);
    outputManager->printa_sf(thisAgent, "Disjunction tests merged                               %-%u\n", stats.merged_disjunctions);
    outputManager->printa_sf(thisAgent, "Operational constraints                                %-%u\n", stats.operational_constraints);
    outputManager->printa_sf(thisAgent, "Non-operational constraints detected                   %-%u\n", stats.constraints_collected);
    outputManager->printa_sf(thisAgent, "Non-operational constraints enforced                   %-%u\n", stats.constraints_attached);
    outputManager->printa_sf(thisAgent, "\n---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "                     Problem-Solving Characteristics\n");
    outputManager->printa_sf(thisAgent, "---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "Rules repaired that had unconnected conditions or actions          %-%u\n", stats.chunks_repaired);
    outputManager->printa_sf(thisAgent, "Chunk used negated reasoning about sub-state                       %-%u\n", stats.tested_local_negation);
    outputManager->printa_sf(thisAgent, "Chunk tested knowledge retrieved from long-term memory             %-%u\n", stats.tested_ltm_recall);
    outputManager->printa_sf(thisAgent, "Justification used negated reasoning about sub-state               %-%u\n", stats.tested_local_negation_just);
    outputManager->printa_sf(thisAgent, "Justification tested knowledge retrieved from long-term memory     %-%u\n", stats.tested_ltm_recall_just);
    outputManager->printa_sf(thisAgent, "\n---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "                           Identity Analysis\n");
    outputManager->printa_sf(thisAgent, "---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "Identities created                                                 %-%u\n", stats.identities_created);
    outputManager->printa_sf(thisAgent, "Distinct identities in learned rules                               %-%u\n", stats.identities_participated);
    outputManager->printa_sf(thisAgent, "Identity propagations                                              %-%u\n", stats.identity_propagations);
    outputManager->printa_sf(thisAgent, "Identity propagations blocked                                      %-%u\n", stats.identity_propagations_blocked);
    outputManager->printa_sf(thisAgent, "Identities joined                                                  %-%u\n", stats.identities_joined);
    outputManager->printa_sf(thisAgent, "Identities literalized                                             %-%u\n", stats.identities_literalized);
    outputManager->printa_sf(thisAgent, "\n---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "                      Learning Skipped or Unsuccessful\n");
    outputManager->printa_sf(thisAgent, "---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "Ignored duplicate of existing rule                                 %-%u\n", stats.duplicates);
    outputManager->printa_sf(thisAgent, "Skipped because problem-solving tested ^quiescence true            %-%u\n", stats.tested_quiescence);
    outputManager->printa_sf(thisAgent, "Skipped because no super-state knowledge tested                    %-%u\n", stats.no_grounds);
    outputManager->printa_sf(thisAgent, "Skipped because MAX-CHUNKS exceeded in a decision cycle            %-%u\n", stats.max_chunks);
    outputManager->printa_sf(thisAgent, "Skipped because MAX-DUPES exceeded for rule this decision cycle    %-%u\n", stats.max_dupes);
}


void Explanation_Memory::print_chunk_stats(chunk_record* pChunkRecord, bool pPrintHeader) {

    assert(pChunkRecord);
    outputManager->set_column_indent(0, 72);
    if (pPrintHeader)
    {
    outputManager->printa_sf(thisAgent, "\nStatistics for learned rule %y (c %u):\n\n",   pChunkRecord->name, pChunkRecord->chunkID);
    }
    outputManager->printa_sf(thisAgent, "Number of conditions:           %-%u\n",          static_cast<uint64_t>(pChunkRecord->chunkInstantiation->conditions->size()));
    outputManager->printa_sf(thisAgent, "- Operational constraints:              %-%u\n", pChunkRecord->stats.operational_constraints);
    outputManager->printa_sf(thisAgent, "- Non-operational constraints detected: %-%u\n", pChunkRecord->stats.constraints_collected);
    outputManager->printa_sf(thisAgent, "- Non-operational constraints enforced: %-%u\n\n", pChunkRecord->stats.constraints_attached);
    outputManager->printa_sf(thisAgent, "Number of actions:              %-%u\n",          static_cast<uint64_t>(pChunkRecord->chunkInstantiation->actions->size()));
    outputManager->printa_sf(thisAgent, "Base instantiation:             %-i %u (%y)\n",    pChunkRecord->baseInstantiation->instantiationID, pChunkRecord->baseInstantiation->production_name);
    if (pChunkRecord->result_inst_records->size() > 0)
    {
        outputManager->printa_sf(thisAgent, "Number of child result instantiations:  %-%u\n",          static_cast<uint64_t>(pChunkRecord->result_inst_records->size()));
        outputManager->printa_sf(thisAgent, "Child result instantiations: " );
        for (auto it = pChunkRecord->result_inst_records->begin(); it != pChunkRecord->result_inst_records->end(); ++it)
        {
            outputManager->printa_sf(thisAgent, "%-i %u (%y)\n", (*it)->instantiationID, (*it)->production_name);
        }
    }
    outputManager->printa_sf(thisAgent, "\n---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "                            Work Performed\n");
    outputManager->printa_sf(thisAgent, "---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "Number of rule firings analyzed during backtracing:     %-%u\n", pChunkRecord->stats.instantations_backtraced);
    outputManager->printa_sf(thisAgent, "Duplicates chunks later created:                        %-%u\n", pChunkRecord->stats.duplicates);
    outputManager->printa_sf(thisAgent, "\nConditions merged:                                    %- %u\n", pChunkRecord->stats.merged_conditions);
    outputManager->printa_sf(thisAgent, "Disjunction tests merged:                               %-%u\n", pChunkRecord->stats.merged_disjunctions);
    outputManager->printa_sf(thisAgent, "\n---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "                           Identity Analysis\n");
    outputManager->printa_sf(thisAgent, "---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "Identities created in rule's sub-state:                             %-%u\n", pChunkRecord->stats.identities_created);
    outputManager->printa_sf(thisAgent, "Distinct identities in learned rules:                               %-%u\n", pChunkRecord->stats.identities_participated);
    outputManager->printa_sf(thisAgent, "Identities joined:                                                  %-%u\n", pChunkRecord->stats.identities_joined);
    outputManager->printa_sf(thisAgent, "Identities literalized:                                             %-%u\n", pChunkRecord->stats.identities_literalized);
    outputManager->printa_sf(thisAgent, "\n---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "                    Problem-Solving Characteristics\n");
    outputManager->printa_sf(thisAgent, "---------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "Used negated reasoning about sub-state:              %-%s\n", (pChunkRecord->stats.tested_local_negation ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "Tested ^quiescence true:                             %-%s\n", (pChunkRecord->stats.tested_quiescence ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "Tested knowledge retrieved from long-term memory:    %-%s\n", (pChunkRecord->stats.tested_ltm_recall ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "Added conditions to repair unconnected conditions or actions:    %-%s\n", (pChunkRecord->stats.repaired ? "Yes" : "No"));
}

void Explanation_Memory::list_explained_rules(short pNumToPrint, bool pChunks, bool pNewLine)
{
    bool            lHeaderPrinted = false;
    short           lNumPrinted = 0;
    ebc_rule_type   lRuleType;
    std::string     lRuleTypeString;

    if (pChunks)
    {
        lRuleType = ebc_chunk;
        lRuleTypeString = "chunks";
    } else {
        lRuleType = ebc_justification;
        lRuleTypeString = "justifications";
    }

    for (auto it = (*chunks).begin(); it != (*chunks).end(); ++it)
    {
        Symbol* d1 = it->first;
        chunk_record* d2 = it->second;

        if (d2->type != lRuleType) continue;
        if (!lHeaderPrinted)
        {
            outputManager->printa_sf (thisAgent, pChunks ? "Chunks available for explanation:%s" : "Justifications available for explanation:%s", pNewLine ? "\n" : " ");
            lHeaderPrinted = true;
        }
        outputManager->printa_sf(thisAgent, "%-%y (c %u)\n", it->first, it->second->chunkID);
        if (pNumToPrint && (++lNumPrinted == pNumToPrint)) break;
    }
    if (!lHeaderPrinted)
    {
        outputManager->printa_sf (thisAgent, "No %s have been recorded.", lRuleTypeString.c_str());
    }
    else
    {
        if (pNumToPrint && (lNumPrinted < chunks->size()))
        {
            outputManager->printa_sf(thisAgent, "\n* Note:  Only listed the first %d %s recorded.  Type 'explain list-%s' to see the other %d %s.\n", static_cast<int64_t>(pNumToPrint), lRuleTypeString.c_str(), lRuleTypeString.c_str(), static_cast<int64_t>(chunks->size() - lNumPrinted), lRuleTypeString.c_str());
        }
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
        outputManager->printa_sf(thisAgent, "\n* Note:  Only printed the first %d rules.  Type 'explain watch' to see the other %d rules.\n", static_cast<int64_t>(pNumToPrint), static_cast<int64_t>(lNumLeftToPrint));
    }
}

void Explanation_Memory::print_identity_set_explanation()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "\nIdentity analysis of problem-solving behind rule %y (c %u):\n\n",   current_discussed_chunk->name, current_discussed_chunk->chunkID);

    current_discussed_chunk->identity_analysis.print_identities_in_chunk();
    current_discussed_chunk->identity_analysis.print_mappings();
}

void Explanation_Memory::print_constraints_enforced()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "\nConstraint analysis of problem-solving behind rule %y (c %u):\n\n",   current_discussed_chunk->name, current_discussed_chunk->chunkID);
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

    outputManager->printa_sf(thisAgent, "This chunk summarizes the problem-solving involved in the following %d rule firings:\n\n", static_cast<int64_t>(current_discussed_chunk->backtraced_inst_records->size()));

    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end();++it)
    {
        outputManager->printa_sf(thisAgent, "   i %u (%y)\n", (*it)->instantiationID, (*it)->production_name);
    }
    outputManager->printa(thisAgent, "\n");
}
