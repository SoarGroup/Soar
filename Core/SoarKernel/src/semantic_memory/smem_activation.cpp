/*
 * smem_activation.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#include "semantic_memory.h"

#include "smem_settings.h"
#include "smem_db.h"
#include "smem_stats.h"
#include "smem_timers.h"
#include "working_memory_activation.h"
#include "working_memory.h"

#include "VariadicBind.h"
#include "guard.hpp"

double SMem_Manager::lti_calc_base(uint64_t pLTI_ID, int64_t time_now, uint64_t n, uint64_t activations_first)
{
    double sum = 0.0;
    double d = settings->base_decay->get_value();
    uint64_t t_k;
    uint64_t t_n = (time_now - activations_first);
    uint64_t small_n;
    int64_t recent_time;
    unsigned int available_history = 0;

    std::packaged_task<uint64_t()> pt_getActivationsTotal([this,pLTI_ID]{
        auto sql = sqlite_thread_guard(SQL->lti_access_get);

        sql->bind(1, pLTI_ID);

        if (!sql->executeStep())
            throw SoarAssertionException("Failed to retrieve column", __FILE__, __LINE__);

        return sql->getColumn(0).getUInt64();
       // activations_first = sql->getColumn(2).getInt();
    });

    if (n == 0)
        n = JobQueue->post(pt_getActivationsTotal).get();

    std::packaged_task<std::tuple<uint64_t,double,uint64_t,int64_t>()> pt_getAllHistory([this,pLTI_ID,time_now,d,n] {
        auto sql = sqlite_thread_guard(SQL->history_get);

        sql->bind(1, pLTI_ID);

        uint64_t t_k = 0;
        double sum = 0;
        uint64_t small_n = 0;
        uint64_t recent_time = 0;

        if (sql->executeStep())
    {
            uint64_t available_history = 0;

            for (int i = 0;i < SMEM_ACT_HISTORY_ENTRIES;++i)
                if (sql->getColumn(i).getUInt64() != 0)
                    ++available_history;
            small_n = available_history;

            t_k = uint64_t(time_now - sql->getColumn(int(available_history - 1)).getInt64());

            for (uint64_t i = 0; i < available_history; i++)
        {
                int64_t time_diff = time_now - sql->getColumn(int(i)).getInt64();

            if (i == 0 && n > 0)
                recent_time = time_diff;

                sum += pow(double(time_diff),
                           double(-d));
            }
        }

        return std::make_tuple(t_k, sum, small_n, recent_time);
    });

    // get all history
    auto t = JobQueue->post(pt_getAllHistory).get();
    t_k = std::get<0>(t);
    sum = std::get<1>(t);
    small_n = std::get<2>(t);
    recent_time = std::get<3>(t);

    // if available history was insufficient, approximate rest
    if (n > small_n && available_history == SMEM_ACT_HISTORY_ENTRIES)
    {
        if (t_n != t_k)
        {
            double apx_numerator = (double(n - SMEM_ACT_HISTORY_ENTRIES) * (pow(double(t_n), 1.0 - d) - pow(double(t_k), 1.0 - d)));
            double apx_denominator = ((1.0 - d) * double(t_n - t_k));

            sum += (apx_numerator / apx_denominator);
        }
        else
            sum += (n - small_n) * pow(double(t_n), double(-d));
        }

    double inhibition_odds = 0;
    if (recent_time != 0 && settings->base_inhibition->get_value() == on )// && smem_in_wmem->find(pLTI_ID) != smem_in_wmem->end())
    {
        inhibition_odds = pow(1+pow(double(recent_time) / 10.0,-1.0),-1.0);
        return ((sum > 0) ? (log(sum/(1+sum)) + log(inhibition_odds/(1+inhibition_odds))) : (SMEM_ACT_LOW));
    }

    return ((sum > 0) ? (log(sum/(1+sum))) : (SMEM_ACT_LOW));
}

// activates a new or existing long-term identifier
// note: optional num_edges parameter saves us a lookup
//       just when storing a new chunk (default is a
//       big number that should never come up naturally
//       and if it does, satisfies thresholding behavior).
double SMem_Manager::lti_activate(uint64_t pLTI_ID, bool add_access, uint64_t num_edges, double touches, bool increment_timer)
{
    ////////////////////////////////////////////////////////////////////////////
    timers->act->start();
    ////////////////////////////////////////////////////////////////////////////

    // access information
    double prev_access_n = 0;
    uint64_t prev_access_t = 0;
    uint64_t prev_access_1 = 0;

    // get old (potentially useful below)
    typedef std::tuple<double,uint64_t,uint64_t> prev_tuple;
    std::packaged_task<prev_tuple()> prev([this,pLTI_ID]
    {
        prev_tuple result = std::make_tuple(0,0,0);

        {
            auto sql = sqlite_thread_guard(SQL->lti_access_get);

            sql->bind(1, pLTI_ID);

            if (!sql->executeStep())
                throw SoarAssertionException("Failed to retrieve column", __FILE__, __LINE__);

            result = std::make_tuple(sql->getColumn(0).getDouble(), /* prev_access_n */
                                     sql->getColumn(1).getUInt64(), /* prev_access_t */
                                     sql->getColumn(2).getUInt64()  /* prev_access_1 */);

            // DID: SQLITE_LOCKED
            //
            // You will get SQLITE_LOCKED if you do not include this.
            // There is a read-write conflict between this thread
            // and the thread the add_access job is called on
            // which will be unable to be resolved until this
            // statement goes out of scope, except that it won't
            // because it is waiting on that job to finish.  Aka
            // you get an infinite loop if you try to use
            // unlock_notify and you get SQLITE_LOCKED if you don't.
            //
            // TL;DR: reset any statements before writes to prevent
            // read-write, write-read, write-write conflicts.
            //sql->reset();
        }

        return result;
    });

    auto prev_t = JobQueue->post(prev).get();
    prev_access_n = std::get<0>(prev_t);
    prev_access_t = std::get<1>(prev_t);
    prev_access_1 = std::get<2>(prev_t);

    bool prohibited;

    int64_t time_now;
    if (add_access)
    {
        if (increment_timer)
        {
            time_now = smem_max_cycle++;
        }
        else
        {
            time_now = smem_max_cycle - 1;
        }

        /*
         * Prohibits are set up such that the only thing we need to do is flip the prohibit bit and the normal
         * updating behavior should take care of most things (with the exception of what we expressly do under the prohibit
         * tests below..
         */
        std::packaged_task<std::tuple<bool,bool>()> pt_ProhibitCheck([this, pLTI_ID] {
            auto sql = sqlite_thread_guard(SQL->prohibit_check);

            sql->bind(1, pLTI_ID);

            if (sql->executeStep())
                return std::make_tuple(true, sql->getColumn(1).getInt64() == true);
            else
                return std::make_tuple(false, false);
        });

        auto t = JobQueue->post(pt_ProhibitCheck).get();
        prohibited = std::get<0>(t);
        bool dirty = std::get<1>(t);

        if (prohibited)
        {
            //Find the number of touches from the most recent activation and remove that much touching.
            if (dirty)
            {
                std::packaged_task<double()> pt_historyRemove([this, pLTI_ID] {
                    double prevN = 0;

                    {
                        auto sql = sqlite_thread_guard(SQL->history_get);

                        sql->bind(1, pLTI_ID);

                        if (sql->executeStep())
                            prevN = sql->getColumn(SMEM_ACT_HISTORY_ENTRIES).getUInt64();
                    }

                    {
                        auto sql = sqlite_thread_guard(SQL->history_remove);
                        sql->bind(1, pLTI_ID);
                        sql->exec();
                    }

                    return prevN;
                });

                prev_access_n -= JobQueue->post(pt_historyRemove).get();
            }

            std::packaged_task<void()> pt_prohibitReset([this, pLTI_ID] {
                auto sql = sqlite_thread_guard(SQL->prohibit_reset);
                sql->bind(1, pLTI_ID);
                sql->exec();
            });

            JobQueue->post(pt_prohibitReset).wait();
        }

        if ((settings->activation_mode->get_value() == smem_param_container::act_base) &&
                (settings->base_update->get_value() == smem_param_container::bupt_incremental))
        {
            int64_t time_diff;

            for (std::set< int64_t >::iterator b = settings->base_incremental_threshes->set_begin(); b != settings->base_incremental_threshes->set_end(); b++)
            {
                if (*b > 0)
                {
                    time_diff = (time_now - *b);

                    if (time_diff > 0)
                    {
                        std::packaged_task<void()> pt_activateLTIs([this,time_diff]{
                            auto sql = sqlite_thread_guard(SQL->lti_get_t);

                            sql->bind(1, time_diff);

                            std::vector<uint64_t> ltis;
                            while (sql->executeStep())
                                ltis.push_back(sql->getColumn(0).getUInt64());

                            sql->reset();

                            for (uint64_t lti : ltis)
                                lti_activate(lti, false);
                        });

                        JobQueue->post(pt_activateLTIs).wait();
                        }
                    }
                }
            }

        statistics->act_updates->set_value(statistics->act_updates->get_value() + 1);
    }
    else
    {
        std::packaged_task<std::tuple<bool,bool>()> pt_ProhibitCheck([this, pLTI_ID] {
            auto sql = sqlite_thread_guard(SQL->prohibit_check);

            sql->bind(1, pLTI_ID);

            if (sql->executeStep())
                return std::make_tuple(true, sql->getColumn(1).getInt64() == true);
            else
                return std::make_tuple(false, false);
        });

        auto t = JobQueue->post(pt_ProhibitCheck).get();
        prohibited = std::get<0>(t);
        bool dirty = std::get<1>(t);

        if (prohibited && dirty)
        {
            std::packaged_task<double()> pt_historyRemove([this, pLTI_ID] {
                double prevN = 0;

                {
                    auto sql = sqlite_thread_guard(SQL->history_get);

                    sql->bind(1, pLTI_ID);

                    if (sql->executeStep())
                        prevN = sql->getColumn(SMEM_ACT_HISTORY_ENTRIES).getUInt64();
                }

        {
                    auto sql = sqlite_thread_guard(SQL->history_remove);
                    sql->bind(1, pLTI_ID);
                    sql->exec();
        }

        {
                    auto sql = sqlite_thread_guard(SQL->prohibit_clean);
                    sql->bind(1, pLTI_ID);
                    sql->exec();
                }

                return prevN;
            });

            prev_access_n -= JobQueue->post(pt_historyRemove).get();
        }

        time_now = smem_max_cycle;
        statistics->act_updates->set_value(statistics->act_updates->get_value() + 1);
    }

    std::packaged_task<void()> updateLTIAccess([this, prev_access_n, add_access, touches, time_now, prev_access_t, prev_access_1, pLTI_ID] {
        auto sql = sqlite_thread_guard(SQL->lti_access_set);

        sql->bind(1, (prev_access_n + (add_access ? touches : 0.0)));
        sql->bind(2, add_access ? time_now : prev_access_t);
        sql->bind(3, prev_access_n == 0 ? (add_access ? time_now : 0) : prev_access_1);
        sql->bind(4, pLTI_ID);

        sql->exec();
    });

    JobQueue->post(updateLTIAccess).wait();

    // get new activation value (depends upon bias)
    double new_activation = 0.0;
    smem_param_container::act_choices act_mode = settings->activation_mode->get_value();
    if (act_mode == smem_param_container::act_recency)
    {
        new_activation = static_cast<double>(time_now);
    }
    else if (act_mode == smem_param_container::act_frequency)
    {
        new_activation = static_cast<double>(prev_access_n + ((add_access) ? (1) : (0)));
    }
    else if (act_mode == smem_param_container::act_base)
    {

        if (prev_access_1 == 0)
        {
            if (add_access)
            {
                if (prohibited)
                {
                    std::packaged_task<void()> pt_push([this,pLTI_ID,time_now, touches] {
                        auto sql = sqlite_thread_guard(SQL->history_push);

                        sql->bind(1, time_now);
                        sql->bind(2, touches);
                        sql->bind(3, pLTI_ID);

                        sql->exec();
                    });

                    JobQueue->post(pt_push).wait();
                }
                else
                {
                    std::packaged_task<void()> pt_add([this,pLTI_ID,time_now, touches] {
                        auto sql = sqlite_thread_guard(SQL->history_add);

                        sql->bind(1, pLTI_ID);
                        sql->bind(2, touches);
                        sql->bind(3, pLTI_ID);

                        sql->exec();
                    });

                    JobQueue->post(pt_add).wait();
                }
            }
            new_activation = lti_calc_base(pLTI_ID, time_now + ((add_access) ? (1) : (0)), prev_access_n + ((add_access) ? (touches) : (0)), prev_access_1);
        }
        else
        {
            if (add_access)
            {
                std::packaged_task<void()> pt_push([this,pLTI_ID,time_now, touches] {
                    auto sql = sqlite_thread_guard(SQL->history_push);

                    sql->bind(1, time_now);
                    sql->bind(2, touches);
                    sql->bind(3, pLTI_ID);

                    sql->exec();
                });

                JobQueue->post(pt_push).wait();
            }
            new_activation = lti_calc_base(pLTI_ID, time_now + ((add_access) ? (1) : (0)), prev_access_n + (add_access ? touches : 0), prev_access_1);
        }
    }

    // get number of augmentations (if not supplied)
    if (num_edges == SMEM_ACT_MAX)
    {
        std::packaged_task<uint64_t()> edges([this,pLTI_ID] {
            auto sql = sqlite_thread_guard(SQL->act_lti_child_ct_get);

            sql->bind(1, pLTI_ID);

            if (!sql->executeStep())
                throw SoarAssertionException("Failed to retrieve column", __FILE__, __LINE__);

            return sql->getColumn(0).getUInt64();
        });

        num_edges = JobQueue->post(edges).get();
    }

    // need a denominator for spreading:
    double baseline_denom = settings->spreading_continue_probability->get_value();
    double k_decay = baseline_denom;
    uint64_t depth_limit = settings->spreading_depth_limit->get_value();
    for (uint64_t i = 0;i < depth_limit;++i)
    {
        // representing a maximum possible spreading value for comparison.  It's an admissible heuristic. (fancy word!)
        baseline_denom = baseline_denom + baseline_denom * k_decay;
    }

    double spread = 0, modified_spread = 0, new_base = 0, additional = 0;
    bool already_in_spread_table = false;
    std::unordered_map<uint64_t, uint64_t>* spreaded_to = smem_spreaded_to;

    if (settings->spreading->get_value() == on &&
        spreaded_to->find(pLTI_ID) != spreaded_to->end() &&
        (*spreaded_to)[pLTI_ID] != 0)
    {
        already_in_spread_table = true;

        std::packaged_task<double()> pt_actLTIFake([this,pLTI_ID] {
            auto sql = sqlite_thread_guard(SQL->act_lti_fake_get);
            sql->bind(1, pLTI_ID);

            if (sql->executeStep())
            {
                return sql->getColumn(1).getDouble();
            }

            return 0.0;
        });

        spread = JobQueue->post(pt_actLTIFake).get();
    }

    if (new_activation == SMEM_ACT_LOW || new_activation == 0.0)
    {
        double decay = settings->base_decay->get_value();
        new_base = pow(static_cast<double>(smem_max_cycle+settings->base_unused_age_offset->get_value()),static_cast<double>(-decay));
        new_base = log(new_base/(1.0+new_base));
    }
    else
    {
        new_base = new_activation;
    }

    if (already_in_spread_table)
    {
        double offset = settings->spreading_baseline->get_value()/baseline_denom;
        modified_spread = (spread == 0 || spread < offset) ? 0.0 : (log(spread) - log(offset));
        //modified_spread = log(spread) - log(offset);
        //spread = ((spread < offset) ? 0.0 : spread);
        //if (new_activation == SMEM_ACT_LOW)
        //    bool test = true;

        std::packaged_task<void()> pt_fake([this, pLTI_ID, new_activation, spread, new_base, modified_spread] {
            auto sql = sqlite_thread_guard(SQL->act_lti_fake_set);

            sql->bind(1, new_activation);
            sql->bind(2, spread);
            sql->bind(3, new_base + modified_spread);
            sql->bind(4, pLTI_ID);

            sql->exec();
        });

        JobQueue->post(pt_fake).wait();    }

    //SQL->act_set->bind_double(1, SMEM_ACT_LOW);
    //SQL->act_set->bind_int(2, pLTI_ID);
    //SQL->act_set->execute(soar_module::op_reinit);

    else
    {
        /* MMerge | The equivalent version of this code is commented out in async */
        std::packaged_task<void()> pt_ltiSet([this, pLTI_ID, new_activation, new_base] {
            auto sql = sqlite_thread_guard(SQL->act_lti_set);

            sql->bind(1, new_activation);
            sql->bind(2, 0.0);
            sql->bind(3, new_base);
            sql->bind(4, pLTI_ID);

            sql->exec();
        });

        JobQueue->post(pt_ltiSet).wait();
    }

    // only if augmentation count is less than threshold do we associate with edges
    if (num_edges < static_cast<uint64_t>(settings->thresh->get_value()) && !already_in_spread_table)
    {
        // activation_value=? WHERE lti=?
        std::packaged_task<void()> set([this,new_base,modified_spread,pLTI_ID] {
            auto sql = sqlite_thread_guard(SQL->act_set);

            sql->bind(1, new_base + modified_spread);
            sql->bind(2, pLTI_ID);

            sql->exec();
        });

        JobQueue->post(set).wait();
    }
    else if (num_edges < static_cast<uint64_t>(settings->thresh->get_value()) && already_in_spread_table)
    {
        //SQL->act_set->bind_double(1, SMEM_ACT_LOW);
        //SQL->act_set->bind_int(2, pLTI_ID);
        //SQL->act_set->execute(soar_module::op_reinit);
    }

    ////////////////////////////////////////////////////////////////////////////
    timers->act->stop();
    ////////////////////////////////////////////////////////////////////////////

    return new_base + modified_spread;
}

