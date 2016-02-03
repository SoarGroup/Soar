#include "debug.h"
#include "explain.h"
#include "agent.h"
#include "condition.h"
#include "instantiation.h"
#include "preference.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "test.h"
#include "working_memory.h"
#include "output_manager.h"

void Explanation_Logger::print_dependency_analysis()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "The formation of '%y' (c%u):\n\n", current_discussed_chunk->name, current_discussed_chunk->chunkID);

    outputManager->printa_sf(thisAgent, "   (1) At time 0, rule '%y' matched, fired (i%u) and created new working memory\n"
                                        "       elements linked to a superstate.\n\n",
        current_discussed_chunk->baseInstantiation->production_name, current_discussed_chunk->baseInstantiation->instantiationID);

    outputManager->printa_sf(thisAgent, "   (2) Conditions of both i%u and any relevant operator selection knowledge are\n"
                                        "       backtraced through to determine what superstate knowledge was tested and\n"
                                        "       should appear in the chunk. (marked with *)\n\n",  current_discussed_chunk->baseInstantiation->instantiationID);
    outputManager->printa(thisAgent,    "   (3) EBC identity analysis is performed to determine the relationships between\n"
                                        "       variables across rules.  (explain -i)\n\n");
    outputManager->printa(thisAgent,    "   (4) EBC constraint analysis is performed to determine all constraints on the \n"
                                        "       values of variables in (3) that were required by problem-solving (explain -c).\n\n");
    outputManager->printa(thisAgent,    "   (5) The mappings determined by the identity set analysis is used to\n"
                                        "       variablize the conditions collected in (2) and the constraints collected\n"
                                        "       in (4).\n\n");
    outputManager->printa(thisAgent,    "   (6) Generalized constraints from (5) are added and final chunk conditions are\n"
                                        "       polished by pruning unnecessary tests and merging appropriate conditions.\n\n");

    outputManager->set_column_indent(0, 70);
    outputManager->set_column_indent(1, 80);
    outputManager->set_column_indent(2, 90);
    outputManager->printa_sf(thisAgent, "Conditions from Step 2 %-Source%-Chunk %-Instantiation that created matched WME\n\n",
        current_discussed_chunk->baseInstantiation->instantiationID, current_discussed_chunk->baseInstantiation->production_name);

    print_condition_list(ebc_actual_trace, false, current_discussed_chunk->baseInstantiation->conditions, current_discussed_chunk->baseInstantiation->original_production, current_discussed_chunk->baseInstantiation->match_level);
    outputManager->printa(thisAgent, "   -->\n");
    print_action_list(ebc_actual_trace, current_discussed_chunk->baseInstantiation->actions, current_discussed_chunk->baseInstantiation->original_production);

    outputManager->printa(thisAgent, "\n");
    outputManager->printa_sf(thisAgent, "Explanation Trace: %-Using variable identity IDs\n\n");

    print_condition_list(ebc_explanation_trace, false, current_discussed_chunk->baseInstantiation->conditions, current_discussed_chunk->baseInstantiation->original_production, current_discussed_chunk->baseInstantiation->match_level);
    outputManager->printa(thisAgent, "   -->\n");
    print_action_list(ebc_explanation_trace, current_discussed_chunk->baseInstantiation->actions, current_discussed_chunk->baseInstantiation->original_production);

    outputManager->printa_sf(thisAgent, "\nIdentity to identity set mappings:\n\n");
    print_identity_set_explanation();

}

void Explanation_Logger::print_instantiation(EBCTraceType pType, instantiation_record* pInstRecord)
{
    print_condition_list(pType, false, pInstRecord->conditions, pInstRecord->original_production, pInstRecord->match_level);
    outputManager->printa(thisAgent, "   -->\n");
    print_action_list(pType, pInstRecord->actions, pInstRecord->original_production);
}


