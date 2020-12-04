#include "semantic_memory.h"

#include "smem_settings.h"
#include "smem_stats.h"
#include "smem_timers.h"
#include "smem_db.h"
#include "smem_structs.h"

#include "agent.h"
#include "condition.h"
#include "decide.h"
#include "ebc.h"
#include "episodic_memory.h"
#include "instantiation.h"
#include "lexer.h"
#include "rhs.h"
#include "test.h"
#include "parser.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "slot.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "working_memory.h"
#include "working_memory_activation.h"
#include "xml.h"

#include <list>
#include <map>
#include <queue>
#include <utility>
#include <ctype.h>
#include <fstream>
#include <algorithm>

#include "smem_math_query.h"

wme_list* SMem_Manager::get_direct_augs_of_id(Symbol* id, tc_number tc)
{
    slot* s;
    wme* w;
    wme_list* return_val = new wme_list;

    // augs only exist for identifiers
    if (id->is_sti())
    {
        if (tc != NIL)
        {
            if (tc == id->tc_num)
            {
                return return_val;
            }
            else
            {
                id->tc_num = tc;
            }
        }

        // impasse wmes
        for (w = id->id->impasse_wmes; w != NIL; w = w->next)
        {
            if (!w->acceptable)
            {
                return_val->push_back(w);
            }
        }

        // input wmes
        for (w = id->id->input_wmes; w != NIL; w = w->next)
        {
            return_val->push_back(w);
        }

        // regular wmes
        for (s = id->id->slots; s != NIL; s = s->next)
        {
            for (w = s->wmes; w != NIL; w = w->next)
            {
                if (!w->acceptable)
                {
                    return_val->push_back(w);
                }
            }
        }
    }

    return return_val;
}

void SMem_Manager::go(bool store_only)
{
    thisAgent->SMem->timers->total->start();

#ifndef SMEM_EXPERIMENT

    respond_to_cmd(store_only);

#else // SMEM_EXPERIMENT

#endif // SMEM_EXPERIMENT

    thisAgent->SMem->timers->total->stop();
}

