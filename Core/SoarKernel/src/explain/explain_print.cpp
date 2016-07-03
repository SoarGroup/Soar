#include "ebc.h"

#include "explain.h"

#include "action_record.h"
#include "condition_record.h"
#include "identity_record.h"
#include "instantiation_record.h"
#include "production_record.h"

#include "agent.h"
#include "condition.h"
#include "instantiation.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "test.h"
#include "working_memory.h"
#include "output_manager.h"
#include "dprint.h"

void Explanation_Logger::switch_to_explanation_trace(bool pEnableExplanationTrace)
{
    print_explanation_trace = pEnableExplanationTrace;
    if (!last_printed_id)
    {
        print_chunk_explanation();
    } else {
        print_instantiation_explanation_for_id(last_printed_id);
    }
}

void Explanation_Logger::print_formation_explanation()
{
    assert(current_discussed_chunk);

    outputManager->printa_sf(thisAgent, "The formation of '%y' (c%u):\n\n", current_discussed_chunk->name, current_discussed_chunk->chunkID);

    outputManager->printa_sf(thisAgent, "   (1) At time %u, rule '%y' matched (i %u) \n"
        "       and created results in a superstate.\n\n",
        current_discussed_chunk->time_formed,
        current_discussed_chunk->baseInstantiation->production_name,
        current_discussed_chunk->baseInstantiation->instantiationID);

    outputManager->printa(thisAgent, "   (2) Conditions of base instantiations and any relevant operator selection knowledge\n"
        "       are backtraced through to determine what superstate knowledge was tested and\n"
        "       should appear in the chunk.\n\n");
    outputManager->printa(thisAgent,    "   (3) EBC analyzes the how variables are used in the explanation trace to determine \n"
        "       sets of elements that share the same semantics.  (explain -i)\n\n");
    outputManager->printa(thisAgent,    "   (4) EBC constraint analysis is performed to determine all constraints on the \n"
        "       values of variables in (3) that were required by problem-solving (explain -c).\n\n");
    outputManager->printa(thisAgent,    "   (5) The mappings determined by the identity set analysis are used to variablize\n"
        "       conditions collected in (2) and constraints collected in (4).  (explain -i)\n\n");
    outputManager->printa(thisAgent,    "   (6) Generalized constraints from (5) are added and final chunk conditions are\n"
        "       polished by pruning unnecessary tests and merging appropriate conditions.\n"
        "       Statistics on polishing are available.  (explain -c) (explain -s)\n\n");
    outputManager->printa_sf(thisAgent, "------------------------------------------------------------------------------------\n\n");

    outputManager->printa_sf(thisAgent, "The following %d instantiations fired to produce results in Step (2)\n\n",
        current_discussed_chunk->result_inst_records->size() + 1);

    outputManager->printa_sf(thisAgent, "Initial base instantiation i %u that fired when %y matched at time %u:\n\n",
        current_discussed_chunk->baseInstantiation->instantiationID,
        current_discussed_chunk->baseInstantiation->production_name,
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
    print_involved_instantiations();
    print_footer(true);
}

void Explanation_Logger::print_footer(bool pPrintDiscussedChunkCommands)
{
    outputManager->printa(thisAgent, "---------------------------------------------------------------------------------------------------------------------\n");
    outputManager->set_column_indent(0, 0);
    outputManager->set_column_indent(1, 16);
    outputManager->set_column_indent(2, 70);
    outputManager->set_column_indent(3, 83);
    if (print_explanation_trace)
    {
        outputManager->printa_sf(thisAgent, "- explain -f %-Explain initial formation of chunk %-explain -w %-Switch to working memory trace    -\n");
    } else {
        outputManager->printa_sf(thisAgent, "- explain -f %-Explain initial formation of chunk %-explain -e %-Switch to explanation trace       -\n");
    }
    outputManager->printa_sf(thisAgent, "- explain -c %-Explain constraints required by problem-solving %-explain -i %-Explain element identity analysis -\n");
    outputManager->printa_sf(thisAgent, "- explain -s %-Print chunk statistics %-explain -s %-Print EBC statistics              -\n");
    outputManager->printa(thisAgent, "---------------------------------------------------------------------------------------------------------------------\n");

}

bool Explanation_Logger::is_condition_related(condition_record* pCondRecord)
{
    //    if ((pCondRecord->condition_tests.id->eq_test->identity == current_explained_ids.id) ||
    //        (pCondRecord->condition_tests.attr->eq_test->identity == current_explained_ids.id) ||
    //        (pCondRecord->condition_tests.value->eq_test->identity == current_explained_ids.id))
    //    {
    //    }
    return false;
}

void Explanation_Logger::print_path_to_base(const inst_record_list* pPathToBase, bool pPrintFinal, const char* pFailedStr, const char* pHeaderStr)
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


void Explanation_Logger::print_action_list(action_record_list* pActionRecords, production* pOriginalRule, action* pRhs, production_record* pExcisedRule)
{
    if (pActionRecords->empty())
    {
        outputManager->printa(thisAgent, "No actions on right-hand-side\n");
    }
    else
    {
        action_record* lAction;
        condition* top = NULL, *bottom = NULL;
        action* rhs;
        int lActionCount = 0;
        thisAgent->outputManager->set_print_indents("");
        thisAgent->outputManager->set_print_test_format(true, false);
        if (print_explanation_trace)
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
                        outputManager->printa_sf(thisAgent, "No rule for this instantiation found in RETE\n");
                        return;
                    }
                } else {
                    p_node_to_conditions_and_rhs(thisAgent, pOriginalRule->p_node, NIL, NIL, &top, &bottom, &rhs);
                    pRhs = rhs;
                }
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
                lAction->print_action(rhs, lActionCount);
//                thisAgent->outputManager->set_print_test_format(true, false);
//                outputManager->printa_sf(thisAgent, "%d:%-%a", lActionCount,  rhs);
//                thisAgent->outputManager->set_print_test_format(false, true);
//                rhs_value rt = rhs->value;
//
//                if (lAction->variablized_action)
//                {
//                    outputManager->printa_sf(thisAgent, "%-%a\n", lAction->variablized_action);
//                } else {
//                    outputManager->printa_sf(thisAgent, "%-%p\n", lAction->instantiated_pref);
//                }
//                //                if (print_explanation_trace)
//                //                {
//                //                    thisAgent->outputManager->set_print_test_format(false, true);
//                //                    outputManager->printa_sf(thisAgent, "%-%p\n", lAction->instantiated_pref);
//                //                    outputManager->printa_sf(thisAgent, "%d:%-%a", lActionCount,  rhs);
//                //                } else {
//                //                    outputManager->printa(thisAgent, "\n");
//                //                }
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
void action_record::print_action(action* pAction, int lActionCount)
{
    std::string tempString;
    Output_Manager* outputManager = thisAgent->outputManager;

    if (pAction->type == FUNCALL_ACTION)
    {
        tempString = "";
        outputManager->rhs_value_to_string(thisAgent, pAction->value, tempString, NULL, NULL, true);
        outputManager->printa_sf(thisAgent, "%d:%-%s%-%s", lActionCount,  tempString.c_str(), tempString.c_str());
    } else {
        outputManager->printa_sf(thisAgent, "%d:%-(", lActionCount);
        print_rhs_value(pAction->id, (variablized_action ? variablized_action->id : NULL), instantiated_pref->o_ids.id, true);
        outputManager->printa(thisAgent, " ^");
        print_rhs_value(pAction->attr, (variablized_action ? variablized_action->attr : NULL), instantiated_pref->o_ids.attr, true);
        outputManager->printa(thisAgent, " ");
        print_rhs_value(pAction->value, (variablized_action ? variablized_action->value : NULL), instantiated_pref->o_ids.value, true);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_value(pAction->referent, (variablized_action ? variablized_action->referent : NULL), instantiated_pref->o_ids.referent, true);
        }
        outputManager->printa_sf(thisAgent, ")%-(");
        print_rhs_value(pAction->id, (variablized_action ? variablized_action->id : NULL), instantiated_pref->o_ids.id, false);
        outputManager->printa(thisAgent, " ^");
        print_rhs_value(pAction->attr, (variablized_action ? variablized_action->attr : NULL), instantiated_pref->o_ids.attr, false);
        outputManager->printa(thisAgent, " ");
        print_rhs_value(pAction->value, (variablized_action ? variablized_action->value : NULL), instantiated_pref->o_ids.value, false);
        outputManager->printa_sf(thisAgent, " %c", preference_to_char(pAction->preference_type));
        if (pAction->referent)
        {
            print_rhs_value(pAction->referent, (variablized_action ? variablized_action->referent : NULL), instantiated_pref->o_ids.referent, false);
        }
        outputManager->printa(thisAgent, ")\n");
    }
    tempString.clear();
}
void Explanation_Logger::print_instantiation_explanation(instantiation_record* pInstRecord, bool printFooter)
{
    if (print_explanation_trace)
    {
        pInstRecord->print_for_explanation_trace(printFooter);
    } else {
        pInstRecord->print_for_wme_trace(printFooter);
    }
}