void Explanation_Logger::print_instantiation_explanation(instantiation_record* pInstRecord)
{
    outputManager->set_column_indent(0, 70);
    outputManager->set_column_indent(1, 80);
    outputManager->set_column_indent(2, 90);
    outputManager->printa_sf(thisAgent, "Instantiation # %u (match of rule %y)%-Source%-Chunk%-Instantiation that created WME\n\n",
        pInstRecord->instantiationID, pInstRecord->production_name);
    print_instantiation(ebc_actual_trace, pInstRecord);
    outputManager->printa(thisAgent, "\n");
    outputManager->set_column_indent(0,70);
    outputManager->printa_sf(thisAgent, "Explanation Trace: %-Using variable identity IDs\n\n");
    print_instantiation(ebc_explanation_trace, pInstRecord);
    outputManager->printa_sf(thisAgent, "\nIdentity to identity set mappings:\n\n");
//    print_identity_set_explanation();
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
void Explanation_Logger::print_condition_list(EBCTraceType pType, bool pForChunk, condition_record_list* pCondRecords, production* pOriginalRule, goal_stack_level pMatch_level)
{
    if (pCondRecords->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
    }
    else
    {
        condition_record* lCond;
        bool lInNegativeConditions = false;
        int lConditionCount = 0;
        condition* top, *bottom, *currentNegativeCond, *current_cond, *print_cond;
        action* rhs;
        test id_test_without_goal_test = NULL, id_test_without_goal_test2 = NULL;
        bool removed_goal_test, removed_impasse_test;

        outputManager->set_column_indent(0, 7);
        outputManager->set_column_indent(1, 70);
        outputManager->set_column_indent(2, 80);
        outputManager->set_column_indent(3, 90);
        thisAgent->outputManager->set_print_test_format(true, false);

        /* If we're printing the explanation trace, we reconstruct the conditions.  We need to do this
         * because we don't want to cache all explanation trace's symbols every time we create an instantiation.
         * We used to and it's very inefficient.  We also can't use the ebChunker's lookup table because that
         * is only for debugging and does not get built for releases. */

        if (pType == ebc_explanation_trace)
        {
            if (!pOriginalRule || !pOriginalRule->p_node)
            {
                outputManager->printa_sf(thisAgent, "Original rule conditions no longer in RETE\n");
                return;
            }
            p_node_to_conditions_and_rhs(thisAgent, pOriginalRule->p_node, NIL, NIL, &top, &bottom, &rhs);
            current_cond = top;
            if (current_cond->type == CONJUNCTIVE_NEGATION_CONDITION)
            {
                currentNegativeCond = current_cond->data.ncc.top;
            } else {
                currentNegativeCond = NULL;
            }
            thisAgent->outputManager->set_print_test_format(false, true);
        }
        for (condition_record_list::iterator it = pCondRecords->begin(); it != pCondRecords->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (lInNegativeConditions)
            {
                if (lCond->type != CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "}\n");
                    lInNegativeConditions = false;
                }
            } else {
                if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "-{\n");
                    lInNegativeConditions = true;
                }
            }
            outputManager->printa_sf(thisAgent, "%d:%-", lConditionCount);

            if (pType == ebc_actual_trace)
            {
                id_test_without_goal_test = copy_test_removing_goal_impasse_tests(thisAgent, lCond->condition_tests.id, &removed_goal_test, &removed_impasse_test);

                outputManager->printa_sf(thisAgent, "(%t%s^%t %t)%s",
                    id_test_without_goal_test, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                    lCond->condition_tests.attr, lCond->condition_tests.value, is_condition_related(lCond) ? "*" : "");
                if (pForChunk)
                {
                    outputManager->printa_sf(thisAgent, "%-%u\n", lCond->conditionID);
                } else {
                    outputManager->printa_sf(thisAgent, "%-Rule%-%s", ((pMatch_level > 0) && (lCond->wme_level_at_firing < pMatch_level) ? "Yes" : "Local"));
                    if (lCond->parent_instantiation)
                    {
                        outputManager->printa_sf(thisAgent, "%-i%u (%s)\n",
                            (lCond->parent_instantiation ? lCond->parent_instantiation->instantiationID : 0),
                            (lCond->parent_instantiation ? lCond->parent_instantiation->production_name->sc->name  : "Soar Architecture"));
                    } else
                    {
                        outputManager->printa_sf(thisAgent, "%-Soar Architecture\n");
                    }
                }
            } else if (pType == ebc_explanation_trace)
            {

                /* Get the next condition from the explanation trace.  This is tricky because NCCs are condition lists within condition lists */
                if (currentNegativeCond)
                {
                    print_cond = currentNegativeCond;
                } else {
                    print_cond = current_cond;
                }
                id_test_without_goal_test = copy_test_removing_goal_impasse_tests(thisAgent, print_cond->data.tests.id_test, &removed_goal_test, &removed_impasse_test);
                id_test_without_goal_test2 = copy_test_removing_goal_impasse_tests(thisAgent, lCond->condition_tests.id, &removed_goal_test, &removed_impasse_test);
                outputManager->printa_sf(thisAgent, "(%o%s^%o %o)%s    %-(%g%s^%g %g)\n",
                    id_test_without_goal_test, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                    print_cond->data.tests.attr_test, print_cond->data.tests.value_test,
                    is_condition_related(lCond) ? "*" : "",
                    id_test_without_goal_test2, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                    lCond->condition_tests.attr, lCond->condition_tests.value);
                if (currentNegativeCond)
                {
                    currentNegativeCond = currentNegativeCond->next;
                }
                if (!currentNegativeCond)
                {
                    current_cond = current_cond->next;
                }
                if (current_cond && (current_cond->type == CONJUNCTIVE_NEGATION_CONDITION) && !currentNegativeCond)
                {
                    currentNegativeCond = current_cond->data.ncc.top;
                }

            } else if (pType == ebc_match_trace)
            {
                if (lCond->matched_wme)
                {
                    outputManager->printa_sf(thisAgent, "(%y ^%y %y)%s",
                        lCond->matched_wme->id, lCond->matched_wme->attr, lCond->matched_wme->value, is_condition_related(lCond) ? "*" : "");
                } else {
                    outputManager->printa_sf(thisAgent, "(Negative condition.  No matched WME.)%s", is_condition_related(lCond) ? "*" : "");
                }
                if (pForChunk && (lCond->matched_wme != NULL))
                {
                    if (lCond->parent_instantiation)
                    {
                        outputManager->printa_sf(thisAgent, "%-i%u (%s)\n",
                            (lCond->parent_instantiation ? lCond->parent_instantiation->instantiationID : 0),
                            (lCond->parent_instantiation ? lCond->parent_instantiation->production_name->sc->name  : "Soar Architecture"));
                    } else
                    {
                        outputManager->printa_sf(thisAgent, "%-Soar Architecture\n");
                    }

                } else {
                    outputManager->printa(thisAgent, "\n");
                }
            }
        }
        if (lInNegativeConditions)
        {
            outputManager->printa(thisAgent, "}\n");
        }
        if (pType == ebc_explanation_trace)
        {
            deallocate_condition_list(thisAgent, top);
            deallocate_action_list(thisAgent, rhs);
        }
    }
}

