#include "debug.h"
#include "explain.h"
#include "agent.h"
#include "condition.h"
#include "instantiation.h"
#include "preference.h"
#include "production.h"
#include "working_memory.h"
#include "output_manager.h"

void Explanation_Logger::print_instantiation_explanation(instantiation_record* pInstRecord)
{
    outputManager->printa_sf(thisAgent, "Instantiation # %u   (match of rule %y)\n\n",
        pInstRecord->instantiationID, pInstRecord->production_name);

    if (pInstRecord->conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
    }
    else
    {
        condition_record* lCond;
        for (condition_record_list::iterator it = pInstRecord->conditions->begin(); it != pInstRecord->conditions->end(); it++)
        {
            lCond = (*it);
//                outputManager->printa_sf(thisAgent, "c%u: %l   from rule %s (i%u)\n", lCond->conditionID, lCond->variablized_cond,
//                    (lCond->parent_instantiation ? lCond->parent_instantiation->production_name->sc->name  : "Architecture"),
//                    (lCond->parent_instantiation ? lCond->parent_instantiation->instantiationID : 0));

                outputManager->printa_sf(thisAgent, "c%u: %l   from rule %s (i%u)\n", lCond->conditionID, lCond->instantiated_cond,
                    (lCond->parent_instantiation ? lCond->parent_instantiation->production_name->sc->name  : "Architecture"),
                    (lCond->parent_instantiation ? lCond->parent_instantiation->instantiationID : 0));

//                outputManager->printa_sf(thisAgent, "c%u: (%y ^%y %y)   from rule %s (i%u)\n",
//                    lCond->conditionID, lCond->matched_wme->id, lCond->matched_wme->attr, lCond->matched_wme->value,
//                    (lCond->parent_instantiation ? lCond->parent_instantiation->production_name->sc->name  : "Architecture"),
//                    (lCond->parent_instantiation ? lCond->parent_instantiation->instantiationID : 0));

        }
    }
    outputManager->printa(thisAgent, "-->\n");

    if (pInstRecord->actions->empty())
    {
        outputManager->printa(thisAgent, "No actions on right-hand-side\n");
    }
    else
    {
        action_record* lAction;
        for (action_record_list::iterator it = pInstRecord->actions->begin(); it != pInstRecord->actions->end(); it++)
        {
            lAction = (*it);
//                outputManager->printa_sf(thisAgent, "a%u: %a\n",
//                     lAction->actionID, lAction->variablized_action);

                outputManager->printa_sf(thisAgent, "a%u: %p\n",
                     lAction->actionID, lAction->instantiated_pref);

//                outputManager->printa_sf(thisAgent, "a%u: %p\n",
//                     lAction->actionID, lAction->instantiated_pref);
        }
    }
    outputManager->printa(thisAgent, "\nAdd the following to show additional information for any chunk or instantiation explanation:\n");
    outputManager->printa(thisAgent, "* 'explain [-e | --explanation-trace':      Print corresponding explanation trace structure\n");
    outputManager->printa(thisAgent, "* 'explain [-i | --identity':               Print showing underlying identity values\n");
}