void Explanation_Logger::print_chunk_explanation()
{
    assert(current_discussed_chunk);

    if (print_explanation_trace)
    {
        current_discussed_chunk->print_for_explanation_trace();
    } else {
        current_discussed_chunk->print_for_wme_trace();
    }
}

void Explanation_Logger::print_explain_summary()
{
    outputManager->set_column_indent(0, 4);
    outputManager->set_column_indent(1, 50);
    outputManager->set_column_indent(2, 49);
    outputManager->printa_sf(thisAgent, "%fExplainer Settings:\n");
    outputManager->printa_sf(thisAgent, "%-Watch all chunk formations        %-%s\n", (enabled ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "%-Number of specific rules watched  %-%d\n", num_rules_watched);

    /* Print specific watched rules and time interval when watch all disabled */
    if (!enabled)
    {
        /* Print last 10 rules watched*/
        outputManager->printa_sf(thisAgent, "%-Rules watched:");
        print_rules_watched(10);
        /* When we add time interval option, print it here */
    }

    outputManager->printa(thisAgent, "\n");
    outputManager->printa_sf(thisAgent, "%-Chunks available for discussion:");
    print_chunk_list(10);

    outputManager->printa(thisAgent, "\n");

    /* Print current chunk and last 10 chunks formed */
    outputManager->printa_sf(thisAgent, "%-Current chunk being discussed: %-%s",
        (current_discussed_chunk ? current_discussed_chunk->name->sc->name : "None" ));
    if (current_discussed_chunk)
    {
        outputManager->printa_sf(thisAgent, "(c%u)\n\n", current_discussed_chunk->chunkID);
    } else {
        outputManager->printa(thisAgent, "\n\n");
    }
    outputManager->printa(thisAgent, "Type 'explain [chunk-name]' or 'explain c [chunk id]' to discuss the formation of that chunk.\n");
}

void Explanation_Logger::print_all_watched_rules()
{
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 0);
    outputManager->set_column_indent(1, 4);
    outputManager->printa(thisAgent, "Rules watched:\n");
    print_rules_watched(0);
}