void SMem_Manager::child_spread(uint64_t lti_id, std::map<uint64_t, std::list<std::pair<uint64_t,double>>*>& lti_trajectories, int depth = 10)
{
    if (lti_trajectories.find(lti_id) == lti_trajectories.end())
    {//If we don't already have the children and their edge weights, we need to get them.
        //soar_module::sqlite_statement* children_q = SQL->web_val_child;
        std::list<uint64_t> children;

        //First, we don't bother changing edge weights unless we have changes with which to update the edge weights.
        if (smem_edges_to_update->find(lti_id) != smem_edges_to_update->end())
        {
            bool first_time = true;// The first set of weights comes from the db store.

            std::map<uint64_t, double> old_edge_weight_map_for_children;
            std::map<uint64_t, double> edge_weight_update_map_for_children;
            std::list<smem_edge_update*>* edge_updates = &(smem_edges_to_update->find(lti_id)->second);
            uint64_t time;
            uint64_t previous_time;
            double total_touches = 0;
            std::list<smem_edge_update*>::iterator edge_begin_it = edge_updates->begin();
            std::list<smem_edge_update*>::iterator edge_it;
            bool prohibited = false;
            double edge_update_decay = thisAgent->SMem->settings->spreading_edge_update_factor->get_value();//.99;
            for (edge_it = edge_begin_it; edge_it != edge_updates->end(); ++edge_it)
            {
                time = (*edge_it)->update_time;
                if (time != previous_time && !first_time)
                {//We need to compile the edge weight changes for the previous timestep before moving on to the next timestep.
                    std::map<uint64_t,double>::iterator updates_begin = old_edge_weight_map_for_children.begin();
                    std::map<uint64_t,double>::iterator updates_it;
                    double normalizing_sum = 0;
                    for (updates_it = updates_begin; updates_it != old_edge_weight_map_for_children.end(); ++updates_it)
                    {//We are looping through the update map and inserting those updates into the old edge weight map.
                        if (edge_weight_update_map_for_children.find(updates_it->first) == edge_weight_update_map_for_children.end())
                        {//If we don't have an update for that edge, we just decrease it.
                            old_edge_weight_map_for_children[updates_it->first] = pow(edge_update_decay,total_touches)*old_edge_weight_map_for_children[updates_it->first];
                            normalizing_sum += old_edge_weight_map_for_children[updates_it->first];
                        }
                        else
                        {//If we do have an update, we adjust.
                            old_edge_weight_map_for_children[updates_it->first] = old_edge_weight_map_for_children[updates_it->first] + edge_weight_update_map_for_children[updates_it->first];
                            normalizing_sum += old_edge_weight_map_for_children[updates_it->first];
                        }
                    }
                    for (updates_it = updates_begin; updates_it != old_edge_weight_map_for_children.end(); ++updates_it)
                    {
                        old_edge_weight_map_for_children[updates_it->first] = old_edge_weight_map_for_children[updates_it->first]/normalizing_sum;
                    }
                    edge_weight_update_map_for_children.clear();
                    total_touches = 0;
                }
                if (first_time)
                {//this is where we extract the old edge weights from the database store.
                    first_time = false;
                    /*children_q->bind_int(1, lti_id);
                    //children_q->bind_int(2, lti_id);
                    while (children_q->execute() == soar_module::row)
                    {
                        if (settings->spreading_loop_avoidance->get_value() == on && children_q->column_int(0) == lti_id)
                        {
                            continue;
                        }
                        old_edge_weight_map_for_children[(uint64_t)(children_q->column_int(0))] = children_q->column_double(1);
                        edge_weight_update_map_for_children[(uint64_t)(children_q->column_int(0))] = 0;
                    }
                    children_q->reinitialize();*/

                    {
                        auto sql = sqlite_thread_guard(SQL->web_val_child);

                        sql->bind(1, lti_id);
                        sql->bind(2, lti_id);
                        while(sql->executeStep())
                        {
                            if (settings->spreading_loop_avoidance->get_value() == on && sql->getColumn(0).getUInt64())
                            {
                                continue;
                            }
                        }
                        old_edge_weight_map_for_children[sql->getColumn(0).getUInt64()] = sql->getColumn(1).getDouble();
                        edge_weight_update_map_for_children[sql->getColumn(0).getUInt64()] = 0;
                    }

                    //JobQueue->post(pt_children_query).wait();
                    
                }
                uint64_t child = (*edge_it)->lti_edge_id;
                double touches = (*edge_it)->num_touches;
                total_touches+=touches;
                /*SQL->prohibit_check->bind_int(1, child);
                prohibited = SQL->prohibit_check->execute()==soar_module::row;
                bool dirty = false;
                if (prohibited)
                {
                    dirty = SQL->prohibit_check->column_int(1)==1;
                }
                SQL->prohibit_check->reinitialize();*/
                //This is where the updates are actually collected for touches from working memory.
                for (int touch_ct = 1; touch_ct <= touches; ++touch_ct)
                {
                    if (edge_weight_update_map_for_children.find(child) != edge_weight_update_map_for_children.end())
                    {
                        edge_weight_update_map_for_children[child] = edge_weight_update_map_for_children[child] + (1.0-old_edge_weight_map_for_children[child])*(1.0-edge_update_decay)*pow(edge_update_decay,touch_ct-1.0);
                    }
                    else
                    {
                        edge_weight_update_map_for_children[child] = 0.0 + (1.0 - old_edge_weight_map_for_children[child])*(1.0-edge_update_decay);
                    }
                }
                previous_time = time;
            }
            std::map<uint64_t,double>::iterator final_updates_begin = old_edge_weight_map_for_children.begin();
            std::map<uint64_t,double>::iterator final_updates_it;
            double normalizing_sum = 0;
            for (final_updates_it = final_updates_begin; final_updates_it != old_edge_weight_map_for_children.end(); ++final_updates_it)
            {//We are looping through the update map and inserting those updates into the old edge weight map.
                if (edge_weight_update_map_for_children.find(final_updates_it->first) == edge_weight_update_map_for_children.end())
                {//If we don't have an update for that edge, we just decrease it.
                    old_edge_weight_map_for_children[final_updates_it->first] = pow(edge_update_decay,total_touches)*old_edge_weight_map_for_children[final_updates_it->first];
                    normalizing_sum += old_edge_weight_map_for_children[final_updates_it->first];
                }
                else
                {//If we do have an update, we adjust.
                    old_edge_weight_map_for_children[final_updates_it->first] = old_edge_weight_map_for_children[final_updates_it->first] + edge_weight_update_map_for_children[final_updates_it->first];
                    normalizing_sum += old_edge_weight_map_for_children[final_updates_it->first];
                }
            }
            for (final_updates_it = final_updates_begin; final_updates_it != old_edge_weight_map_for_children.end(); ++final_updates_it)
            {
                old_edge_weight_map_for_children[final_updates_it->first] = old_edge_weight_map_for_children[final_updates_it->first]/normalizing_sum;
            }
            edge_weight_update_map_for_children.clear();
            //This is the point at which the final timestamp's updates should be applied and then we should write those to the db and then we should clear out the malloc'd (new) updates.
            //We use a new sqlite command that updates an existing edge with a new value for the edge weight. We loop over all edges in the old edge weight map for children for the vals.
            //After the loop of commits to the table, we then loop over thge original updates map attached to the agent to do the deletions (frees).
            std::map<uint64_t,double>::iterator updates_begin = old_edge_weight_map_for_children.begin();
            std::map<uint64_t,double>::iterator updates_it;
            double update_sum = 0;
            //soar_module::sqlite_statement* update_edge = SQL->web_update_child_edge;
            for (updates_it = updates_begin; updates_it != old_edge_weight_map_for_children.end(); ++updates_it)
            {// args are edge weight, parent lti it, child lti id.
                std::packaged_task<void()> pt_update_edge([this, lti_id, updates_it] {
                    auto sql = sqlite_thread_guard(SQL->web_update_child_edge);

                    sql->bind(1, updates_it->second);
                    sql->bind(2, lti_id);
                    sql->bind(3, updates_it->first);
                    sql->exec();
                });
                JobQueue->post(pt_update_edge).wait();
            }
            for (edge_it = edge_begin_it; edge_it != edge_updates->end(); ++edge_it)
            {
                delete (*edge_it);
            }
            smem_edges_to_update->erase(lti_id);
        }
        /*children_q->bind_int(1, lti_id);
        //children_q->bind_int(2, lti_id);
        lti_trajectories[lti_id] = new std::list<std::pair<uint64_t, double>>;
        while (children_q->execute() == soar_module::row)
        {
            //if (children_q->column_int(0) == lti_id)
            //{
            //    continue;
            //}
            (lti_trajectories[lti_id])->push_back(std::make_pair((uint64_t)(children_q->column_int(0)),children_q->column_double(1)));
            children.push_back(children_q->column_int(0));
        }
        children_q->reinitialize();*/

        {
            auto sql = sqlite_thread_guard(SQL->web_val_child);

            sql->bind(1, lti_id);
            sql->bind(2, lti_id);
            while(sql->executeStep())
            {
                if (settings->spreading_loop_avoidance->get_value() == on && sql->getColumn(0).getUInt64())
                {
                    continue;
                }
            }
            (lti_trajectories[lti_id])->push_back(std::make_pair(sql->getColumn(0).getUInt64(),sql->getColumn(1).getDouble()));
            children.push_back(sql->getColumn(0).getUInt64());
        }

        //JobQueue->post(pt_children_query_again).wait();

        if (depth > 1)
        {
            for (std::list<uint64_t>::iterator child_iterator = children.begin(); child_iterator != children.end(); ++child_iterator)
            {
                child_spread(*child_iterator, lti_trajectories, depth-1);
            }
        }
    }
}