void SMem_Manager::respond_to_cmd(bool store_only)
{

    attach();

    // start at the bottom and work our way up
    // (could go in the opposite direction as well)
    Symbol* state = thisAgent->bottom_goal;

    wme_list* wmes;
    wme_list* cmds;
    wme_list::iterator w_p;

    symbol_triple_list meta_wmes;
    symbol_triple_list retrieval_wmes;
    wme_set cue_wmes;

    Symbol* query;
    std::list<Symbol*> orquery;
    Symbol* negquery;
    Symbol* retrieve;
    Symbol* math;
    uint64_t depth;
    bool update_LTI_Links = false;
    bool link_to_ltm = true;
    symbol_list prohibit;
    symbol_list store;

    enum path_type { blank_slate, cmd_bad, cmd_retrieve, cmd_query, cmd_store_new, cmd_store, cmd_prohibit } path;

    unsigned int time_slot = ((store_only) ? (1) : (0));
    uint64_t wme_count;
    bool new_cue;

    tc_number tc;

    Symbol* parent_sym;
    std::queue<Symbol*> syms;

    int parent_level;
    std::queue<int> levels;

    bool do_wm_phase = false;

    //Free this up as soon as we start a phase that allows queries
    if(!store_only){
        delete thisAgent->lastCue;
        thisAgent->lastCue = NULL;
    }

    while (state != NULL)
    {
        ////////////////////////////////////////////////////////////////////////////
        thisAgent->SMem->timers->api->start();
        ////////////////////////////////////////////////////////////////////////////

        // make sure this state has had some sort of change to the cmd
        // NOTE: we only care one-level deep!
        new_cue = false;
        wme_count = 0;
        cmds = NIL;
        {
            tc = get_new_tc_number(thisAgent);

            // initialize BFS at command
            syms.push(state->id->smem_info->cmd_wme->value); // smem_cmd_header
            levels.push(0);

            while (!syms.empty())
            {
                // get state
                parent_sym = syms.front();
                syms.pop();

                parent_level = levels.front();
                levels.pop();

                // get children of the current identifier
                wmes = get_direct_augs_of_id(parent_sym, tc);
                {
                    for (w_p = wmes->begin(); w_p != wmes->end(); w_p++)
                    {
                        if (((store_only) && ((parent_level != 0) || ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_store))) ||
                                ((!store_only) && ((parent_level != 0) || ((*w_p)->attr != thisAgent->symbolManager->soarSymbols.smem_sym_store))))
                        {
                            wme_count++;

                            if ((*w_p)->timetag > state->id->smem_info->last_cmd_time[ time_slot ])
                            {
                                new_cue = true;
                                state->id->smem_info->last_cmd_time[ time_slot ] = (*w_p)->timetag;
                            }

                            if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                    (parent_level == 0) &&
                                    (((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_query) || ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_store)))
                            {
                                syms.push((*w_p)->value);
                                levels.push(parent_level + 1);
                            }
                        }
                    }

                    // free space from aug list
                    if (cmds == NIL)
                    {
                        cmds = wmes;
                    }
                    else
                    {
                        delete wmes;
                    }
                }
            }

            // see if any WMEs were removed
            if (state->id->smem_info->last_cmd_count[ time_slot ] != wme_count)
            {
                new_cue = true;
                state->id->smem_info->last_cmd_count[ time_slot ] = wme_count;
            }


            if (new_cue)
            {
                // clear old results
                clear_result(state);

                do_wm_phase = true;
            }
        }

        // a command is issued if the cue is new
        // and there is something on the cue
        if (new_cue && wme_count)
        {
            cue_wmes.clear();
            meta_wmes.clear();
            retrieval_wmes.clear();

            // initialize command vars
            retrieve = NIL;
            query = NIL;
            negquery = NIL;
            math = NIL;
            store.clear();
            prohibit.clear();
            path = blank_slate;
            depth = 1;

            // process top-level symbols
            for (w_p = cmds->begin(); w_p != cmds->end(); w_p++)
            {
                cue_wmes.insert((*w_p));

                if (path != cmd_bad)
                {
                    // collect information about known commands
                    if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_retrieve)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                (path == blank_slate))
                        {
                            retrieve = (*w_p)->value;
                            path = cmd_retrieve;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_query)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate || path == cmd_prohibit) || (path == cmd_query)) &&
                                (query == NIL))

                        {
                            query = (*w_p)->value;
                            orquery.push_back((*w_p)->value);
                            path = cmd_query;
                        }
                        /*else if ((path == cmd_query) && (query != NIL))
                        {
                            //Since we have already received a query command and now we are receiving another, this must be an or-query.
                            //This means that we will issue several queries and return the winner among the multiple queries.
                            //The logic amounts to performing an or/union operation over the separate queries.
                            orquery.push_back((*w_p)->value);
                        }*/
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_negquery)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate || path == cmd_prohibit) || (path == cmd_query)) &&
                                (negquery == NIL))

                        {
                            negquery = (*w_p)->value;
                            path = cmd_query;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_prohibit)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate || path == cmd_prohibit) || (path == cmd_query)) &&
                                ((*w_p)->value->id->LTI_ID != NIL))
                        {
                            prohibit.push_back((*w_p)->value);
                            if (path == blank_slate || path == cmd_prohibit)
                            {
                                path = cmd_prohibit;
                            }
                            else
                            {
                                path = cmd_query;
                            }
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_math_query)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate || path == cmd_prohibit) || (path == cmd_query)) &&
                                (math == NIL))
                        {
                            math = (*w_p)->value;
                            path = cmd_query;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_store)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate) || (path == cmd_store)))
                        {
                            store.push_back((*w_p)->value);
                            path = cmd_store;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_store_new)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate) || (path == cmd_store_new)))
                        {
                            store.push_back((*w_p)->value);
                            path = cmd_store_new;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_overwrite)
                    {
                        if (((*w_p)->value->symbol_type == STR_CONSTANT_SYMBOL_TYPE) &&
                            (((*w_p)->value == thisAgent->symbolManager->soarSymbols.yes) ||
                                ((*w_p)->value == thisAgent->symbolManager->soarSymbols.no)) &&
                                ((path == blank_slate) || (path == cmd_store_new)))
                        {
                            update_LTI_Links = ((*w_p)->value == thisAgent->symbolManager->soarSymbols.yes);
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_link_to_ltm)
                    {
                        if (((*w_p)->value->symbol_type == STR_CONSTANT_SYMBOL_TYPE) &&
                            (((*w_p)->value == thisAgent->symbolManager->soarSymbols.yes) ||
                                ((*w_p)->value == thisAgent->symbolManager->soarSymbols.no)) &&
                                ((path == blank_slate) || (path == cmd_retrieve) || (path == cmd_query)))
                        {
                            link_to_ltm = ((*w_p)->value == thisAgent->symbolManager->soarSymbols.yes);
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_depth)
                    {
                        if ((*w_p)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
                        {
                            depth = ((*w_p)->value->ic->value > 0) ? (*w_p)->value->ic->value : 1;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else
                    {
                        path = cmd_bad;
                    }
                }
            }

            // if on path 3 must have query/neg-query
            if ((path == cmd_query) && (query == NULL))
            {
                path = cmd_bad;
            }

            // must be on a path
            if (path == blank_slate)
            {
                path = cmd_bad;
            }

            ////////////////////////////////////////////////////////////////////////////
            thisAgent->SMem->timers->api->stop();
            ////////////////////////////////////////////////////////////////////////////

            // process command
            if (path != cmd_bad)
            {
                // performing any command requires an initialized database
                attach();
                clear_instance_mappings();

                // retrieve
                if (path == cmd_retrieve)
                {
                    if (retrieve->id->LTI_ID == NIL)
                    {
                        // retrieve is not pointing to an lti!
                        add_triple_to_recall_buffer(meta_wmes, state->id->smem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.smem_sym_failure, retrieve);
                    }
                    else
                    {
                        // status: success
                        add_triple_to_recall_buffer(meta_wmes, state->id->smem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.smem_sym_success, retrieve);

                        // install memory directly onto the retrieve identifier
                        install_memory(state, retrieve->id->LTI_ID, NULL, true, meta_wmes, retrieval_wmes, wm_install, depth);

                        // add one to the expansions stat
                        thisAgent->SMem->statistics->retrievals->set_value(thisAgent->SMem->statistics->retrievals->get_value() + 1);
                    }
                }
                // query
                else if (path == cmd_query)
                {
                    id_set prohibit_lti;
                    symbol_list::iterator sym_p;

                    for (sym_p = prohibit.begin(); sym_p != prohibit.end(); sym_p++)
                    {
                        prohibit_lti.insert((*sym_p)->id->LTI_ID);
                    }
                    process_query(state, orquery, negquery, math, &(prohibit_lti), cue_wmes, meta_wmes, retrieval_wmes, qry_full, 1, NIL, depth, wm_install);

                    // add one to the cbr stat
                    thisAgent->SMem->statistics->queries->set_value(thisAgent->SMem->statistics->queries->get_value() + 1);
                }
                else if (path == cmd_store_new)
                {
                    symbol_list::iterator sym_p;

                    ////////////////////////////////////////////////////////////////////////////
                    thisAgent->SMem->timers->storage->start();
                    ////////////////////////////////////////////////////////////////////////////

                    // start transaction (if not lazy)
                    if (thisAgent->SMem->settings->lazy_commit->get_value() == off)
                    {
                        thisAgent->SMem->SQL->begin->execute(soar_module::op_reinit);
                    }

                    for (sym_p = store.begin(); sym_p != store.end(); sym_p++)
                    {
                        store_new((*sym_p), store_level, update_LTI_Links);

                        // status: success
                        add_triple_to_recall_buffer(meta_wmes, state->id->smem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.smem_sym_success, (*sym_p));

                        // add one to the store stat
                        thisAgent->SMem->statistics->stores->set_value(thisAgent->SMem->statistics->stores->get_value() + 1);
                    }

                    // commit transaction (if not lazy)
                    if (thisAgent->SMem->settings->lazy_commit->get_value() == off)
                    {
                        thisAgent->SMem->SQL->commit->execute(soar_module::op_reinit);
                    }

                    ////////////////////////////////////////////////////////////////////////////
                    thisAgent->SMem->timers->storage->stop();
                    ////////////////////////////////////////////////////////////////////////////
                }
                else if (path == cmd_store)
                {
                    symbol_list::iterator sym_p;

                    ////////////////////////////////////////////////////////////////////////////
                    thisAgent->SMem->timers->storage->start();
                    ////////////////////////////////////////////////////////////////////////////

                    // start transaction (if not lazy)
                    if (thisAgent->SMem->settings->lazy_commit->get_value() == off)
                    {
                        thisAgent->SMem->SQL->begin->execute(soar_module::op_reinit);
                    }

                    for (sym_p = store.begin(); sym_p != store.end(); sym_p++)
                    {
                        update((*sym_p), store_level, update_LTI_Links);

                        // status: success
                        add_triple_to_recall_buffer(meta_wmes, state->id->smem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.smem_sym_success, (*sym_p));

                        // add one to the store stat
                        thisAgent->SMem->statistics->stores->set_value(thisAgent->SMem->statistics->stores->get_value() + 1);
                    }

                    // commit transaction (if not lazy)
                    if (thisAgent->SMem->settings->lazy_commit->get_value() == off)
                    {
                        thisAgent->SMem->SQL->commit->execute(soar_module::op_reinit);
                    }

                    ////////////////////////////////////////////////////////////////////////////
                    thisAgent->SMem->timers->storage->stop();
                    ////////////////////////////////////////////////////////////////////////////
                    /*
                     *     prohibit_set = new soar_module::sqlite_statement(new_db, "UPDATE smem_prohibited SET prohibited=1,dirty=1 WHERE lti_id=?");
    add(prohibit_set);

    prohibit_add = new soar_module::sqlite_statement(new_db, "INSERT OR IGNORE INTO smem_prohibited (lti_id,prohibited,dirty) VALUES (?,0,0)");
    add(prohibit_add);

    prohibit_check = new soar_module::sqlite_statement(new_db, "SELECT lti_id,dirty FROM smem_prohibited WHERE lti_id=? AND prohibited=1");
    add(prohibit_check);
                     */
                }
                else if (path == cmd_prohibit)
                {
                    symbol_list::iterator sym_p;

                    for (sym_p = prohibit.begin(); sym_p != prohibit.end(); sym_p++)
                    {
                        SQL->prohibit_check->bind_int(1, (*sym_p)->id->LTI_ID);
                        if (SQL->prohibit_check->execute() != soar_module::row)
                        {
                            SQL->prohibit_set->bind_int(1, (*sym_p)->id->LTI_ID);
                            SQL->prohibit_set->execute(soar_module::op_reinit);
                        }
                        SQL->prohibit_check->reinitialize();
                    }
                    /*
                     * This allows prohibits to modify BLA without a query present.
                     */
                }
            }
            else
            {
                add_triple_to_recall_buffer(meta_wmes, state->id->smem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.smem_sym_bad_cmd, state->id->smem_info->cmd_wme->value);
            }

            if (!meta_wmes.empty() || !retrieval_wmes.empty())
            {

                // process preference assertion en masse
                install_recall_buffer(state, cue_wmes, meta_wmes, retrieval_wmes, !link_to_ltm);

                // clear cache
                {
                    symbol_triple_list::iterator mw_it;

                    for (mw_it = retrieval_wmes.begin(); mw_it != retrieval_wmes.end(); mw_it++)
                    {
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->id);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->attr);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->value);
                        thisAgent->memoryManager->free_with_pool(MP_sym_triple, (*mw_it));
                    }
                    retrieval_wmes.clear();

                    for (mw_it = meta_wmes.begin(); mw_it != meta_wmes.end(); mw_it++)
                    {
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->id);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->attr);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->value);
                        thisAgent->memoryManager->free_with_pool(MP_sym_triple, (*mw_it));
                    }
                    meta_wmes.clear();
                }

                // process wm changes on this state
                do_wm_phase = true;
            }

            // clear cue wmes
            cue_wmes.clear();
        }
        else
        {
            ////////////////////////////////////////////////////////////////////////////
            thisAgent->SMem->timers->api->stop();
            ////////////////////////////////////////////////////////////////////////////
        }

        // free space from aug list
        delete cmds;

        state = state->id->higher_goal;
    }

    if (do_wm_phase)
    {
        do_working_memory_phase(thisAgent);
    }
}