void Explanation_Logger::print_all_chunks()
{
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 0);
    outputManager->set_column_indent(1, 4);
    outputManager->printa(thisAgent, "Chunks available for discussion:\n");
    print_chunk_list(0);
}

void Explanation_Logger::print_explainer_stats()
{
    outputManager->set_column_indent(0, 50);

    outputManager->printa_sf(thisAgent, "%f-------------------------\n");
    outputManager->printa_sf(thisAgent, "EBC Executions Statistics\n");
    outputManager->printa_sf(thisAgent, "-------------------------\n");
    outputManager->printa_sf(thisAgent, "Chunks attempted                           %-%u\n", stats.chunks_attempted);
    outputManager->printa_sf(thisAgent, "Chunks successfully built                  %-%u\n", stats.chunks_succeeded);
    outputManager->printa_sf(thisAgent, "Chunk failures reverted to justifications  %-%u\n", stats.chunks_reverted);

    outputManager->printa_sf(thisAgent, "Justifications attempted                   %-%u\n", stats.justifications_attempted);
    outputManager->printa_sf(thisAgent, "Justifications successfully built          %-%u\n", stats.justifications_succeeded);

    outputManager->printa_sf(thisAgent, "\nInstantiations built                     %- %u\n", thisAgent->ebChunker->get_instantiation_count());
    outputManager->printa_sf(thisAgent, "Instantiations backtraced through          %-%u\n", stats.instantations_backtraced);
    outputManager->printa_sf(thisAgent, "Instantiations backtraced through twice    %-%u\n", stats.seen_instantations_backtraced);

    outputManager->printa_sf(thisAgent, "\nConditions merged                        %- %u\n", stats.merged_conditions);
    outputManager->printa_sf(thisAgent, "Constraints collected                      %-%u\n", stats.constraints_collected);
    outputManager->printa_sf(thisAgent, "Constraints attached                       %-%u\n", stats.constraints_attached);
    outputManager->printa_sf(thisAgent, "Grounding conditions generated             %-%u\n", stats.grounding_conditions_added);

    outputManager->printa_sf(thisAgent, "\n----------------------\n");
    outputManager->printa_sf(thisAgent, "EBC Failure Statistics\n");
    outputManager->printa_sf(thisAgent, "----------------------\n");

    outputManager->printa_sf(thisAgent, "Duplicate chunk already existed            %-%u\n", stats.duplicates);
    outputManager->printa_sf(thisAgent, "Chunk tested local negation                %-%u\n", stats.tested_local_negation);
    outputManager->printa_sf(thisAgent, "Could not re-order chunk                   %-%u\n", stats.unorderable);
    outputManager->printa_sf(thisAgent, "Chunk had no grounds                       %-%u\n", stats.no_grounds);
    outputManager->printa_sf(thisAgent, "Already reached max-chunks                 %-%u\n", stats.max_chunks);
    outputManager->printa_sf(thisAgent, "Chunk formed did not match WM              %-%u\n", stats.chunk_did_not_match);
    outputManager->printa_sf(thisAgent, "Justification formed did not match WM      %-%u\n", stats.justification_did_not_match);

    outputManager->printa_sf(thisAgent, "\n------------------------\n");
    outputManager->printa_sf(thisAgent, "EBC Explainer Statistics\n");
    outputManager->printa_sf(thisAgent, "------------------------\n");
    outputManager->printa_sf(thisAgent, "Chunks records                             %-%d\n", chunks->size());
    outputManager->printa_sf(thisAgent, "Actions records                            %-%d\n", all_actions->size());
    outputManager->printa_sf(thisAgent, "Condition records                          %-%d\n", all_conditions->size());
    outputManager->printa_sf(thisAgent, "Instantiation records                      %-%d\n", instantiations->size());
}

