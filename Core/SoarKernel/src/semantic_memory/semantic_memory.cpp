/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  semantic_memory.cpp
 *
 * =======================================================================
 * Description  :  Various functions for Soar-SMem
 * =======================================================================
 */

#include "semantic_memory.h"

#include "smem_settings.h"
#include "smem_stats.h"
#include "smem_timers.h"
#include "smem_db.h"
#include "smem_structs.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
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

const std::string SMem_Manager::memoryDatabasePath = "file:smem_soar?mode=memory&cache=shared";
const int SMem_Manager::sqlite3Flags =
    SQLite::OPEN_READWRITE      |
    SQLite::OPEN_URI            |
    SQLite::OPEN_NOMUTEX        |
    SQLite::OPEN_CREATE;
const int SMem_Manager::sqlite3Timeout = 1000;

void SMem_Manager::recreateDB(std::string db_path)
{
    if (db_path != memoryDatabasePath)
        db_path = "file:" + db_path + "?cache=shared";

    DB = std::make_shared<SQLite::Database>(db_path, SMem_Manager::sqlite3Flags, SMem_Manager::sqlite3Timeout);
    JobQueue = std::make_shared<sqlite_job_queue>(db_path);
}

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
    Symbol* negquery;
    Symbol* retrieve;
    Symbol* math;
    uint64_t depth;
    bool update_LTI_Links = false;
    symbol_list prohibit;
    symbol_list store;

    enum path_type { blank_slate, cmd_bad, cmd_retrieve, cmd_query, cmd_store_new, cmd_store } path;

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
        if ((new_cue && wme_count))
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
                                ((path == blank_slate) || (path == cmd_query)) &&
                                (query == NIL))

                        {
                            query = (*w_p)->value;
                            path = cmd_query;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_negquery)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate) || (path == cmd_query)) &&
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
                                ((path == blank_slate) || (path == cmd_query)) &&
                                ((*w_p)->value->id->LTI_ID != NIL))
                        {
                            prohibit.push_back((*w_p)->value);
                            path = cmd_query;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_math_query)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate) || (path == cmd_query)) &&
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
                    dprint(DT_SMEM_INSTANCE, "SMem Manager responding to retrieve command.\n");
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
                    dprint(DT_SMEM_INSTANCE, "SMem Manager responding to query command.\n");
                    id_set prohibit_lti;
                    symbol_list::iterator sym_p;

                    for (sym_p = prohibit.begin(); sym_p != prohibit.end(); sym_p++)
                    {
                        prohibit_lti.insert((*sym_p)->id->LTI_ID);
                    }
                    process_query(state, query, negquery, math, prohibit_lti, cue_wmes, meta_wmes, retrieval_wmes, qry_full, nullptr, 1, depth, wm_install, settings->synchronous_db->get_value());

                    // add one to the cbr stat
                    thisAgent->SMem->statistics->queries->set_value(thisAgent->SMem->statistics->queries->get_value() + 1);
                }
                else if (path == cmd_store_new)
                {
                    dprint(DT_SMEM_INSTANCE, "SMem Manager responding to store-new command.\n");
                    symbol_list::iterator sym_p;

                    ////////////////////////////////////////////////////////////////////////////
                    thisAgent->SMem->timers->storage->start();
                    ////////////////////////////////////////////////////////////////////////////

                    // start transaction (if not lazy)
                    if (thisAgent->SMem->settings->lazy_commit->get_value() == off)
                    {
//                        thisAgent->SMem->SQL->begin.exec();
//                        thisAgent->SMem->SQL->begin.reset();
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
//                        thisAgent->SMem->SQL->commit.exec();
//                        thisAgent->SMem->SQL->commit.reset();
                    }

                    ////////////////////////////////////////////////////////////////////////////
                    thisAgent->SMem->timers->storage->stop();
                    ////////////////////////////////////////////////////////////////////////////
                }
                else if (path == cmd_store)
                {
                    dprint(DT_SMEM_INSTANCE, "SMem Manager responding to store command.\n");
                    symbol_list::iterator sym_p;

                    ////////////////////////////////////////////////////////////////////////////
                    thisAgent->SMem->timers->storage->start();
                    ////////////////////////////////////////////////////////////////////////////

                    // start transaction (if not lazy)
                    if (thisAgent->SMem->settings->lazy_commit->get_value() == off)
                    {
//                        thisAgent->SMem->SQL->begin.exec();
//                        thisAgent->SMem->SQL->begin.reset();
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
//                        thisAgent->SMem->SQL->commit.exec();
//                        thisAgent->SMem->SQL->commit.reset();
                    }

                    ////////////////////////////////////////////////////////////////////////////
                    thisAgent->SMem->timers->storage->stop();
                    ////////////////////////////////////////////////////////////////////////////
                }
            }
            else
            {
                add_triple_to_recall_buffer(meta_wmes, state->id->smem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.smem_sym_bad_cmd, state->id->smem_info->cmd_wme->value);
            }

            if (!meta_wmes.empty() || !retrieval_wmes.empty())
            {
                dprint(DT_SMEM_INSTANCE, "SMem Manager installing recall buffer.\n");
                // process preference assertion en masse
                install_recall_buffer(state, cue_wmes, meta_wmes, retrieval_wmes);

                // clear cache
                {
                    symbol_triple_list::iterator mw_it;

                    for (mw_it = retrieval_wmes.begin(); mw_it != retrieval_wmes.end(); mw_it++)
                    {
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->id);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->attr);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->value);

                        delete(*mw_it);
                    }
                    retrieval_wmes.clear();

                    for (mw_it = meta_wmes.begin(); mw_it != meta_wmes.end(); mw_it++)
                    {
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->id);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->attr);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->value);

                        delete(*mw_it);
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