void Explanation_Logger::print_chunk(EBCTraceType pType, chunk_record* pChunkRecord)
{
    if (current_discussed_chunk->conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
    }
    else
    {
        condition_record* lCond;
        for (condition_record_list::iterator it = current_discussed_chunk->conditions->begin(); it != current_discussed_chunk->conditions->end(); it++)
        {
            lCond = (*it);
            if (pType == ebc_chunk)
            {
                thisAgent->outputManager->set_print_test_format(true, false);
                outputManager->printa_sf(thisAgent, "   %l   c%u originally from rule %s (i%u)\n", lCond->variablized_cond, lCond->conditionID,
                    (lCond->parent_instantiation ? lCond->parent_instantiation->production_name->sc->name  : "Architecture"),
                    (lCond->parent_instantiation ? lCond->parent_instantiation->instantiationID : 0));
                thisAgent->outputManager->clear_print_test_format();
            } else if (pType == ebc_explanation)
            {
                thisAgent->outputManager->set_print_test_format(false, true);
                outputManager->printa_sf(thisAgent, "   %l   c%u originally from rule %s (i%u)\n", lCond->instantiated_cond, lCond->conditionID,
                    (lCond->parent_instantiation ? lCond->parent_instantiation->production_name->sc->name  : "Architecture"),
                    (lCond->parent_instantiation ? lCond->parent_instantiation->instantiationID : 0));
                thisAgent->outputManager->clear_print_test_format();
            } else if (pType == ebc_match)
            {
                outputManager->printa_sf(thisAgent, "   (%y ^%y %y)   matched condition c%u in rule %s (i%u)\n",
                    lCond->matched_wme->id, lCond->matched_wme->attr, lCond->matched_wme->value, lCond->conditionID,
                    (lCond->parent_instantiation ? lCond->parent_instantiation->production_name->sc->name  : "Architecture"),
                    (lCond->parent_instantiation ? lCond->parent_instantiation->instantiationID : 0));
            } else if (pType == ebc_original)
            {
            } else {
                assert(false);
            }
        }
    }
    outputManager->printa(thisAgent, "-->\n");

    if (current_discussed_chunk->actions->empty())
    {
        outputManager->printa(thisAgent, "No actions on right-hand-side\n");
    }
    else
    {
        action_record* lAction;
        for (action_record_list::iterator it = current_discussed_chunk->actions->begin(); it != current_discussed_chunk->actions->end(); it++)
        {
            lAction = (*it);
            if (pType == ebc_chunk)
            {
//                outputManager->printa_sf(thisAgent, "   %a              a%u\n",
//                     lAction->variablized_action, lAction->actionID);
                thisAgent->outputManager->set_print_test_format(false, true);
                outputManager->printa_sf(thisAgent, "   %p              a%u\n",
                     lAction->instantiated_pref, lAction->actionID);
                thisAgent->outputManager->clear_print_test_format();
            } else if (pType == ebc_explanation)
            {
                thisAgent->outputManager->set_print_test_format(false, true);
                outputManager->printa_sf(thisAgent, "   %p              a%u\n",
                     lAction->instantiated_pref, lAction->actionID);
                thisAgent->outputManager->clear_print_test_format();
            } else if (pType == ebc_match)
            {
                thisAgent->outputManager->set_print_test_format(true, false);
                outputManager->printa_sf(thisAgent, "   %p              a%u\n",
                     lAction->instantiated_pref, lAction->actionID);
                thisAgent->outputManager->clear_print_test_format();
            } else if (pType == ebc_original)
            {
            } else {
                assert(false);
            }
        }
    }
}

void Explanation_Logger::print_chunk_explanation()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "Explanation of the formation of '%y' (c%u):\n\n", current_discussed_chunk->name, current_discussed_chunk->chunkID);

    outputManager->printa_sf(thisAgent, "   (1) At t?, rule '%y'(i%u) matched, fired and created a result    ('explain instantiation %u')\n",
        current_discussed_chunk->baseInstantiation->production_name, current_discussed_chunk->baseInstantiation->instantiationID, current_discussed_chunk->baseInstantiation->instantiationID);
    outputManager->printa_sf(thisAgent, "   (2) Conditions of i%u and CDPS are backtraced through            ('explain --backtrace')\n",  current_discussed_chunk->baseInstantiation->instantiationID);
    outputManager->printa(thisAgent, "   (3) EBC magic happens to produce the following chunk:\n\n");
    outputManager->printa_sf(thisAgent, "sp {%y\n", current_discussed_chunk->name);
//    print_chunk(ebc_chunk, current_discussed_chunk);
//    outputManager->printa_sf(thisAgent, "}\nExplanation trace:\nsp {%y\n", current_discussed_chunk->name);
    print_chunk(ebc_chunk, current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "}\n\nWorking memory trace:\n\n");
    print_chunk(ebc_match, current_discussed_chunk);
    outputManager->printa(thisAgent, "\nThe following commands now apply to this chunk:\n");
    outputManager->printa(thisAgent, "* 'explain [-b | --backtrace]':              Explain problem-solving backtrace\n");
    outputManager->printa(thisAgent, "* 'explain [-c | --constraints]':            Explain constraints required by problem-solving\n");
    outputManager->printa_sf(thisAgent, "* 'explain [-s | --stats]':                  Print statistics for %y\n", current_discussed_chunk->name);
    outputManager->printa(thisAgent, "\nAdd the following to show additional information for any chunk or instantiation explanation:\n");
    outputManager->printa(thisAgent, "* 'explain [-e | --explanation-trace]':      Print corresponding explanation trace structure\n");
    outputManager->printa(thisAgent, "* 'explain [-i | --identity]':               Print showing underlying identity values\n");
}