void SMem_Manager::trajectory_construction(uint64_t lti_id, std::map<uint64_t, std::list<std::pair<uint64_t, double>>*>& lti_trajectories, int depth = 0, bool initial = false)
{
    //If this isn't the initial formation of the trajectories for this lti, we should get rid of the old trajectory
    if (!initial)
    {
        auto sql = sqlite_thread_guard(SQL->trajectory_remove_lti);
        sql->bind(1,lti_id);
        sql->exec();
    }
    //The way the traversal is managed is by keeping track of where the largest amount of leftover spread still is.
    //We prioritize visiting those places first.
    smem_prioritized_lti_traversal_queue lti_traversal_queue;
    /* I might make this better later, but for now, I wanted to do a prioritized network traversal and I wanted to keep track of the
     * path, so I have a queue of lists. */
    bool ever_added = false;
    uint64_t current_lti;
    uint64_t depth_limit = settings->spreading_depth_limit->get_value();
    uint64_t limit = settings->spreading_limit->get_value();
    uint64_t count = 0;
    std::list<std::pair<uint64_t, double>>* current_lti_list = new std::list<std::pair<uint64_t, double>>();
    //We initialize this with the given lti and a total initial activation weight of 1. Changing of the total weight from this source
    //can be done at time of application with a scalar.
    current_lti_list->push_back(std::make_pair(lti_id, 1.0));
    double initial_activation = 1.0;
    lti_traversal_queue.push(std::make_pair(initial_activation,current_lti_list));
    //The prioritized traversal has now been initialized with a single path consisting of only the source of activation.
    //There is a limit to the size of the stored info.
    double decay_prob = settings->spreading_continue_probability->get_value();
    //The decay_prob is a measure of how much decay we get during traversal along the edges.
    double baseline_prob = settings->spreading_baseline->get_value();
    //The baseline prob is a measure of how low spreading gets before it is ignored
    //(This is a deterministic version of "lost in the noise").
    //TODO: It may be that we want two baselines - one for comparing to BLA and one for the purpose here.

    std::list<std::pair<uint64_t, double>>::iterator lti_iterator;
    std::list<std::pair<uint64_t, double>>::iterator lti_begin;
    std::list<std::pair<uint64_t, double>>::iterator lti_end;
    std::list<std::pair<uint64_t, double>>::iterator old_list_iterator;
    std::list<std::pair<uint64_t, double>>::iterator old_list_iterator_begin;
    std::list<std::pair<uint64_t, double>>::iterator old_list_iterator_end;
    std::list<std::pair<uint64_t, double>>::iterator new_list_iterator;
    std::list<std::pair<uint64_t, double>>::iterator new_list_iterator_begin;
    std::list<std::pair<uint64_t, double>>::iterator new_list_iterator_end;

    bool good_lti = true;
    depth = 0;
    //uint64_t fan_out;
    std::map<uint64_t, double> spread_map;
    //This map is the amount of spreading activation a recipient accumulates
    //from the source throughout the traversal.
    while (!lti_traversal_queue.empty() && count < limit)
    {//while we have network left to traversal and we haven't hit our computational limit.
        //Find the children of the current_lti_id.
        current_lti_list = lti_traversal_queue.top().second;
        //depth = current_lti_list->size();
        current_lti = current_lti_list->back().first;
        //We pick up this path where we left off.
        child_spread(current_lti, lti_trajectories, 1);//TODO: to make compatible with DBs that have huge-fan: only retrieve children above a threshold edge weight..
        //We get the children of the node we left off at if we don't already have its children.
        lti_begin = lti_trajectories[current_lti]->begin();//first child;
        lti_end = lti_trajectories[current_lti]->end(); //last child;
        old_list_iterator_begin = current_lti_list->begin();//The beginning of the path we were on.
        old_list_iterator_end = current_lti_list->end();//Where we left off in that path.
        //Why the above two? we're going to make a deep copy later.
        initial_activation = decay_prob*(lti_traversal_queue.top().first);//We always decay with depth.
        lti_traversal_queue.pop();//Get rid of the old list.
        for (lti_iterator = lti_begin; lti_iterator != lti_end && count < limit && initial_activation > baseline_prob; ++lti_iterator)
        {
            std::set<std::pair<uint64_t, double>> visited;
            //First, we make a new copy of the list for the next step of the traversal.
            std::list<std::pair<uint64_t,double>>* new_list = new std::list<std::pair<uint64_t,double>>();
            for (old_list_iterator = old_list_iterator_begin; old_list_iterator != old_list_iterator_end; ++old_list_iterator)
            {//looping over the contents of the old list and copying.
                new_list->push_back((*old_list_iterator));
                if (settings->spreading_loop_avoidance->get_value() == on)
                {
                    visited.insert((*old_list_iterator));//We we have loop avoidance on, we need to keep track of the old path elements.
                }
            }
            good_lti = true;
            if (new_list->empty())
            {
                good_lti = false;
            }
            if (lti_iterator->first == 0 || lti_iterator->second < baseline_prob)
            {
                good_lti = false;
            }
            if (settings->spreading_loop_avoidance->get_value() == on)
            {
                good_lti = (visited.find(*lti_iterator) == visited.end());
            }//If we've already seen this child earlier in the list, it's no good (if loop avoidance is on)
            statistics->trajectories_total->set_value(statistics->trajectories_total->get_value()+1);
            if (!good_lti)
            {
                delete new_list;//We don't need the copy.
                new_list = NULL;
                continue;//We don't need to do processing because we're skipping this already visited child.
            }
            //Add the new good lti to the list
            new_list->push_back((*lti_iterator));
            if (spread_map.find(lti_iterator->first) == spread_map.end())
            {//If we haven't already given some spread to this recipient from this source, we have to start with an initial contribution
                spread_map[lti_iterator->first] = initial_activation*lti_iterator->second;
            }
            else
            {//Otherwise, we just continue to accumulate.
                spread_map[lti_iterator->first] = spread_map[lti_iterator->first] + initial_activation*lti_iterator->second;
            }
            //In both above cases, we multiplied the activation by the edge weight. (lti iterator has children and their edge weights)
            //Now that we've done the activation for this additional step, we add the path.
            new_list_iterator_begin = new_list->begin();
            new_list_iterator_end = new_list->end();
            depth = 0;
            {
                auto sql = sqlite_thread_guard(SQL->trajectory_add);
            for (new_list_iterator = new_list_iterator_begin; new_list_iterator != new_list_iterator_end && depth < (depth_limit + 2); ++new_list_iterator)
            {
                    sql->bind(++depth, new_list_iterator->first);
            }//We add the amount of traversal we have.
            while (depth < 11)
            {//And we pad unused columns with 0. This helps the indexing ignore these columns later. I could maybe do the same with NULL.
                //It depends on the specifics of partial indexing in sqlite... Point is - I know this works for that efficiency gain.
                    sql->bind(++depth, 0);
                }
                sql->exec();
            }
            //For later use, this is bookkeeping:
            ever_added = true;
            ++count;
            //If we still have room for more, we add the new path to the p-queue for additional traversal.
            if (new_list->size() < depth_limit + 1 && count < limit &&  decay_prob*initial_activation*lti_iterator->second > baseline_prob)
            {//if we aren't at the depth limit, the total traversal size limit, and the activation is big enough
                lti_traversal_queue.push(std::make_pair(initial_activation,new_list));
            }
            else
            {
                delete new_list;//No need to keep the copy+1more otherwise.
                new_list = NULL;
            }
        }
        /* MMerge | Was commented out in development, but not in async */
        lti_traversal_queue.pop();//Get rid of the old list.
        delete current_lti_list;//no longer need it.
    }
    //Once we've generated the full spread map of accumulated spread for recipients from this source, we record it.
    for (std::map<uint64_t,double>::iterator spread_map_it = spread_map.begin(); spread_map_it != spread_map.end(); ++spread_map_it)
    {
        auto sql = sqlite_thread_guard(SQL->likelihood_cond_count_insert);
        sql->bind(1,lti_id);
        sql->bind(2,spread_map_it->first);
        sql->bind(3,spread_map_it->second);
        sql->exec();
    }
    //In the special case where we don't ever add anything, we need to insert all zeros as the traversal.
    if (!ever_added)
    {
        auto sql = sqlite_thread_guard(SQL->trajectory_add);
        sql->bind(1,lti_id);
        sql->bind(2,0);
        sql->bind(3,0);
        sql->bind(4,0);
        sql->bind(5,0);
        sql->bind(6,0);
        sql->bind(7,0);
        sql->bind(8,0);
        sql->bind(9,0);
        sql->bind(10,0);
        sql->bind(11,0);
        sql->bind(12,1);
        sql->exec();
    }
    //cleaning up
    while (!lti_traversal_queue.empty())
    {
        delete (lti_traversal_queue.top().second);
        lti_traversal_queue.pop();
    }
}