void Explanation_Logger::print_chunk_stats() {

    assert(current_discussed_chunk);
    outputManager->set_column_indent(0, 45);
    outputManager->printa_sf(thisAgent, "%fStatistics for '%y' (c%u):\n\n",                         current_discussed_chunk->name, current_discussed_chunk->chunkID);
    outputManager->printa_sf(thisAgent, "Number of conditions           %-%u\n",          current_discussed_chunk->conditions->size());
    outputManager->printa_sf(thisAgent, "Number of actions              %-%u\n",          current_discussed_chunk->actions->size());
    outputManager->printa_sf(thisAgent, "Base instantiation             %-i %u (%y)\n",    current_discussed_chunk->baseInstantiation->instantiationID, current_discussed_chunk->baseInstantiation->production_name);
    outputManager->printa_sf(thisAgent, "Extra result instantiations " );
    if (current_discussed_chunk->result_inst_records->size() > 0)
    {
        for (auto it = current_discussed_chunk->result_inst_records->begin(); it != current_discussed_chunk->result_inst_records->end(); ++it)
        {
            outputManager->printa_sf(thisAgent, "%-i %u (%y)\n", (*it)->instantiationID, (*it)->production_name);
        }
    }

    outputManager->printa_sf(thisAgent, "\nInstantiations backtraced through        %- %u\n", current_discussed_chunk->stats.instantations_backtraced);
    outputManager->printa_sf(thisAgent, "Instantiations backtraced multiple times   %-%u\n", current_discussed_chunk->stats.seen_instantations_backtraced);
    outputManager->printa_sf(thisAgent, "Conditions merged                          %-%u\n", current_discussed_chunk->stats.merged_conditions);
    outputManager->printa_sf(thisAgent, "Constraints collected                      %-%u\n", current_discussed_chunk->stats.constraints_collected);
    outputManager->printa_sf(thisAgent, "Constraints attached                       %-%u\n", current_discussed_chunk->stats.constraints_attached);
    outputManager->printa_sf(thisAgent, "Grounding conditions added                 %-%u\n", current_discussed_chunk->stats.num_grounding_conditions_added);

    outputManager->printa_sf(thisAgent, "\nDuplicates chunks later created          %- %u\n", current_discussed_chunk->stats.duplicates);
    outputManager->printa_sf(thisAgent, "Tested negation in local substate          %-%s\n", (current_discussed_chunk->stats.tested_local_negation ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "Failed chunk reverted to justification     %-%s\n", (current_discussed_chunk->stats.reverted ? "Yes" : "No"));
}

void Explanation_Logger::print_chunk_list(short pNumToPrint)
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
        outputManager->printa_sf(thisAgent, "\n* Note:  Only printed the first %d chunks.  Type 'explain --list' to see the other %d chunks.\n", pNumToPrint, ( (*chunks).size() - pNumToPrint));
    }
}

bool Explanation_Logger::print_watched_rules_of_type(agent* thisAgent, unsigned int productionType, short &pNumToPrint)
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

void Explanation_Logger::print_rules_watched(short pNumToPrint)
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
        outputManager->printa_sf(thisAgent, "\n* Note:  Only printed the first %d rules.  Type 'explain --watch' to see the other %d rules.\n", pNumToPrint, lNumLeftToPrint);
    }
}

void Explanation_Logger::print_condition_explanation(uint64_t pCondID)
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "Printing explanation of condition %u in relation to chunk %y.\n", pCondID, current_discussed_chunk->name);
}

void Explanation_Logger::print_identity_set_explanation()
{
    assert(current_discussed_chunk);
    current_discussed_chunk->identity_analysis->print_identity_mappings();
}

void Explanation_Logger::print_constraints_enforced()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "Constraints enforced during formation of chunk %y.\n\nNot yet implemented.\n", current_discussed_chunk->name);
}


void Explanation_Logger::print_involved_instantiations()
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

    outputManager->printa_sf(thisAgent, "All %d rule firings involved in problem solving:\n\n", current_discussed_chunk->backtraced_inst_records->size());

    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end();++it)
    {
        outputManager->printa_sf(thisAgent, "   i %u (%y)\n", (*it)->instantiationID, (*it)->production_name);
    }
    outputManager->printa(thisAgent, "\n");
}
