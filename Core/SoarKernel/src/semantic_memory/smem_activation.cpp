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
        reset_guard r(SQL.lti_access_get);

        SQL.lti_access_get.bind(0, pLTI_ID);
        SQL.lti_access_get.exec();

        n = SQL.lti_access_get.getColumn(0).getInt();
        activations_first = SQL.lti_access_get.getColumn(2).getInt();
    }

    // get all history
    reset_guard r(SQL.history_get);

    SQL.history_get.bind(1, pLTI_ID);
    SQL.history_get.exec();

    int available_history = static_cast<int>((SMEM_ACT_HISTORY_ENTRIES < n) ? (SMEM_ACT_HISTORY_ENTRIES) : (n));
    t_k = static_cast<uint64_t>(time_now - SQL.history_get.getColumn(available_history - 1).getInt());

    for (int i = 0; i < available_history; i++)
    {
        sum += pow(static_cast<double>(time_now - SQL.history_get.getColumn(i).getInt()),
                   static_cast<double>(-d));
    }

    // if available history was insufficient, approximate rest
    if (n > SMEM_ACT_HISTORY_ENTRIES)
    {
        double apx_numerator = (static_cast<double>(n - SMEM_ACT_HISTORY_ENTRIES) * (pow(static_cast<double>(t_n), 1.0 - d) - pow(static_cast<double>(t_k), 1.0 - d)));
        double apx_denominator = ((1.0 - d) * static_cast<double>(t_n - t_k));

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
                        std::list< uint64_t > to_update;

                        reset_guard r(SQL.lti_get_t);
                        SQL.lti_get_t.bind(1, time_diff);

                        while (SQL.lti_get_t.executeStep())
                            to_update.push_back(SQL.lti_get_t.getColumn(0).getInt());

                        for (std::list< uint64_t >::iterator it = to_update.begin(); it != to_update.end(); it++)
                        {
                            lti_activate((*it), false);
                        }
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

    {
        // get old (potentially useful below)
        reset_guard r(SQL.lti_access_get);

        SQL.lti_access_get.bind(1, pLTI_ID);
        SQL.lti_access_get.exec();

        prev_access_n = SQL.lti_access_get.getColumn(0).getInt();
        prev_access_t = SQL.lti_access_get.getColumn(1).getInt();
        prev_access_1 = SQL.lti_access_get.getColumn(2).getInt();

        // set new
        if (add_access)
        {
            reset_guard r(SQL.lti_access_set);

            SQLite::bind(SQL.lti_access_set,
                 prev_access_n + 1,
                 time_now,
                 ((prev_access_n == 0) ? (time_now) : (prev_access_1)),
                 pLTI_ID);

            SQL.lti_access_set.exec();
        }
    }

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
                reset_guard r(SQL.history_add);

                SQLite::bind(SQL.history_add, pLTI_ID, time_now);
                SQL.history_add.exec();
            }

            new_activation = lti_calc_base(pLTI_ID, time_now + ((add_access) ? (1) : (0)), prev_access_n + ((add_access) ? (1) : (0)), prev_access_1);;
        }
        else
        {
            if (add_access)
            {
                reset_guard r(SQL.history_push);

                SQLite::bind(SQL.history_push, time_now, pLTI_ID);
                SQL.history_push.exec();
            }

            new_activation = lti_calc_base(pLTI_ID, time_now + ((add_access) ? (1) : (0)), prev_access_n + ((add_access) ? (1) : (0)), prev_access_1);
        }
    }

    // get number of augmentations (if not supplied)
    if (num_edges == SMEM_ACT_MAX)
    {
        reset_guard r(SQL.act_lti_child_ct_get);
        SQLite::bind(SQL.act_lti_child_ct_get, pLTI_ID);

        SQL.act_lti_child_ct_get.exec();
        num_edges = SQL.act_lti_child_ct_get.getColumn(0).getInt();
    }

    // only if augmentation count is less than threshold do we associate with edges
    if (num_edges < static_cast<uint64_t>(settings->thresh->get_value()))
    {
        // activation_value=? WHERE lti=?
        reset_guard r(SQL.act_set);

        SQLite::bind(SQL.act_set, new_activation, pLTI_ID);
        SQL.act_set.exec();
    }

    // always associate activation with lti
    {
        // activation_value=? WHERE lti=?
        reset_guard r(SQL.act_lti_set);
        SQLite::bind(SQL.act_lti_set, new_activation, pLTI_ID);
        SQL.act_lti_set.exec();
    }

    ////////////////////////////////////////////////////////////////////////////
    timers->act->stop();
    ////////////////////////////////////////////////////////////////////////////

    return new_activation;
}