void SMem_Manager::calc_spread_trajectories()
{
    std::packaged_task<void()> pt_calcTrajectories([this] {
        attach();
        auto lti_all = sqlite_thread_guard(SQL->lti_all);
        uint64_t lti_id;
        int j = 0;
        //smem_delete_trajectory_indices();//This is for efficiency.
        //It's super inefficient to maintain the database indexing during this batch processing
        //It's way better to delete and rebuild. However, for testing and small DBs, it's fine. I'm testing... so... it's commented for now.
        // - scijones (Yell at me if you see this.)
        double p1 = settings->spreading_continue_probability->get_value();
        std::map<uint64_t, std::list<std::pair<uint64_t, double>>*> lti_trajectories;
        while (lti_all->executeStep())
        {//loop over all ltis.
            lti_id = lti_all->getColumn(0).getUInt64();
            trajectory_construction(lti_id,lti_trajectories,0,true);
        }

        //smem_create_trajectory_indices();//TODO: Fix this and the above commend about it. YELL AT ME.
        //Cleanup the map.
        for (std::map<uint64_t,std::list<std::pair<uint64_t, double>>*>::iterator to_delete = lti_trajectories.begin(); to_delete != lti_trajectories.end(); ++to_delete)
        {
            delete to_delete->second;
        }

        auto sql = sqlite_thread_guard(SQL->lti_count_num_appearances_init);
        sql->exec();
    });
    JobQueue->post(pt_calcTrajectories).wait();
}

/*inline soar_module::sqlite_statement* SMem_Manager::setup_manual_web_crawl(smem_weighted_cue_element* el, uint64_t lti_id)
{
    soar_module::sqlite_statement* q = NULL;
    if (el->element_type == attr_t)
    {
        q = SQL->web_attr_all_manual;
    }
    else if (el->element_type == value_const_t)
    {
        q = SQL->web_const_all_manual;
        q->bind_int(3, el->value_hash);
    }
    else if (el->element_type == value_lti_t)
    {
        q = SQL->web_lti_all_manual;
        q->bind_int(3, el->value_lti);
    }
    q->bind_int(2, lti_id);
    q->bind_int(1, el->attr_hash);

    return q;
}*/

std::shared_ptr<sqlite_thread_guard> SMem_Manager::setup_manual_web_crawl(smem_weighted_cue_element* el, uint64_t lti_id)
{
    // first, point to correct query and setup
    // query-specific parameters
    std::shared_ptr<sqlite_thread_guard> sql;

    if (el->element_type == attr_t)
    {
        // attribute_s_id=?
        sql = std::make_shared<sqlite_thread_guard>(SQL->web_attr_all_manual);
    }
    else if (el->element_type == value_const_t)
    {
        // attribute_s_id=? AND value_constant_s_id=?
        sql = std::make_shared<sqlite_thread_guard>(SQL->web_const_all_manual);
        (*sql)->bind(3, el->value_hash);
    }
    else if (el->element_type == value_lti_t)
    {
        // attribute_s_id=? AND value_lti_id=?
        sql = std::make_shared<sqlite_thread_guard>(SQL->web_lti_all_manual);
        (*sql)->bind(3, el->value_lti);
    }
    (*sql)->bind(2, lti_id);
    (*sql)->bind(1, el->attr_hash);

    return sql;
}