void Explanation_Logger::explain_summary()
{
    outputManager->printa_sf(thisAgent, "%f-------------------\nExplainer Settings:\n-------------------\n");
    outputManager->printa_sf(thisAgent, "Watch all chunk formations                 %s\n", (enabled ? "Yes" : "No"));
    outputManager->printa_sf(thisAgent, "Number of specific rules watched           %d\n", num_rules_watched);

    /* Print specific watched rules and time interval when watch all disabled */
    if (!enabled)
    {
        /* Print last 10 rules watched*/
        outputManager->printa(thisAgent, "--------------\nRules watched:\n--------------\n");
        print_rules_watched(10);
        /* When we add time interval option, print it here */
    }

    outputManager->printa(thisAgent, "--------------------------------\nChunks available for discussion:\n--------------------------------\n");
    print_chunk_list(10);

    /* Print current chunk and last 10 chunks formed */
    outputManager->printa_sf(thisAgent, "\n   Current chunk being discussed:          %s",
        (current_discussed_chunk ? current_discussed_chunk->name->sc->name : "None" ));
    if (current_discussed_chunk)
    {
        outputManager->printa_sf(thisAgent, "(c%u)\n\n", current_discussed_chunk->chunkID);
    } else {
        outputManager->printa(thisAgent, "\n\n");
    }
    outputManager->printa(thisAgent, "Type 'explain [chunk-name]' to discuss the formation of that chunk.\n");
}

void Explanation_Logger::print_all_watched_rules()
{
    outputManager->printa(thisAgent, "--------------\nRules watched:\n--------------\n");
    print_rules_watched(0);
}


void Explanation_Logger::print_all_chunks()
{
    outputManager->printa(thisAgent, "--------------------------------\nChunks available for discussion:\n--------------------------------\n");
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

    outputManager->printa_sf(thisAgent, "\nConditions merged                        %u\n", stats.merged_conditions);
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

void Explanation_Logger::explain_chunk_stats() {

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

    outputManager->printa_sf(thisAgent, "\nDuplicates chunks later created  %u\n", current_discussed_chunk->stats.duplicates);
    outputManager->printa_sf(thisAgent, "Tested negation in local substate          %s\n", (current_discussed_chunk->stats.tested_local_negation ? "Yes" : "No"));

}

void Explanation_Logger::print_chunk_list(short pNumToPrint)
{
    short lNumPrinted = 0;
    for (std::unordered_map< Symbol*, chunk_record* >::iterator it = (*chunks).begin(); it != (*chunks).end(); ++it)
    {
        Symbol* d1 = it->first;
        chunk_record* d2 = it->second;
        outputManager->printa_sf(thisAgent, "   %y (c%u)\n", it->first, it->second->chunkID);
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
            outputManager->printa_sf(thisAgent, "   %y\n", prod->name);
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

void Explanation_Logger::print_dependency_analysis()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "Printing dependency analysis of chunk %y.\n", current_discussed_chunk->name);
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

    outputManager->printa_sf(thisAgent, "---------------------------------------------------------\n"
                                        "Variable identities --> Identity sets --> Chunk variables\n"
                                        "---------------------------------------------------------\n", current_discussed_chunk->name);
    std::unordered_map< uint64_t, uint64_t >::iterator iter;

    for (iter = identity_mappings->begin(); iter != identity_mappings->end(); ++iter)
    {
        outputManager->printa_sf(thisAgent, "o%u(%y)  --> o%u(%y)  -->\n",
            iter->first, thisAgent->ebChunker->get_ovar_for_o_id(iter->first),
            iter->second, thisAgent->ebChunker->get_ovar_for_o_id(iter->second));
    }
}

void Explanation_Logger::print_constraints_enforced()
{
    assert(current_discussed_chunk);
    outputManager->printa_sf(thisAgent, "Constraints enforced during formation of chunk %y.\n\nNot yet implemented.\n", current_discussed_chunk->name);
}