//    auto lockCheckQueryResults = [=]() -> bool {
//        std::lock_guard<std::mutex> lock(agent_jobqueue_boundary_mutex);
//        return query_results.size() > 0;
//    };

    if (query_results.size() > 0)
    {
        meta_wmes.clear();

        std::lock_guard<std::mutex> lock(agent_jobqueue_boundary_mutex);

        while (query_results.size() > 0)
        {
            auto result = query_results.back();
            query_results.pop_back();

            uint64_t king_id = result.king_id;
            uint64_t depth = result.depth;
            Symbol* state = result.state;
            Symbol* query = result.query;
            Symbol* negquery = result.negquery;

            // produce results
            if (king_id != NIL)
            {
                // success!
                add_triple_to_recall_buffer(meta_wmes, state->id->smem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.smem_sym_success, query);
                if (negquery)
                {
                    add_triple_to_recall_buffer(meta_wmes, state->id->smem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.smem_sym_success, negquery);
                }

                ////////////////////////////////////////////////////////////////////////////
                timers->query->stop();
                ////////////////////////////////////////////////////////////////////////////
                install_memory(state, king_id, NIL, (settings->activate_on_query->get_value() == on), meta_wmes, retrieval_wmes, smem_install_type::wm_install, depth);
            }
            else
            {
                add_triple_to_recall_buffer(meta_wmes, state->id->smem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.smem_sym_failure, query);
                if (negquery)
                {
                    add_triple_to_recall_buffer(meta_wmes, state->id->smem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.smem_sym_failure, negquery);
                }

                ////////////////////////////////////////////////////////////////////////////
                timers->query->stop();
                ////////////////////////////////////////////////////////////////////////////
            }

            if (!meta_wmes.empty())
            {
                dprint(DT_SMEM_INSTANCE, "SMem Manager installing recall buffer.\n");
                // process preference assertion en masse
                install_recall_buffer(state, cue_wmes, meta_wmes, retrieval_wmes);

                // clear cache
                {
                    symbol_triple_list::iterator mw_it;

                    for (mw_it = meta_wmes.begin(); mw_it != meta_wmes.end(); mw_it++)
                    {
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->id);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->attr);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->value);

                        delete(*mw_it);
                    }
                    meta_wmes.clear();
                }
                
                // process wm changes on this state
                do_wm_phase = true;
            }
        }
    }

    if (do_wm_phase)
    {
        dprint(DT_SMEM_INSTANCE, "SMem Manager initiating working memory phase.\n");
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

        if (pref->in_tm)
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
    if (thisAgent->SMem->connected())
    {
        if (thisAgent->SMem->settings->append_db->get_value() == off)
        {
            close();
            init_db();
        }
    }
}

SMem_Manager::SMem_Manager(agent* myAgent)
: thisAgent(myAgent),
settings(new smem_param_container(myAgent)),
statistics(new smem_stat_container(myAgent))
{
    thisAgent->SMem = this;

    timers = new smem_timer_container(thisAgent);

    smem_validation = 0;
};

void SMem_Manager::clean_up_for_agent_deletion()
{
    /* This is not in destructor because it may be called before other
     * deletion code that may need params, stats or timers to exist */
    // cleanup exploration

    try {
    close();
    } catch (...) {
        std::exception_ptr e = std::current_exception();

        try
        {
            if (e)
                std::rethrow_exception(e);
        } catch (std::exception& e)
        {
            std::cerr << "Exception thrown while cleaning up for agent deletion.  Database might be in inconsistent state:" << e.what() << std::endl;
        }
    }
    delete settings;
    delete statistics;
    delete timers;
}