void SMem_Manager::calc_spread(std::set<uint64_t>* current_candidates, bool do_manual_crawl, smem_weighted_cue_list::const_iterator* cand_set)
{

    /*
     * The goal of this function is to as lazily as possible give spreading activation values when needed.
     * Lazy to such a point that we basically do queries twice instead of once just to avoid calculating
     * extra spreading activation. (We do an unsorted query first to get current_candidates.)
     *
     * The procedure:
     * 1 - for all new elements of the context, generate the mapping between sources and sinks w/raw traversal vals
     *      (add_fingerprint)
     *      1.1 - for all new mappings this added, add the relevant row to the uncommitted spreading table. (We may not use)
     *          (add_uncommitted_fingerprint)
     * 2 - for all removed elements of the context, remove their mapping between source and sink.
     *      (delete_old_spread)
     *      2.1 - for all removed elements that we never committed, just plain remove from uncommitted spreading table too.
     *          (delete_old_uncommitted_spread)
     *      2.2 - for all removed elements that we have committed, just make a new uncommitted note that we need to apply the negative.
     *          (reverse_old_committed_spread)
     * 3 - for all current_candidates, actually bother to go ahead and loop over their entries in the uncommitted spreading table.
     *      (calc_uncommitted_spread)
     *      3.1 - Get old val
     *      3.2 - do the math
     *      3.3 - make into new val
     */
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading->start();
    ////////////////////////////////////////////////////////////////////////////
    uint64_t count = 0;
    std::map<uint64_t,std::list<std::pair<uint64_t,double>>*> lti_trajectories;
    timers->spreading_wma_3->start();
    batch_invalidate_from_lti();
    timers->spreading_wma_3->stop();
    //if (settings->spreading_traversal->get_value() == smem_param_container::deterministic)
    {//One can do random walks or one can simulate them by doing a breadth-first traversal with coefficients. This is the latter.
        ////////////////////////////////////////////////////////////////////////////
        timers->spreading_1->start();
        ////////////////////////////////////////////////////////////////////////////
        for(std::set<uint64_t>::iterator it = smem_context_additions->begin(); it != smem_context_additions->end(); ++it)
        {//We keep track of old walks. If we haven't changed smem, no need to recalculate.
            //SQL->trajectory_check_invalid->bind_int(1,*it);
            //SQL->trajectory_get->bind_int(1,*it);
            //bool was_invalid = (SQL->trajectory_check_invalid->execute() == soar_module::row);
            //If the previous trajectory is no longer valid because of a change to memory or we don't have a trajectory, we might need to remove
            //the old one.
            //bool no_trajectory = SQL->trajectory_get->execute() != soar_module::row;
            //SQL->trajectory_check_invalid->reinitialize();

            std::packaged_task<bool()> pt_trajectoryCheck([this, it] {
                auto sql = sqlite_thread_guard(SQL->trajectory_check_invalid);

                sql->bind(1,*it);

                return sql->executeStep();
            });

            std::packaged_task<bool()> pt_trajectoryGet([this, it] {
                auto sql = sqlite_thread_guard(SQL->trajectory_get);

                sql->bind(1,*it);

                return sql->executeStep();
            });

            bool no_trajectory = JobQueue->post(pt_trajectoryGet).get();
            bool was_invalid = JobQueue->post(pt_trajectoryCheck).get();

            //SQL->trajectory_get->reinitialize();
            if (was_invalid || no_trajectory)
            {
                //We also need to make a new one.
                if (was_invalid)
                {
                    {
                        auto sql = sqlite_thread_guard(SQL->likelihood_cond_count_remove);
                        sql->bind(1,(*it));
                        sql->exec();
                    }
                    {
                        auto sql = sqlite_thread_guard(SQL->lti_count_num_appearances_remove);
                        sql->bind(1,(*it));
                        sql->exec();
                    }
                }
                trajectory_construction(*it,lti_trajectories);
                //statistics->expansions->set_value(statistics->expansions->get_value() + 1);
                //smem_calc_likelihoods_for_det_trajectories(thisAgent, (*it));
                {
                    auto sql = sqlite_thread_guard(SQL->lti_count_num_appearances_insert);
                    sql->bind(1,(*it));
                    sql->exec();
                }
            }
        }
        ////////////////////////////////////////////////////////////////////////////
        timers->spreading_1->stop();
        ////////////////////////////////////////////////////////////////////////////
    }
    /*else
    {//Random walks can be handled a little differently.
        for(std::set<uint64_t>::iterator it = thisAgent->smem_context_additions->begin(); it != thisAgent->smem_context_additions->end(); ++it)
        {//We keep track of old walks. If we havent changed smem, no need to recalculate.
            SQL->trajectory_get->bind_int(1,*it);
            count = 0;
            while(SQL->trajectory_get->execute() == soar_module::row)
            {
                count++;
            }//TODO: Make sure that we are doing invalidate correctly. This is likely not right. Spreading is correct for deterministic right now.
            if (count < settings->spreading_number_trajectories->get_value())
            {//Instead of needing to recalculate the whole thing, we can just do the random walks that were actually invalidated,
                //which could perhaps be faster.
                for (int i = 0; i < settings->spreading_number_trajectories->get_value()-count; ++i)
                {
                    std::list<uint64_t> trajectory;
                    trajectory.push_back(*it);
                    trajectory_construction(thisAgent,trajectory,lti_trajectories,settings->spreading_depth_limit->get_value());
                }
                for (int i = 1; i < 11; i++)
                {
                    SQL->likelihood_cond_count_insert->bind_int(i,(*it));
                }
                SQL->likelihood_cond_count_insert->execute(soar_module::op_reinit);
                SQL->lti_count_num_appearances_insert->bind_int(1,(*it));
                SQL->lti_count_num_appearances_insert->execute(soar_module::op_reinit);
            }
            SQL->trajectory_get->reinitialize();
        }
    }*/
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_2->start();
    ////////////////////////////////////////////////////////////////////////////
    for (std::map<uint64_t,std::list<std::pair<uint64_t,double>>*>::iterator to_delete = lti_trajectories.begin(); to_delete != lti_trajectories.end(); ++to_delete)
    {
        delete to_delete->second;
    }
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_2->stop();
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_3->start();
    ////////////////////////////////////////////////////////////////////////////
    //soar_module::sqlite_statement* add_fingerprint = SQL->add_fingerprint;
    //soar_module::sqlite_statement* select_fingerprint = SQL->select_fingerprint;
    //soar_module::sqlite_statement* add_uncommitted_fingerprint = SQL->add_uncommitted_fingerprint;
    //soar_module::sqlite_statement* remove_fingerprint_reversal = SQL->remove_fingerprint_reversal;
    for (std::set<uint64_t>::iterator it = smem_context_additions->begin(); it != smem_context_additions->end(); ++it)
    {//Now we add the walks/traversals we've done. //can imagine doing this as a batch process through a join on a list of the additions if need be.

        //std::packaged_task<void()> pt_addSelectedFingerprints([this,it]{
        {
            auto select = sqlite_thread_guard(SQL->select_fingerprint);
            select->bind(1, (*it));
            while (select->executeStep())
            {//lti_id,num_appearances_i_j,num_appearances,sign,lti_source
                uint64_t lti_recipient = select->getColumn(0).getUInt64();
                double num_app_i_j = select->getColumn(1).getDouble();
                double num_app = select->getColumn(2).getDouble();
                uint64_t sign = select->getColumn(3).getUInt64();
                uint64_t lti_source = select->getColumn(4).getUInt64();
                //std::packaged_task<void()> pt_addFingerprint([this,lti_recipient, num_app_i_j, num_app, sign, lti_source]{
                {
                    auto sql = sqlite_thread_guard(SQL->add_fingerprint);
                    sql->bind(1,lti_recipient);
                    sql->bind(2,num_app_i_j);
                    sql->bind(3,num_app);
                    sql->bind(4,sign);
                    sql->bind(5,lti_source);
                    sql->exec();
                }
                //});
                //Right here, I have a chance to add to "spreaded_to" because we have a row with a pariticular recipient.
                //When this fingerprint goes away, we can remove the recipient if this is the only fingerprint contributing to that recipient.
                //This is done by reference counting by fingerprint.
                if (smem_recipients_of_source->find(lti_source) == smem_recipients_of_source->end())
                {//This source has no recipients yet. we need to add this element to the map. This means making a new set.
                    (*(smem_recipients_of_source))[lti_source] = new std::set<uint64_t>;
                    (*(smem_recipients_of_source))[lti_source]->insert(lti_recipient);
                }
                else
                {//This source already has recipients. We just need to add to the set.
                    (*(smem_recipients_of_source))[lti_source]->insert(lti_recipient);
                }
                if (smem_recipient->find(lti_recipient) == smem_recipient->end())
                {
                    (*(smem_recipient))[lti_recipient] = 1;
                }
                else
                {//I need a second one of these that keeps track of those that actually received spread. OR - more clever:
                    //I just make the value of this a set of sources and when that set exists = potential spread.
                    //when it is populated with elements = those are the ones actually contributing spread.
                    (*(smem_recipient))[lti_recipient] = (*(smem_recipient))[lti_recipient] + 1;
                }
                //JobQueue->post(pt_addFingerprint).wait();
            }
        }
        //});

        //JobQueue->post(pt_addSelectedFingerprints).wait();

        /*select_fingerprint->bind_int(1,(*it));
        while (select_fingerprint->execute() == soar_module::row)
        {
            add_fingerprint->bind_int(1,select_fingerprint->column_int(0));
            add_fingerprint->bind_double(2,select_fingerprint->column_double(1));
            add_fingerprint->bind_double(3,select_fingerprint->column_double(2));
            add_fingerprint->bind_int(4,select_fingerprint->column_int(3));
            add_fingerprint->bind_int(5,select_fingerprint->column_int(4));
            add_fingerprint->execute(soar_module::op_reinit);
            //Right here, I have a chance to add to "spreaded_to" because we have a row with a pariticular recipient.
            //When this fingerprint goes away, we can remove the recipient if this is the only fingerprint contributing to that recipient.
            //This is done by reference counting by fingerprint.
            if (smem_recipients_of_source->find(select_fingerprint->column_int(4)) == smem_recipients_of_source->end())
            {//This source has no recipients yet. we need to add this element to the map. This means making a new set.
                (*(smem_recipients_of_source))[select_fingerprint->column_int(4)] = new std::set<uint64_t>;
                (*(smem_recipients_of_source))[select_fingerprint->column_int(4)]->insert(select_fingerprint->column_int(0));
            }
            else
            {//This source already has recipients. We just need to add to the set.
                (*(smem_recipients_of_source))[select_fingerprint->column_int(4)]->insert(select_fingerprint->column_int(0));
            }
            if (smem_recipient->find(select_fingerprint->column_int(0)) == smem_recipient->end())
            {
                (*(smem_recipient))[select_fingerprint->column_int(0)] = 1;
            }
            else
            {//I need a second one of these that keeps track of those that actually received spread. OR - more clever:
                //I just make the value of this a set of sources and when that set exists = potential spread.
                //when it is populated with elements = those are the ones actually contributing spread.
                (*(smem_recipient))[select_fingerprint->column_int(0)] = (*(smem_recipient))[select_fingerprint->column_int(0)] + 1;
            }
        }
        select_fingerprint->reinitialize();*/
        //I need to split this into separate select and insert batches. The select will allow me to keep an in-memory record of
        //potential spread recipients. The insert is then the normal insert. A select/insert combo would be nice, but that doesn't
        //make sense with the sqlite api.

    }
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_3->stop();
    ////////////////////////////////////////////////////////////////////////////
    //for (std::set<uint64_t>::iterator it = thisAgent->smem_context_additions->begin(); it != thisAgent->smem_context_additions->end(); ++it)
    //{
        //add_uncommitted_fingerprint->bind_int(1,(*it));
        //add_uncommitted_fingerprint->execute(soar_module::op_reinit);
        //remove_fingerprint_reversal->bind_int(1,(*it));
        //remove_fingerprint_reversal->bind_int(2,(*it));
        //remove_fingerprint_reversal->execute(soar_module::op_reinit);
    //}
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_4->start();
    ////////////////////////////////////////////////////////////////////////////
    smem_context_additions->clear();
    //soar_module::sqlite_statement* delete_old_spread = SQL->delete_old_spread;
    //soar_module::sqlite_statement* delete_old_uncommitted_spread = SQL->delete_old_uncommitted_spread;
    //soar_module::sqlite_statement* reverse_old_committed_spread = SQL->reverse_old_committed_spread;
    //delete_old_spread->prepare();
    std::unordered_map<uint64_t,uint64_t>* spreaded_to = smem_spreaded_to;
    std::set<uint64_t>::iterator recipient_it;
    std::set<uint64_t>::iterator recipient_begin;
    std::set<uint64_t>::iterator recipient_end;
    for (std::set<uint64_t>::iterator source_it = smem_context_removals->begin(); source_it != smem_context_removals->end(); ++source_it)
    {
        std::packaged_task<void()> pt_deleteOldSpread([this,source_it]{
            auto sql = sqlite_thread_guard(SQL->delete_old_spread);
            sql->bind(1, *source_it);
            sql->exec();
        });
        auto j = JobQueue->post(pt_deleteOldSpread);

        if (smem_recipients_of_source->find((*source_it)) != smem_recipients_of_source->end())
        {//This very well should be the case in fact... changed to an assert instead of if
            //Scratch that. The lti could have no real fingerprint, meaning that it doesn't have recipients.
            std::set<uint64_t>* recipient_set = (smem_recipients_of_source->at(*source_it));
            recipient_begin = recipient_set->begin();
            recipient_end = recipient_set->end();
            for (recipient_it = recipient_begin; recipient_it != recipient_end; ++recipient_it)
            {//We need to decrement the number of sources that lead to each recipient for each recipient from this source.
                assert(smem_recipient->find((*recipient_it)) != smem_recipient->end());
                //We check if this recipient now no longer has any spread sources as a result.
                if ((*(smem_recipient))[(*recipient_it)] == 1)
                {
                    smem_recipient->erase((*recipient_it));
                    //Also, if the element has been spreaded to,
                    //this allows us to detect that we can migrate the activation from the spread table to the base-level table.
                    if (spreaded_to->find(*recipient_it) != spreaded_to->end())
                    {
                        std::packaged_task<std::tuple<double,double>()> pt_fakeGet([this,recipient_it]{
                            auto sql = sqlite_thread_guard(SQL->act_lti_fake_get);
                            sql->bind(1, *recipient_it);
                            sql->exec();
                            return std::make_tuple(sql->getColumn(0).getDouble(),sql->getColumn(1).getDouble());
                        });
                        auto result_t = JobQueue->post(pt_fakeGet).get();
                        double prev_base = std::get<0>(result_t);
                        double spread = std::get<1>(result_t);
                        /*SQL->act_lti_fake_get->bind_int(1,*recipient_it);
                        SQL->act_lti_fake_get->execute();
                        //double spread = SQL->act_lti_fake_get->column_double(1);//This is the spread before changes.
                        double prev_base = SQL->act_lti_fake_get->column_double(0);
                        SQL->act_lti_fake_get->reinitialize();*/

                        std::packaged_task<void()> pt_fakeDelete([this, recipient_it]{
                            auto sql = sqlite_thread_guard(SQL->act_lti_fake_delete);
                            sql->bind(1, *recipient_it);
                            sql->exec();
                        });
                        JobQueue->post(pt_fakeDelete).wait();
                        /*SQL->act_lti_fake_delete->bind_int(1, *recipient_it);
                        SQL->act_lti_fake_delete->execute(soar_module::op_reinit);*/

                        std::packaged_task<void()> pt_lti_set([this, prev_base, recipient_it]{
                            auto sql = sqlite_thread_guard(SQL->act_lti_set);
                            sql->bind(1, prev_base == 0 ? SMEM_ACT_LOW:prev_base);
                            sql->bind(2,0.0);
                            sql->bind(3,prev_base);
                            sql->bind(4, *recipient_it);
                            sql->exec();
                        });
                        JobQueue->post(pt_fakeDelete).wait();
                        /*SQL->act_lti_set->bind_double(1, ((static_cast<double>(prev_base)==0) ? (SMEM_ACT_LOW):(prev_base)));
                        SQL->act_lti_set->bind_double(2, 0);
                        SQL->act_lti_set->bind_double(3, prev_base);
                        SQL->act_lti_set->bind_int(4, *recipient_it);
                        SQL->act_lti_set->execute(soar_module::op_reinit);*/

                        spreaded_to->erase(*recipient_it);

                        std::packaged_task<void()> pt_actSet([this, prev_base, recipient_it]{
                            auto sql = sqlite_thread_guard(SQL->act_set);
                            sql->bind(1, prev_base);
                            sql->bind(2, *recipient_it);
                            sql->exec();
                        });
                        JobQueue->post(pt_actSet).wait();
                        /*SQL->act_set->bind_double(1, prev_base);
                        SQL->act_set->bind_int(2, *recipient_it);
                        SQL->act_set->execute(soar_module::op_reinit);*/

                        //SQL->act_lti_fake_get->reinitialize();
                    }
                }
                else
                {
                    (*(smem_recipient))[(*recipient_it)] = (*(smem_recipient))[(*recipient_it)] - 1;
                }
            }
            delete recipient_set;
            smem_recipients_of_source->erase((*source_it));
        }
        //delete_old_uncommitted_spread->bind_int(1,(*it));
        //delete_old_uncommitted_spread->bind_int(2,(*it));
        //delete_old_uncommitted_spread->execute(soar_module::op_reinit);
        //reverse_old_committed_spread->bind_int(1,(*it));
        //reverse_old_committed_spread->execute(soar_module::op_reinit);
        j.wait();
        /*delete_old_spread->bind_int(1,(*source_it));
        delete_old_spread->execute(soar_module::op_reinit);*/
    }
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_4->stop();
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_5->start();
    ////////////////////////////////////////////////////////////////////////////
    smem_context_removals->clear();
    double prev_base;
    double raw_prob;
    double additional;
    double offset;
    bool still_exists;
    double spread = 0;
    double modified_spread = 0;
    std::set<uint64_t> pruned_candidates;
    //soar_module::sqlite_statement* list_uncommitted_spread = SQL->list_uncommitted_spread;
    //soar_module::sqlite_statement* list_current_spread = SQL->list_current_spread;
   //do_manual_crawl = true;
    if (do_manual_crawl)
    {//This means that the candidate set was quite large, so we instead manually check the sql store for candidacy.
        //soar_module::sqlite_statement* q_manual;
        auto list_current_spread = sqlite_thread_guard(SQL->list_current_spread);
        while (list_current_spread->executeStep())
        {//we loop over all spread sinks
            auto q_manual = setup_manual_web_crawl(**cand_set, list_current_spread->getColumn(0).getUInt64());
            if ((*q_manual)->executeStep())//and if the sink is a candidate, we will actually calculate on it later.
            {
                pruned_candidates.insert(list_current_spread->getColumn(0).getUInt64());
            }
            //q_manual->reinitialize();
        }
    }
    //list_current_spread->reinitialize();
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_5->stop();
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_6->start();
    ////////////////////////////////////////////////////////////////////////////
    //gotta calculate correct denominator for baseline value
    double baseline_denom = settings->spreading_continue_probability->get_value();
    double decay_const = baseline_denom;
    int depth_limit = settings->spreading_depth_limit->get_value();
    for (int i = 0; i < depth_limit; i++)
    {
        baseline_denom = baseline_denom + baseline_denom*decay_const;
    }

    //soar_module::sqlite_statement* calc_current_spread = SQL->calc_current_spread;
    std::set<uint64_t>* actual_candidates = ( do_manual_crawl ? &pruned_candidates : current_candidates);
    std::unordered_set<uint64_t> updated_candidates;
    //spreaded_to->clear();
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_6->stop();
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_7->start();
    ////////////////////////////////////////////////////////////////////////////
    for (std::set<uint64_t>::iterator candidate = actual_candidates->begin(); candidate != actual_candidates->end(); ++candidate)//for every sink that has some spread, we calculate
    {
        ////////////////////////////////////////////////////////////////////////////
        timers->spreading_7_1->start();
        ////////////////////////////////////////////////////////////////////////////
        if (spreaded_to->find(*candidate) != spreaded_to->end())
        {
            std::packaged_task<std::tuple<double,double>()> pt_fakeGet([this,candidate]{
                auto sql = sqlite_thread_guard(SQL->act_lti_fake_get);
                sql->bind(1, *candidate);
                sql->exec();
                return std::make_tuple(sql->getColumn(0).getDouble(),sql->getColumn(1).getDouble());
            });
            auto result_t = JobQueue->post(pt_fakeGet).get();
            prev_base = std::get<0>(result_t);
            spread = std::get<1>(result_t);
            /*SQL->act_lti_fake_get->bind_int(1,*recipient_it);
            SQL->act_lti_fake_get->execute();
            //double spread = SQL->act_lti_fake_get->column_double(1);//This is the spread before changes.
            double prev_base = SQL->act_lti_fake_get->column_double(0);
            SQL->act_lti_fake_get->reinitialize();*/

            std::packaged_task<void()> pt_fakeDelete([this, candidate]{
                auto sql = sqlite_thread_guard(SQL->act_lti_fake_delete);
                sql->bind(1, *candidate);
                sql->exec();
            });
            JobQueue->post(pt_fakeDelete).wait();
            /*SQL->act_lti_fake_delete->bind_int(1, *recipient_it);
            SQL->act_lti_fake_delete->execute(soar_module::op_reinit);*/

            std::packaged_task<void()> pt_lti_set([this, prev_base, candidate]{
                auto sql = sqlite_thread_guard(SQL->act_lti_set);
                sql->bind(1, prev_base == 0 ? SMEM_ACT_LOW:prev_base);
                sql->bind(2,0.0);
                sql->bind(3,prev_base);
                sql->bind(4, *candidate);
                sql->exec();
            });
            JobQueue->post(pt_fakeDelete).wait();

            /*SQL->act_lti_fake_get->bind_int(1,*candidate);
            SQL->act_lti_fake_get->execute();
            //double spread = SQL->act_lti_fake_get->column_double(1);//This is the spread before changes.
            double prev_base = SQL->act_lti_fake_get->column_double(0);
            SQL->act_lti_fake_get->reinitialize();
            SQL->act_lti_fake_delete->bind_int(1, *candidate);
            SQL->act_lti_fake_delete->execute(soar_module::op_reinit);
            SQL->act_lti_set->bind_double(1, ((static_cast<double>(prev_base)==0) ? (SMEM_ACT_LOW):(prev_base)));
            SQL->act_lti_set->bind_double(2, 0);
            SQL->act_lti_set->bind_double(3, prev_base);
            SQL->act_lti_set->bind_int(4, *candidate);
            SQL->act_lti_set->execute(soar_module::op_reinit);*/
            //SQL->act_lti_fake_get->reinitialize();
            spreaded_to->erase(*candidate);
        }
        ////////////////////////////////////////////////////////////////////////////
        timers->spreading_7_1->stop();
        ////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////
        timers->spreading_7_2->start();
        ////////////////////////////////////////////////////////////////////////////
        //calc_current_spread->bind_int(1,(*candidate));
        auto calc_current_spread = sqlite_thread_guard(SQL->calc_current_spread);
        calc_current_spread->bind(1, (*candidate));
        while (calc_current_spread->executeStep() && calc_current_spread->getColumn(2).getDouble())
        {
            //First, I need to get the existing info for this lti_id.
            bool already_in_spread_table = false;

            bool addition = ((calc_current_spread->getColumn(3).getUInt64()) == 1);
            if (addition)
            {

                if (spreaded_to->find(*candidate) == spreaded_to->end())//(updated_candidates.find(*candidate) == updated_candidates.end())
                {
                    ////////////////////////////////////////////////////////////////////////////
                    timers->spreading_7_2_1->start();
                    ////////////////////////////////////////////////////////////////////////////
                    std::packaged_task<std::tuple<double,double>()> pt_ltiGet([this,candidate]{
                        auto sql = sqlite_thread_guard(SQL->act_lti_get);
                        sql->bind(1, *candidate);
                        sql->exec();
                        return std::make_tuple(sql->getColumn(0).getDouble(),sql->getColumn(1).getDouble());
                    });
                    auto result_t = JobQueue->post(pt_ltiGet).get();
                    prev_base = std::get<0>(result_t);
                    spread = std::get<1>(result_t);
                    (*spreaded_to)[*candidate] = 1;
                    /*SQL->act_lti_get->bind_int(1,*candidate);
                    SQL->act_lti_get->execute();
                    spread = SQL->act_lti_get->column_double(1);//This is the spread before changes.
                    prev_base = SQL->act_lti_get->column_double(0);
                    SQL->act_lti_get->reinitialize();*/
                    ////////////////////////////////////////////////////////////////////////////
                    timers->spreading_7_2_1->stop();
                    ////////////////////////////////////////////////////////////////////////////
                }
                else
                {
                    ////////////////////////////////////////////////////////////////////////////
                    timers->spreading_7_2_2->start();
                    ////////////////////////////////////////////////////////////////////////////
                    already_in_spread_table = true;
                    //if (!(updated_candidates.find(*candidate) == updated_candidates.end()))
                    {
                        (*spreaded_to)[*candidate] = (*spreaded_to)[*candidate] + 1;
                    }
                    std::packaged_task<std::tuple<double,double>()> pt_fakeGet([this,candidate]{
                        auto sql = sqlite_thread_guard(SQL->act_lti_fake_get);
                        sql->bind(1, *candidate);
                        sql->exec();
                        return std::make_tuple(sql->getColumn(0).getDouble(),sql->getColumn(1).getDouble());
                    });
                    auto result_t = JobQueue->post(pt_fakeGet).get();
                    prev_base = std::get<0>(result_t);
                    spread = std::get<1>(result_t);

                    /*SQL->act_lti_fake_get->bind_int(1,*candidate);
                    SQL->act_lti_fake_get->execute();
                    spread = SQL->act_lti_fake_get->column_double(1);//This is the spread before changes.
                    prev_base = SQL->act_lti_fake_get->column_double(0);
                    SQL->act_lti_fake_get->reinitialize();*/
                    ////////////////////////////////////////////////////////////////////////////
                    timers->spreading_7_2_2->stop();
                    ////////////////////////////////////////////////////////////////////////////
                }
                ////////////////////////////////////////////////////////////////////////////
                timers->spreading_7_2_3->start();
                ////////////////////////////////////////////////////////////////////////////
                if (updated_candidates.find(*candidate) == updated_candidates.end())
                {//If we have yet to update the spread to this candidate this cycle, we need to reset it to 0.
                    spread = 0;
                    updated_candidates.insert(*candidate);
                }
                ////////////////////////////////////////////////////////////////////////////
                timers->spreading_7_2_3->stop();
                ////////////////////////////////////////////////////////////////////////////
/*
                if (settings->spreading_normalization->get_value() == off && settings->spreading_traversal->get_value() == smem_param_container::deterministic && settings->spreading_loop_avoidance->get_value() == on)
                {
                    raw_prob = (((double)(calc_uncommitted_spread->column_double(2))));
                }
                else*/
                //Alright, the big point here is to make the magnitude of activation in some manner depend on the magnitude of
                //the WMA of the source of the spread. That's weird, but whatever. The big deal is to translate the WMA
                //of the source (which we also have to define) into probability space. this isn't entirely principled, but if we normalize
                //across all WMA for sources, we don't really have to translate. I'll start with just using a raw multiplicative factor because
                //it'll be easier. So, spread X factor. Now, that factor will first be like some super activation. I'll basically add the WMAs
                //individual activations together like they were all boosting the same thing. This should normalize in a sense. Whatever, this is
                //the first stab.
                ////////////////////////////////////////////////////////////////////////////
                timers->spreading_7_2_4->start();
                ////////////////////////////////////////////////////////////////////////////
                wma_decay_element total_element;
                total_element.this_wme = NULL;
                total_element.just_removed = false;
                total_element.just_created = true;
                total_element.forget_cycle = 0;
                total_element.num_references = 0;
                //std::list<wma_reference> touches;
                std::list<wma_cycle_reference> cycles;
                unsigned int counter;
                auto wmas = smem_wmas->equal_range(calc_current_spread->getColumn(4).getUInt64());//column_int(4));
                double pre_logd_wma = 0;
                bool used_wma = false;
                if (thisAgent->SMem->settings->spreading_wma_source->get_value() == true)
                {
                    for (auto wma = wmas.first; wma != wmas.second; ++wma)
                    {
                        used_wma = true;
                        // Now that we have a wma decay element, we loop over its history and increment all of the fields for our fake decay element.
                        total_element.num_references += wma->second->touches.total_references;
                        counter = wma->second->touches.history_ct;
                        while(counter)
                        {
                            int cycle_diff = thisAgent->WM->wma_d_cycle_count - wma->second->touches.access_history[counter-1].d_cycle;
                            assert(cycle_diff > 0);
                            //cycles.push_back(wma->second->touches.access_history[counter]);
                            if (cycle_diff < thisAgent->WM->wma_power_size)
                            {
                                pre_logd_wma += wma->second->touches.access_history[counter-1].num_references * thisAgent->WM->wma_power_array[ cycle_diff ];
                            }
                            else
                            {
                                pre_logd_wma += wma->second->touches.access_history[counter-1].num_references * pow(cycle_diff,thisAgent->WM->wma_params->decay_rate->get_value());
                            }
                            counter--;
                        }
                    }
                }
                ////////////////////////////////////////////////////////////////////////////
                timers->spreading_7_2_4->stop();
                ////////////////////////////////////////////////////////////////////////////
                //For now, I'm not going to deal with the Petrov approximation and leave this value incorrectly
                //truncated so that old activations are just completely ignored.
                //In theory, BLA corresponds to log-odds, so I guess we're dealing with odds for the above value.
                ////////////////////////////////////////////////////////////////////////////
                timers->spreading_7_2_5->start();
                ////////////////////////////////////////////////////////////////////////////
                double wma_multiplicative_factor = pre_logd_wma/(1.0+pre_logd_wma);//1;//pre_logd_wma/(1.0+pre_logd_wma);
                if (!used_wma || pre_logd_wma == 0)
                {
                    //wma_multiplicative_factor = 1; This is actually bad. Since probabilities max at one, I rewarded not having wma.
                    pre_logd_wma = pow(static_cast<double>(smem_max_cycle+settings->base_unused_age_offset->get_value()),static_cast<double>(-(settings->base_decay->get_value())));
                    wma_multiplicative_factor = pre_logd_wma/(1.0+pre_logd_wma);
                }
                {
                    raw_prob = wma_multiplicative_factor*((calc_current_spread->getColumn(2).getDouble())/(calc_current_spread->getColumn(1).getDouble()));
                }
                //offset = (settings->spreading_baseline->get_value())/(calc_spread->column_double(1));
                offset = (settings->spreading_baseline->get_value())/baseline_denom;//(settings->spreading_limit->get_value());
                additional = (raw_prob > offset ? raw_prob : offset);//(log(raw_prob)-log(offset));//This is a hack to prevent bad values for low wma spread.//additional = (raw_prob > offset ? raw_prob : offset+offset*.1);//(log(raw_prob)-log(offset));//This is a hack to prevent bad values for low wma spread.
                spread = (spread == 0 ? additional : additional+spread);//Now, we've adjusted the activation according to this new addition.
                ////////////////////////////////////////////////////////////////////////////
                timers->spreading_7_2_5->stop();
                ////////////////////////////////////////////////////////////////////////////
                ////////////////////////////////////////////////////////////////////////////
                timers->spreading_7_2_6->start();
                ////////////////////////////////////////////////////////////////////////////
                /*SQL->act_lti_child_ct_get->bind_int(1, *candidate);
                SQL->act_lti_child_ct_get->execute();
                uint64_t num_edges = SQL->act_lti_child_ct_get->column_int(0);
                SQL->act_lti_child_ct_get->reinitialize();*/

                std::packaged_task<uint64_t()> pt_childCtGet([this, candidate]{
                    auto sql = sqlite_thread_guard(SQL->act_lti_child_ct_get);
                    sql->bind(1, *candidate);
                    sql->exec();
                    return sql->getColumn(0).getUInt64();
                });

                uint64_t num_edegs = JobQueue->post(pt_childCtGet).get();

                double modified_spread = (log(spread)-log(offset));
                double new_base;
                if (static_cast<double>(prev_base)==static_cast<double>(SMEM_ACT_LOW) || static_cast<double>(prev_base) == 0)
                {//used for base-level - thisAgent->smem_max_cycle - We assume that the memory was accessed at least "age of the agent" ago if there is no record.
                    double decay = settings->base_decay->get_value();
                    new_base = pow(static_cast<double>(smem_max_cycle+settings->base_unused_age_offset->get_value()),static_cast<double>(-decay));
                    new_base = log(new_base/(1+new_base));
                }
                else
                {
                    new_base = prev_base;
                }
                ////////////////////////////////////////////////////////////////////////////
                timers->spreading_7_2_6->stop();
                ////////////////////////////////////////////////////////////////////////////
                //SQL->add_committed_fingerprint->bind_int(1,*candidate);
                //SQL->add_committed_fingerprint->bind_double(2,(double)(calc_current_spread->column_double(2)));
                //SQL->add_committed_fingerprint->bind_double(3,(double)(calc_current_spread->column_double(1)));
                //SQL->add_committed_fingerprint->bind_int(4,(calc_current_spread->column_int(4)));
                //SQL->add_committed_fingerprint->execute(soar_module::op_reinit);
                if (already_in_spread_table)
                {
                    ////////////////////////////////////////////////////////////////////////////
                    timers->spreading_7_2_7->start();
                    ////////////////////////////////////////////////////////////////////////////
                    std::packaged_task<void()> pt_fakeSet([this, prev_base, spread, modified_spread, new_base, candidate]{
                        auto sql = sqlite_thread_guard(SQL->act_lti_fake_set);
                        sql->bind(1, prev_base == 0 ? SMEM_ACT_LOW : prev_base);
                        sql->bind(2, spread);
                        sql->bind(3, modified_spread + new_base);
                        sql->bind(4, *candidate);
                        sql->exec();
                    });
                    JobQueue->post(pt_fakeSet).wait();
                    /*SQL->act_lti_fake_set->bind_double(1, ((static_cast<double>(prev_base)==0) ? (SMEM_ACT_LOW):(prev_base)));
                    SQL->act_lti_fake_set->bind_double(2, spread);
                    SQL->act_lti_fake_set->bind_double(3, modified_spread+ new_base);
                    SQL->act_lti_fake_set->bind_int(4, *candidate);
                    SQL->act_lti_fake_set->execute(soar_module::op_reinit);*/
                    ////////////////////////////////////////////////////////////////////////////
                    timers->spreading_7_2_7->stop();
                    ////////////////////////////////////////////////////////////////////////////
                }
                else
                {
                    ////////////////////////////////////////////////////////////////////////////
                    timers->spreading_7_2_8->start();
                    ////////////////////////////////////////////////////////////////////////////
                    std::packaged_task<void()> pt_fakeInsert([this, candidate, prev_base, spread, modified_spread, new_base]{
                        auto sql = sqlite_thread_guard(SQL->act_lti_fake_insert);
                        sql->bind(1, *candidate);
                        sql->bind(2, ((static_cast<double>(prev_base)==0) ? (SMEM_ACT_LOW):(prev_base)));
                        sql->bind(3, spread);
                        sql->bind(4, modified_spread + new_base);
                        sql->exec();
                    });
                    JobQueue->post(pt_fakeInsert).wait();

                    //In order to prevent the activation from the augmentations table from coming into play after this has been given spread, we set the augmentations bla to be smemactlow
                    std::packaged_task<void()> pt_actSet([this, candidate]{
                        auto sql = sqlite_thread_guard(SQL->act_set);
                        sql->bind(1, static_cast<double>(SMEM_ACT_LOW));
                        sql->bind(2, *candidate);
                        sql->exec();
                    });
                    JobQueue->post(pt_actSet).wait();


                    // The above fix was abhorrently slow. Instead of changing the activation value and forcing a reindexing for like 3 indexes on the largest table,
                    // I fix the issue by using more clever queries that involve not pulling lti activations from the default table when it has spread.

                    ////////////////////////////////////////////////////////////////////////////
                    timers->spreading_7_2_8->stop();
                    ////////////////////////////////////////////////////////////////////////////
                }

            }
            /*else
            {
                ////////////////////////////////////////////////////////////////////////////
                thisAgent->smem_timers->spreading_4->start();
                ////////////////////////////////////////////////////////////////////////////
                bool remove = (((*spreaded_to)[*candidate]) == 1);
                (*spreaded_to)[*candidate] = (*spreaded_to)[*candidate] - 1;
                if ((*spreaded_to)[*candidate]==-1)
                {// This shouldn't actually ever happen.
                    assert(false);
                    SQL->act_lti_get->bind_int(1,*candidate);
                    SQL->act_lti_get->execute();
                    spread = SQL->act_lti_get->column_double(1);//This is the spread before changes.
                    prev_base = SQL->act_lti_get->column_double(0);
                    SQL->act_lti_get->reinitialize();
                }
                else
                {
                    SQL->act_lti_fake_get->bind_int(1,*candidate);
                    SQL->act_lti_fake_get->execute();
                    spread = SQL->act_lti_fake_get->column_double(1);//This is the spread before changes.
                    prev_base = SQL->act_lti_fake_get->column_double(0);
                    SQL->act_lti_fake_get->reinitialize();
                }

                //if (settings->spreading_normalization->get_value() == off && settings->spreading_traversal->get_value() == smem_param_container::deterministic && settings->spreading_loop_avoidance->get_value() == on)
                //{//Basically, this is for when normalization is off.
                //    raw_prob = (((double)(calc_uncommitted_spread->column_double(2))));
                //}
                //else
                {//This is the default behavior.
                    raw_prob = (((double)(calc_current_spread->column_double(2)))/(calc_current_spread->column_double(1)));
                }//There is some offset value so that we aren't going to compare to negative infinity (log(0)).
                //It could be thought of as an overall confidence in spreading itself.
                offset = (settings->spreading_baseline->get_value())/baseline_denom;//(settings->spreading_limit->get_value());
                //additional = (log(raw_prob)-log(offset));

                SQL->delete_commit_of_negative_fingerprint->bind_int(1,*candidate);
                SQL->delete_commit_of_negative_fingerprint->bind_int(2,calc_current_spread->column_int(4));
                SQL->delete_commit_of_negative_fingerprint->execute(soar_module::op_reinit);
                spread-=raw_prob;//additional;//Now, we've adjusted the activation according to this new addition.
                SQL->act_lti_child_ct_get->bind_int(1, *candidate);
                SQL->act_lti_child_ct_get->execute();

                uint64_t num_edges = SQL->act_lti_child_ct_get->column_int(0);

                SQL->act_lti_child_ct_get->reinitialize();
                double modified_spread = ((spread < offset) || (spread < 0)) ? (0) : (log(spread)-log(offset));
                spread = (spread < offset) ? (0) : (spread);
                //This is the same sort of activation updating one would have to do with base-level.
                //double prev_base = SQL->act_lti_get->column_double(0);
                double new_base;
                if (static_cast<double>(prev_base)==static_cast<double>(SMEM_ACT_LOW) || static_cast<double>(prev_base) == 0)
                {//used for base-level - thisAgent->smem_max_cycle - We assume that the memory was accessed at least "age of the agent" ago if there is no record.
                    double decay = settings->base_decay->get_value();
                    new_base = pow(static_cast<double>(thisAgent->smem_max_cycle),static_cast<double>(-decay));
                    new_base = log(new_base/(1+new_base));
                }
                else
                {
                    new_base = prev_base;
                }
                if (!remove)
                {
                    SQL->act_lti_fake_set->bind_double(1, ((static_cast<double>(prev_base)==0) ? (SMEM_ACT_LOW):(prev_base)));
                    SQL->act_lti_fake_set->bind_double(2, spread);
                    SQL->act_lti_fake_set->bind_double(3, modified_spread+new_base);
                    SQL->act_lti_fake_set->bind_int(4, *candidate);
                    SQL->act_lti_fake_set->execute(soar_module::op_reinit);
                }
                else
                {
                    SQL->act_lti_fake_delete->bind_int(1, *candidate);
                    SQL->act_lti_fake_delete->execute(soar_module::op_reinit);
                    SQL->act_lti_set->bind_double(1, ((static_cast<double>(prev_base)==0) ? (SMEM_ACT_LOW):(prev_base)));
                    SQL->act_lti_set->bind_double(2, spread);
                    SQL->act_lti_set->bind_double(3, modified_spread+new_base);
                    SQL->act_lti_set->bind_int(4, *candidate);
                    SQL->act_lti_set->execute(soar_module::op_reinit);
                }
                ////////////////////////////////////////////////////////////////////////////
                thisAgent->smem_timers->spreading_4->stop();
                ////////////////////////////////////////////////////////////////////////////
            }*/
        }
        //calc_current_spread->reinitialize();
        ////////////////////////////////////////////////////////////////////////////
        timers->spreading_7_2->stop();
        ////////////////////////////////////////////////////////////////////////////
        //SQL->prepare_delete_committed_fingerprint->bind_int(1,*candidate);
        //SQL->prepare_delete_committed_fingerprint->execute(soar_module::op_reinit);
    }
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_7->stop();
    ////////////////////////////////////////////////////////////////////////////
    //SQL->delete_committed_fingerprint->execute(soar_module::op_reinit);
    //SQL->delete_committed_fingerprint_2->execute(soar_module::op_reinit);
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading->stop();
    ////////////////////////////////////////////////////////////////////////////
}



