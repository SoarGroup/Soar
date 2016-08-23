/*
 * smem_install.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */
#include "semantic_memory.h"
#include "smem_settings.h"
#include "smem_timers.h"
#include "smem_db.h"

#include "agent.h"
#include "dprint.h"
#include "ebc.h"
#include "instantiation.h"
#include "mem.h"
#include "preference.h"
#include "production.h"
#include "symbol_manager.h"
#include "working_memory_activation.h"

void SMem_Manager::install_buffered_triple_list(Symbol* state, wme_set& cue_wmes, symbol_triple_list& my_list, bool meta)
{
    if (my_list.empty())
    {
        return;
    }

    instantiation* inst = make_architectural_instantiation(thisAgent, state, &cue_wmes, &my_list);
    for (preference* pref = inst->preferences_generated; pref;)
    {
        // add the preference to temporary memory

        if (add_preference_to_tm(thisAgent, pref))
        {
            // and add it to the list of preferences to be removed
            // when the goal is removed
            insert_at_head_of_dll(state->id->preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev);
            pref->on_goal_list = true;

            if (meta)
            {
                // if this is a meta wme, then it is completely local
                // to the state and thus we will manually remove it
                // (via preference removal) when the time comes
                state->id->smem_info->smem_wmes->push_back(pref);
            }
        }
        else
        {
            if (pref->reference_count == 0)
            {
                preference* previous = pref;
                pref = pref->inst_next;
                possibly_deallocate_preference_and_clones(thisAgent, previous);
                continue;
            }
        }

        pref = pref->inst_next;
    }

    if (!meta)
    {
        // otherwise, we submit the fake instantiation to backtracing
        // such as to potentially produce justifications that can follow
        // it to future adventures (potentially on new states)
        instantiation* my_justification_list = NIL;
        dprint(DT_MILESTONES, "Calling chunk instantiation from _smem_process_buffered_wme_list...\n");
        thisAgent->explanationBasedChunker->set_learning_for_instantiation(inst);
        thisAgent->explanationBasedChunker->build_chunk_or_justification(inst, &my_justification_list);

        // if any justifications are created, assert their preferences manually
        // (copied mainly from assert_new_preferences with respect to our circumstances)
        if (my_justification_list != NIL)
        {
            preference* just_pref = NIL;
            instantiation* next_justification = NIL;

            for (instantiation* my_justification = my_justification_list;
                my_justification != NIL;
                my_justification = next_justification)
            {
                next_justification = my_justification->next;

                if (my_justification->in_ms)
                {
                    insert_at_head_of_dll(my_justification->prod->instantiations, my_justification, next, prev);
                }

                for (just_pref = my_justification->preferences_generated; just_pref != NIL;)
                {
                    if (add_preference_to_tm(thisAgent, just_pref))
                    {
                        if (wma_enabled(thisAgent))
                        {
                            wma_activate_wmes_in_pref(thisAgent, just_pref);
                        }
                    }
                    else
                    {
                        if (just_pref->reference_count == 0)
                        {
                            preference* previous = just_pref;
                            just_pref = just_pref->inst_next;
                            possibly_deallocate_preference_and_clones(thisAgent, previous);
                            continue;
                        }
                    }

                    just_pref = just_pref->inst_next;
                }
            }
        }
    }
}

void SMem_Manager::install_recall_buffer(Symbol* state, wme_set& cue_wmes, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes)
{
    install_buffered_triple_list(state, cue_wmes, meta_wmes, true);
    install_buffered_triple_list(state, cue_wmes, retrieval_wmes, false);
}

void SMem_Manager::add_triple_to_recall_buffer(symbol_triple_list& my_list, Symbol* id, Symbol* attr, Symbol* value)
{
    my_list.push_back(new symbol_triple(id, attr, value));

    thisAgent->symbolManager->symbol_add_ref(id);
    thisAgent->symbolManager->symbol_add_ref(attr);
    thisAgent->symbolManager->symbol_add_ref(value);
}

void SMem_Manager::clear_instance_mappings()
{
    lti_to_sti_map.clear();
    sti_to_identity_map.clear();
}


uint64_t SMem_Manager::get_identity_for_recalled_sti(Symbol* pSTI, uint64_t pI_ID)
{
    sym_to_id_map::iterator lIter;

    lIter = sti_to_identity_map.find(pSTI);

    if (lIter != sti_to_identity_map.end())
    {
        return (lIter->second);
    } else {
        uint64_t return_val;
        return_val = thisAgent->explanationBasedChunker->get_or_create_o_id(pSTI, pI_ID);
        sti_to_identity_map[pSTI] = return_val;
        return return_val;
    }
}

Symbol* SMem_Manager::get_sti_for_lti(smem_lti_id pLTI_ID, goal_stack_level pLevel, char pChar)
{
    id_to_sym_map::iterator lIter;

    lIter = lti_to_sti_map.find(pLTI_ID);

    if (lIter != lti_to_sti_map.end())
    {
        thisAgent->symbolManager->symbol_add_ref(lIter->second);
        return (lIter->second);
    } else {
        Symbol* return_val;
        return_val = thisAgent->symbolManager->make_new_identifier(pChar, pLevel, NIL);
        return_val->id->level = pLevel;
        return_val->id->promotion_level = pLevel;
        return_val->id->smem_lti = pLTI_ID;
        lti_to_sti_map[pLTI_ID] = return_val;
        return return_val;
    }
}