void SMem_Manager::clear_result(Symbol* state)
{
    preference* pref;

    while (!state->id->smem_info->smem_wmes->empty())
    {
        pref = state->id->smem_info->smem_wmes->back();
        state->id->smem_info->smem_wmes->pop_back();

        if (pref->in_tm)// && pref->slot)
        {
            remove_preference_from_tm(thisAgent, pref);
        }
    }
}

// performs cleanup when a state is removed
void SMem_Manager::reset(Symbol* state)
{
    if (state == NULL)
    {
        state = thisAgent->top_goal;
    }

    while (state)
    {
        smem_data* data = state->id->smem_info;

        data->last_cmd_time[0] = 0;
        data->last_cmd_time[1] = 0;
        data->last_cmd_count[0] = 0;
        data->last_cmd_count[1] = 0;

        // this will be called after prefs from goal are already removed,
        // so just clear out result stack
        data->smem_wmes->clear();

        state = state->id->lower_goal;
    }
}

bool SMem_Manager::clear()
{
    if (thisAgent->SMem->connected())
    {
        boolean oldAppendSetting = thisAgent->SMem->settings->append_db->get_value();
        thisAgent->SMem->settings->append_db->set_value(off);
        close();
        init_db();
        thisAgent->SMem->settings->append_db->set_value(oldAppendSetting);
        return true;
    }
    return false;
}