void Explanation_Logger::print_action_list(EBCTraceType pType, action_record_list* pActionRecords, production* pOriginalRule, bool pPrintIdentity)
{
    if (pActionRecords->empty())
    {
        outputManager->printa(thisAgent, "No actions on right-hand-side\n");
    }
    else
    {
        action_record* lAction;
        condition* top, *bottom;
        action* rhs;
        int lActionCount = 0;
        outputManager->set_column_indent(0, 7);
        outputManager->set_column_indent(1, 70);
        outputManager->set_column_indent(2, 80);
        thisAgent->outputManager->set_print_indents("");
        thisAgent->outputManager->set_print_test_format(true, false);
        if (pType == ebc_explanation_trace)
        {
            if (!pOriginalRule || !pOriginalRule->p_node)
            {
                outputManager->printa_sf(thisAgent, "Original rule actions no longer in RETE\n");
                return;
            }
            p_node_to_conditions_and_rhs(thisAgent, pOriginalRule->p_node, NIL, NIL, &top, &bottom, &rhs);
        }
        for (action_record_list::iterator it = pActionRecords->begin(); it != pActionRecords->end(); it++)
        {
            lAction = (*it);
            ++lActionCount;
            if (pType == ebc_actual_trace)
            {
                outputManager->printa_sf(thisAgent, "%d:%-%p\n", lActionCount, lAction->instantiated_pref);
            } else if (pType == ebc_explanation_trace)
            {
                thisAgent->outputManager->set_print_test_format(true, false);
                outputManager->printa_sf(thisAgent, "%d:%-%a", lActionCount,  rhs);
                if (pPrintIdentity)
                {
                    thisAgent->outputManager->set_print_test_format(false, true);
                    outputManager->printa_sf(thisAgent, "%-%p\n", lAction->instantiated_pref);
                } else {
                    outputManager->printa(thisAgent, "\n");
                }
                rhs = rhs->next;
            } else if (pType == ebc_match_trace)
            {
                outputManager->printa_sf(thisAgent, "%d:%-%p\n", lActionCount, lAction->instantiated_pref);
            }
        }
        if (pType == ebc_explanation_trace)
        {
            deallocate_condition_list(thisAgent, top);
            deallocate_action_list(thisAgent, rhs);
        }
        thisAgent->outputManager->clear_print_test_format();
    }
}