void SMem_Manager::invalidate_trajectories(uint64_t lti_parent_id, std::map<uint64_t, int64_t>* delta_children)
{
    std::map<uint64_t, int64_t>::iterator delta_child;
    std::list<uint64_t>* negative_children = new std::list<uint64_t>;
    for (delta_child = delta_children->begin(); delta_child != delta_children->end(); ++delta_child)
    {//for every edge change in smem, we need to properly invalidate trajectories used in spreading.
        if (delta_child->second > 0)
        {
            std::packaged_task<void()> pt_invalidateFromLTI([this, lti_parent_id]{
                auto sql = sqlite_thread_guard(SQL->trajectory_invalidate_from_lti);
            for (int i = 1; i < 11; i++)
            {
                    sql->bind(i, lti_parent_id);
                    //As it turns out, sqlite is smart about unioning ors that all have a single index.
                }
                sql->exec();
            });
            JobQueue->post(pt_invalidateFromLTI).wait();
            /*for (int i = 1; i < 11; i++)
            {
                SQL->trajectory_invalidate_from_lti->bind_int(i, lti_parent_id);
                //As it turns out, sqlite is smart about unioning ors that all have a single index.
            }
            SQL->trajectory_invalidate_from_lti->execute(soar_module::op_reinit);*/
        }
        else if (delta_child->second < 0)
        {
            negative_children->push_front(delta_child->first);
        }
    }
    // If we even get here, it means that we only had negative children (removals) and we invalidate according to them.
    // (Additions make you invalidate a lot more than removals.)
    while (!negative_children->empty())
    {//For negative edge changes, only trajectories that used that edge need to be removed.
        //sqlite command to delete trajectories involving parent to delta_children->front();
        uint64_t front = negative_children->front();
        std::packaged_task<void()> pt_invalidateEdge([this, lti_parent_id, front]{
            auto sql = sqlite_thread_guard(SQL->trajectory_invalidate_edge);
        for (int i = 1; i < 11; i++)
        {
                sql->bind(2*i-1, lti_parent_id);
                sql->bind(1*i, front);
            }
            sql->exec();
        });
        JobQueue->post(pt_invalidateEdge).wait();
        /*for (int i = 1; i < 11; i++)
        {
            SQL->trajectory_invalidate_edge->bind_int(2*i-1, lti_parent_id);
            SQL->trajectory_invalidate_edge->bind_int(1*i, negative_children->front());
        }*/
        //SQL->trajectory_invalidate_edge->execute(soar_module::op_reinit);
        negative_children->pop_front();
    }
    delete negative_children;
}