void SMem_Manager::install_memory(Symbol* state, smem_lti_id lti_id, Symbol* sti, bool activate_lti, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes, smem_install_type install_type, uint64_t depth, std::set<smem_lti_id>* visited)
{
    ////////////////////////////////////////////////////////////////////////////
    smem_timers->ncb_retrieval->start();
    ////////////////////////////////////////////////////////////////////////////

    // get the ^result header for this state
    Symbol* result_header = NULL;
    if (install_type == wm_install)
    {
        result_header = state->id->smem_result_header;
    }

    // get identifier if not known
    bool sti_created_here = false;
    if (install_type == wm_install)
    {
        if (sti == NIL)
        {
            //        soar_module::sqlite_statement* q = smem_stmts->lti_letter_num;
            //
            //        q->bind_int(1, lti_id);
            //        q->execute();
            //
            //        sti = lti_soar_make(lti_id, static_cast<char>(q->column_int(0)), static_cast<uint64_t>(q->column_int(1)), result_header->id->level);
            //
            //        q->reinitialize();
            sti = get_sti_for_lti(lti_id, result_header->id->level);
            sti_created_here = true;
        } else {
            assert(sti->id->smem_lti && sti->id->level && (sti->id->level <= result_header->id->level));
        }
    }
    // activate lti
    if (activate_lti)
    {
        lti_activate(lti_id, true);
    }

    // point retrieved to lti
    if (install_type == wm_install)
    {
        if (visited == NULL)
        {
            add_triple_to_recall_buffer(meta_wmes, result_header, thisAgent->symbolManager->soarSymbols.smem_sym_retrieved, sti);
        }
        else
        {
            add_triple_to_recall_buffer(meta_wmes, result_header, thisAgent->symbolManager->soarSymbols.smem_sym_depth_retrieved, sti);
        }
    }

    /* MToDo | Not sure if this is still needed */
    if (sti_created_here)
    {
        // if the identifier was created above we need to
        // remove a single ref count AFTER the wme
        // is added (such as to not deallocate the symbol
        // prematurely)
        thisAgent->symbolManager->symbol_remove_ref(&sti);
    }

    bool triggered = false;

    // if no children, then retrieve children
    // merge may override this behavior
    if (((smem_params->merge->get_value() == smem_param_container::merge_add) ||
            ((sti->id->impasse_wmes == NIL) &&
             (sti->id->input_wmes == NIL) &&
             (sti->id->slots == NIL)))
            || (install_type == fake_install)) //(The final bit is if this is being called by the remove command.)

    {
        if (visited == NULL)
        {
            triggered = true;
            visited = new std::set<smem_lti_id>;
        }

        soar_module::sqlite_statement* expand_q = smem_stmts->web_expand;
        Symbol* attr_sym;
        Symbol* value_sym;

        // get direct children: attr_type, attr_hash, value_type, value_hash, value_letter, value_num, value_lti
        expand_q->bind_int(1, lti_id);

        std::set<Symbol*> children;

        while (expand_q->execute() == soar_module::row)
        {
            // make the identifier symbol irrespective of value type
            attr_sym = rhash_(static_cast<byte>(expand_q->column_int(0)), static_cast<smem_hash_id>(expand_q->column_int(1)));

            // identifier vs. constant
            if (expand_q->column_int(6) != SMEM_AUGMENTATIONS_NULL)
            {
                value_sym = get_sti_for_lti(static_cast<smem_lti_id>(expand_q->column_int(6)), sti->id->level, static_cast<char>(expand_q->column_int(4)));
//                value_sym = lti_soar_make(static_cast<smem_lti_id>(expand_q->column_int(6)), static_cast<char>(expand_q->column_int(4)), static_cast<uint64_t>(expand_q->column_int(5)), sti->id->level);
                if (depth > 1)
                {
                    children.insert(value_sym);
                }
            }
            else
            {
                value_sym = rhash_(static_cast<byte>(expand_q->column_int(2)), static_cast<smem_hash_id>(expand_q->column_int(3)));
            }

            // add wme
            add_triple_to_recall_buffer(retrieval_wmes, sti, attr_sym, value_sym);

            // deal with ref counts - attribute/values are always created in this function
            // (thus an extra ref count is set before adding a wme)
            thisAgent->symbolManager->symbol_remove_ref(&attr_sym);
            thisAgent->symbolManager->symbol_remove_ref(&value_sym);
        }
        expand_q->reinitialize();

        //Attempt to find children for the case of depth.
        std::set<Symbol*>::iterator iterator;
        std::set<Symbol*>::iterator end = children.end();
        for (iterator = children.begin(); iterator != end; ++iterator)
        {
            if (visited->find((*iterator)->id->smem_lti) == visited->end())
            {
                visited->insert((*iterator)->id->smem_lti);
                install_memory(state, (*iterator)->id->smem_lti, (*iterator), (smem_params->activate_on_query->get_value() == on), meta_wmes, retrieval_wmes, install_type, depth - 1, visited);
            }
        }
    }

    if (triggered)
    {
        delete visited;
    }

    ////////////////////////////////////////////////////////////////////////////
    smem_timers->ncb_retrieval->stop();
    ////////////////////////////////////////////////////////////////////////////
}