void Explanation_Logger::print_chunk(EBCTraceType pType, chunk_record* pChunkRecord)
{
    if (pType == ebc_actual_trace)
    {
        outputManager->set_column_indent(0, 70);
        outputManager->set_column_indent(1, 80);
        outputManager->printa_sf(thisAgent, "sp {%y %-Condition ID\n\n", current_discussed_chunk->name);
    } else if (pType == ebc_explanation_trace)
    {
        outputManager->set_column_indent(0, 70);
        outputManager->printa_sf(thisAgent, "Explanation Trace:     %-Using variable identity set IDs\n\n");
    } else if (pType == ebc_match_trace)
    {
        outputManager->printa_sf(thisAgent, "Working Memory trace: %-Instantiation that created matched WME\n\n");
    }
    print_condition_list(pType, true, pChunkRecord->conditions, pChunkRecord->original_production);
    outputManager->printa(thisAgent, "      -->\n");

    /* For chunks, actual rhs is same as explanation trace without identity information on the rhs*/
    if (pType == ebc_actual_trace) {
        print_action_list(ebc_explanation_trace, pChunkRecord->actions, pChunkRecord->original_production, false);
        outputManager->printa(thisAgent, "}\n");
    } else {
        print_action_list(pType, pChunkRecord->actions, pChunkRecord->original_production);
    }
}

void Explanation_Logger::print_chunk_explanation()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "Now explaining chunk '%y' (c%u)\n\n", current_discussed_chunk->name, current_discussed_chunk->chunkID);

    print_chunk(ebc_actual_trace, current_discussed_chunk);
    outputManager->printa(thisAgent, "\n");
    print_chunk(ebc_explanation_trace, current_discussed_chunk);
    outputManager->printa(thisAgent, "\n");
    print_chunk(ebc_match_trace, current_discussed_chunk);

//    outputManager->printa_sf(thisAgent, "\nIdentity to identity set mappings:\n\n");
//    print_identity_set_explanation();
    outputManager->set_column_indent(0, 70);
    outputManager->printa(thisAgent, "\nThe following commands now apply to this chunk:\n\n");
    outputManager->printa_sf(thisAgent, "   explain [-d | --dependency-analysis]%-Explain backtrace of problem-solving\n");
    outputManager->printa_sf(thisAgent, "   explain [-c | --constraints]%-Explain constraints required by problem-solving\n");
    outputManager->printa_sf(thisAgent, "   explain [-s | --stats]%-Print chunking statistics for %y\n", current_discussed_chunk->name);
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
    outputManager->printa_sf(thisAgent, "%f-------------------------\n");
    outputManager->printa_sf(thisAgent, "EBC Executions Statistics\n");
    outputManager->printa_sf(thisAgent, "-------------------------\n");
    outputManager->printa_sf(thisAgent, "Chunks                                     %u\n", thisAgent->ebChunker->get_chunk_count());
    outputManager->printa_sf(thisAgent, "Chunks attempted                           %u\n", stats.chunks_attempted);
    outputManager->printa_sf(thisAgent, "Chunks successfully built                  %u\n", stats.succeeded);

    outputManager->printa_sf(thisAgent, "\nJustifications                             %u\n", thisAgent->ebChunker->get_justification_count());
    outputManager->printa_sf(thisAgent, "Justifications attempted                   %u\n", stats.justifications_attempted);
    outputManager->printa_sf(thisAgent, "Justifications successfully built          %u\n", stats.justifications);

    outputManager->printa_sf(thisAgent, "\nInstantiations built                       %u\n", thisAgent->ebChunker->get_instantiation_count());
    outputManager->printa_sf(thisAgent, "Instantiations backtraced through          %u\n", stats.instantations_backtraced);
    outputManager->printa_sf(thisAgent, "Instantiations backtraced through twice    %u\n", stats.seen_instantations_backtraced);

    outputManager->printa_sf(thisAgent, "\nConditions merged                          %u\n", stats.merged_conditions);
    outputManager->printa_sf(thisAgent, "Constraints collected                      %u\n", stats.constraints_collected);
    outputManager->printa_sf(thisAgent, "Constraints attached                       %u\n", stats.constraints_attached);

    outputManager->printa_sf(thisAgent, "----------------------\n");
    outputManager->printa_sf(thisAgent, "EBC Failure Statistics\n");
    outputManager->printa_sf(thisAgent, "----------------------\n");

    outputManager->printa_sf(thisAgent, "Duplicate chunk already existed            %u\n", stats.duplicates);
    outputManager->printa_sf(thisAgent, "Chunk tested local negation                %u\n", stats.tested_local_negation);
    outputManager->printa_sf(thisAgent, "Could not re-order chunk                   %u\n", stats.unorderable);
    outputManager->printa_sf(thisAgent, "Chunk had no grounds                       %u\n", stats.no_grounds);
    outputManager->printa_sf(thisAgent, "Already reached max-chunks                 %u\n", stats.max_chunks);
    outputManager->printa_sf(thisAgent, "Chunk formed did not match WM              %u\n", stats.chunk_did_not_match);
    outputManager->printa_sf(thisAgent, "Justification formed did not match WM      %u\n", stats.justification_did_not_match);

    outputManager->printa_sf(thisAgent, "------------------------\n");
    outputManager->printa_sf(thisAgent, "EBC Explainer Statistics\n");
    outputManager->printa_sf(thisAgent, "------------------------\n");
    outputManager->printa_sf(thisAgent, "Chunks records                             %u\n", total_recorded.chunks);
    outputManager->printa_sf(thisAgent, "Actions records                            %u\n", total_recorded.actions);
    outputManager->printa_sf(thisAgent, "Condition records                          %u\n", total_recorded.conditions);
    outputManager->printa_sf(thisAgent, "Instantiation records                      %u\n", total_recorded.instantiations);
    outputManager->printa_sf(thisAgent, "Instantiation references in conditions     %u\n", total_recorded.instantiations_referenced);
    outputManager->printa_sf(thisAgent, "Instantiations skipped                     %u\n", total_recorded.instantiations_skipped);
}