//I could later change the below helper functions to merely post the job and return the posted job so that it could later be waited on.

void SMem_Manager::invalidate_from_lti(uint64_t invalid_parent)
{
    std::packaged_task<void()> pt_invalidFromLTI([this, invalid_parent]{
        auto sql = sqlite_thread_guard(SQL->trajectory_invalidate_from_lti);
    for (int i = 1; i < 11; i++)
    {//A changing edge weight is treated as an invalidation of cases that could have used that edge.
            sql->bind(i,invalid_parent);
        }
        sql->exec();
    });
    JobQueue->post(pt_invalidFromLTI).wait();
    /*for (int i = 1; i < 11; i++)
    {//A changing edge weight is treated as an invalidation of cases that could have used that edge.
        SQL->trajectory_invalidate_from_lti->bind_int(i,invalid_parent);
    }
    SQL->trajectory_invalidate_from_lti->execute(soar_module::op_reinit);*/
}

void SMem_Manager::add_to_invalidate_from_lti_table(uint64_t invalid_parent)
{
    std::packaged_task<void()> pt_invalidFromLTIAdd([this, invalid_parent]{
        auto sql = sqlite_thread_guard(SQL->trajectory_invalidate_from_lti_add);
        sql->bind(1, invalid_parent);
        sql->exec();
    });
    JobQueue->post(pt_invalidFromLTIAdd).wait();
    /*SQL->trajectory_invalidate_from_lti_add->bind_int(1, invalid_parent);
    SQL->trajectory_invalidate_from_lti_add->execute(soar_module::op_reinit);*/
}