void SMem_Manager::reinit()
{
    if (thisAgent->SMem->connected() && (thisAgent->SMem->settings->database->get_value() == smem_param_container::file))
    {
        close();
        init_db();
    }
}

bool SMem_Manager::edge_updating_on()
{
    if (thisAgent->SMem->settings->spreading_edge_updating->get_value() == on)
    {
        return true;
    }
    return false;
}

SMem_Manager::SMem_Manager(agent* myAgent)
{
    thisAgent = myAgent;
    thisAgent->SMem = this;

    settings = new smem_param_container(thisAgent);
    statistics = new smem_stat_container(thisAgent);
    timers = new smem_timer_container(thisAgent);

    DB = new soar_module::sqlite_database();

    smem_validation = 0;

    smem_in_wmem = new std::map<uint64_t, uint64_t>();
    smem_wmas = new smem_wma_map();
    smem_spreaded_to = new std::unordered_map<uint64_t, int64_t>();
    smem_recipient = new std::unordered_map<uint64_t, int64_t>();
    smem_recipients_of_source = new std::unordered_map<uint64_t,std::set<uint64_t>*>();
    smem_context_additions = new std::set<uint64_t>();
    smem_context_removals = new std::set<uint64_t>();
    smem_edges_to_update = new smem_update_map();

};

void SMem_Manager::clean_up_for_agent_deletion()
{
    /* This is not in destructor because it may be called before other
     * deletion code that may need params, stats or timers to exist */
    // cleanup exploration

    close();
    delete settings;
    delete statistics;
    delete timers;
    delete DB;
    delete smem_in_wmem;
    delete smem_wmas;
    delete smem_spreaded_to;
    delete smem_recipient;
    delete smem_recipients_of_source;
    delete smem_context_additions;
    delete smem_context_removals;
    delete smem_edges_to_update;
}