void Explanation_Logger::print_chunk_stats() {

    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "%fStatistics for '%y' (c%u):\n\n",                            current_discussed_chunk->name, current_discussed_chunk->chunkID);
    outputManager->printa_sf(thisAgent, "Number of conditions                       %u\n",        current_discussed_chunk->conditions->size());
    outputManager->printa_sf(thisAgent, "Number of actions                          %u\n",        current_discussed_chunk->actions->size());
    outputManager->printa_sf(thisAgent, "Base instantiation                         i%u\n",       current_discussed_chunk->baseInstantiation->instantiationID);
    outputManager->printa_sf(thisAgent, "Base instantiation matched rule            %y\n",        current_discussed_chunk->baseInstantiation->production_name);
    outputManager->printa_sf(thisAgent, "\nInstantiations backtraced through          %u\n",    current_discussed_chunk->stats.instantations_backtraced);
    outputManager->printa_sf(thisAgent, "Instantiations backtraced through twice    %u\n",      current_discussed_chunk->stats.seen_instantations_backtraced);
    outputManager->printa_sf(thisAgent, "Conditions merged                          %u\n",      current_discussed_chunk->stats.merged_conditions);
    outputManager->printa_sf(thisAgent, "Constraints collected                      %u\n",      current_discussed_chunk->stats.constraints_collected);
    outputManager->printa_sf(thisAgent, "Constraints attached                       %u\n",      current_discussed_chunk->stats.constraints_attached);

    outputManager->printa_sf(thisAgent, "\nDuplicates chunks later created            %u\n", current_discussed_chunk->stats.duplicates);
    outputManager->printa_sf(thisAgent, "Tested negation in local substate          %s\n", (current_discussed_chunk->stats.tested_local_negation ? "Yes" : "No"));

}

void Explanation_Logger::print_chunk_list(short pNumToPrint)
{
    short lNumPrinted = 0;
    for (std::unordered_map< Symbol*, chunk_record* >::iterator it = (*chunks).begin(); it != (*chunks).end(); ++it)
    {
        Symbol* d1 = it->first;
        chunk_record* d2 = it->second;
        outputManager->printa_sf(thisAgent, "%-%-%y (c%u)\n", it->first, it->second->chunkID);
        if (pNumToPrint && (lNumPrinted >= pNumToPrint))
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
    id_to_id_map_type* identity_mappings = current_discussed_chunk->identity_set_mappings;
    if (identity_mappings->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "This chunk had no identity to identity set assignments.\n");
        return;
    }

    std::unordered_map< uint64_t, uint64_t >::iterator iter;

    outputManager->set_column_indent(0, 6);
    outputManager->set_column_indent(1, 26);
    outputManager->set_column_indent(2, 31);
    outputManager->printa_sf(thisAgent, "ID %-Original %-Set %-Final\n\n");

    for (iter = identity_mappings->begin(); iter != identity_mappings->end(); ++iter)
    {
        outputManager->printa_sf(thisAgent, "%u%-%y%",
            iter->first, thisAgent->ebChunker->get_ovar_for_o_id(iter->first));
        if (iter->second == NULL_IDENTITY_SET)
        {
            outputManager->printa_sf(thisAgent, "%-0%-NULL IDENTITY SET (retains literal value)\n");

        } else {
            outputManager->printa_sf(thisAgent, "%-%u%-%y\n",
                iter->second, thisAgent->ebChunker->get_ovar_for_o_id(iter->second));
        }
    }
}

void Explanation_Logger::print_constraints_enforced()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "Constraints enforced during formation of chunk %y.\n\nNot yet implemented.\n", current_discussed_chunk->name);
}