void SMem_Manager::batch_invalidate_from_lti()
{
    /* MMerge | Not sure if this line will work by newer version of smem had this check first
     *     if (SQL->trajectory_invalidation_check_for_rows->execute() == soar_module::row)
     * */
    std::packaged_task<uint64_t()> pt_invalid_check_for_rows([this]{
        auto sql = sqlite_thread_guard(SQL->trajectory_invalidation_check_for_rows);
        sql->exec();
        return sql->getColumn(0).getUInt64();
    });

    if (JobQueue->post(pt_invalid_check_for_rows).get())

    {

        std::packaged_task<void()> pt_invalidFromTable([this]{
            auto sql = sqlite_thread_guard(SQL->trajectory_invalidate_from_lti_table);
            sql->exec();
        });
        JobQueue->post(pt_invalidFromTable).wait();

        std::packaged_task<void()> pt_invalidClear([this]{
            auto sql = sqlite_thread_guard(SQL->trajectory_invalidate_from_lti_clear);
            sql->exec();
        });
        JobQueue->post(pt_invalidClear).wait();
        /*SQL->trajectory_invalidate_from_lti_table->execute(soar_module::op_reinit);
          SQL->trajectory_invalidate_from_lti_clear->execute(soar_module::op_reinit);*/
    }
}
