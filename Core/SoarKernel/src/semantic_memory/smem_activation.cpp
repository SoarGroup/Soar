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

#include "VariadicBind.h"
#include "guard.hpp"

double SMem_Manager::lti_calc_base(uint64_t pLTI_ID, int64_t time_now, uint64_t n, uint64_t activations_first)
{
    double sum = 0.0;
    double d = settings->base_decay->get_value();
    uint64_t t_k;
    uint64_t t_n = (time_now - activations_first);

    if (n == 0)
    {
        JobQueue.post([&]() mutable {
            auto sql = sqlite_thread_guard(SQL.lti_access_get);

            SQLite::bind(*sql, pLTI_ID);
            assert(sql->executeStep());

            n = sql->getColumn(0).getInt();
            activations_first = sql->getColumn(2).getInt();
        })->wait();
    }

    // get all history
    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.history_get);

        SQLite::bind(*sql, pLTI_ID);

        if (sql->executeStep())
        {
            uint64_t available_history = (SMEM_ACT_HISTORY_ENTRIES < n) ? (SMEM_ACT_HISTORY_ENTRIES) : (n);
            t_k = uint64_t(time_now - sql->getColumn(int(available_history - 1)).getInt64());

            for (uint64_t i = 0; i < available_history; i++)
            {
                sum += pow(double(time_now - sql->getColumn(int(i)).getInt64()),
                           double(-d));
            }
        }
    })->wait();

    // if available history was insufficient, approximate rest
    if (n > SMEM_ACT_HISTORY_ENTRIES)
    {
        double apx_numerator = (double(n - SMEM_ACT_HISTORY_ENTRIES) * (pow(double(t_n), 1.0 - d) - pow(double(t_k), 1.0 - d)));
        double apx_denominator = ((1.0 - d) * double(t_n - t_k));

        sum += (apx_numerator / apx_denominator);
    }

    return ((sum > 0) ? (log(sum)) : (SMEM_ACT_LOW));
}

// activates a new or existing long-term identifier
// note: optional num_edges parameter saves us a lookup
//       just when storing a new chunk (default is a
//       big number that should never come up naturally
//       and if it does, satisfies thresholding behavior).
double SMem_Manager::lti_activate(uint64_t pLTI_ID, bool add_access, uint64_t num_edges)
{
    ////////////////////////////////////////////////////////////////////////////
    timers->act->start();
    ////////////////////////////////////////////////////////////////////////////

    int64_t time_now;
    if (add_access)
    {
        time_now = smem_max_cycle++;

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
                        JobQueue.post([&]() {
                            auto sql = sqlite_thread_guard(SQL.lti_get_t);

                            SQLite::bind(*sql, time_diff);

                            while (sql->executeStep())
                                lti_activate(sql->getColumn(0).getInt64(), false);
                        })->wait();
                    }
                }
            }
        }
    }
    else
    {
        time_now = smem_max_cycle;

        statistics->act_updates->set_value(statistics->act_updates->get_value() + 1);
    }

    // access information
    uint64_t prev_access_n = 0;
    uint64_t prev_access_t = 0;
    uint64_t prev_access_1 = 0;

    // get old (potentially useful below)
    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.lti_access_get);

        SQLite::bind(*sql, pLTI_ID);
        assert(sql->executeStep());

        prev_access_n = sql->getColumn(0).getUInt64();
        prev_access_t = sql->getColumn(1).getUInt64();
        prev_access_1 = sql->getColumn(2).getUInt64();

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
        sql->reset();

        // set new
        if (add_access)
        {
            JobQueue.post([&]() {
                auto sql = sqlite_thread_guard(SQL.lti_access_set);

                SQLite::bind(*sql,
                             prev_access_n + 1,
                             time_now,
                             ((prev_access_n == 0) ? (time_now) : (prev_access_1)),
                             pLTI_ID);

                sql->exec();
            })->wait();
        }
    })->wait();

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
        if (prev_access_n == 0)
        {
            if (add_access)
            {
                JobQueue.post([&]() {
                    auto sql = sqlite_thread_guard(SQL.history_add);

                    SQLite::bind(*sql, pLTI_ID, time_now);
                    sql->exec();
                })->wait();
            }

            new_activation = lti_calc_base(pLTI_ID, time_now + ((add_access) ? (1) : (0)), prev_access_n + ((add_access) ? (1) : (0)), prev_access_1);;
        }
        else
        {
            if (add_access)
            {
                JobQueue.post([&]() {
                    auto sql = sqlite_thread_guard(SQL.history_push);

                    SQLite::bind(*sql, time_now, pLTI_ID);
                    sql->exec();
                })->wait();
            }

            new_activation = lti_calc_base(pLTI_ID, time_now + ((add_access) ? (1) : (0)), prev_access_n + ((add_access) ? (1) : (0)), prev_access_1);
        }
    }

    // get number of augmentations (if not supplied)
    if (num_edges == SMEM_ACT_MAX)
    {
        JobQueue.post([&]() mutable {
            auto sql = sqlite_thread_guard(SQL.act_lti_child_ct_get);

            SQLite::bind(*sql, pLTI_ID);

            assert(sql->executeStep());
            num_edges = sql->getColumn(0).getUInt64();
        })->wait();
    }

    // only if augmentation count is less than threshold do we associate with edges
    if (num_edges < static_cast<uint64_t>(settings->thresh->get_value()))
    {
        // activation_value=? WHERE lti=?
        JobQueue.post([&]() {
            auto sql = sqlite_thread_guard(SQL.act_set);

            SQLite::bind(*sql, new_activation, pLTI_ID);
            sql->exec();
        })->wait();
    }

    // always associate activation with lti
    // activation_value=? WHERE lti=?
    JobQueue.post([&]() {
        auto sql = sqlite_thread_guard(SQL.act_lti_set);

        SQLite::bind(*sql, new_activation, pLTI_ID);
        sql->exec();
    })->wait();

    ////////////////////////////////////////////////////////////////////////////
    timers->act->stop();
    ////////////////////////////////////////////////////////////////////////////

    return new_activation;
}
