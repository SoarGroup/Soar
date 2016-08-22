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

double SMem_Manager::smem_lti_calc_base(smem_lti_id lti, int64_t time_now, uint64_t n, uint64_t activations_first)
{
    double sum = 0.0;
    double d = smem_params->base_decay->get_value();
    uint64_t t_k;
    uint64_t t_n = (time_now - activations_first);

    if (n == 0)
    {
        smem_stmts->lti_access_get->bind_int(1, lti);
        smem_stmts->lti_access_get->execute();

        n = smem_stmts->lti_access_get->column_int(0);
        activations_first = smem_stmts->lti_access_get->column_int(2);

        smem_stmts->lti_access_get->reinitialize();
    }

    // get all history
    smem_stmts->history_get->bind_int(1, lti);
    smem_stmts->history_get->execute();
    {
        int available_history = static_cast<int>((SMEM_ACT_HISTORY_ENTRIES < n) ? (SMEM_ACT_HISTORY_ENTRIES) : (n));
        t_k = static_cast<uint64_t>(time_now - smem_stmts->history_get->column_int(available_history - 1));

        for (int i = 0; i < available_history; i++)
        {
            sum += pow(static_cast<double>(time_now - smem_stmts->history_get->column_int(i)),
                       static_cast<double>(-d));
        }
    }
    smem_stmts->history_get->reinitialize();

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
double SMem_Manager::smem_lti_activate(smem_lti_id lti, bool add_access, uint64_t num_edges)
{
    ////////////////////////////////////////////////////////////////////////////
    smem_timers->act->start();
    ////////////////////////////////////////////////////////////////////////////

    int64_t time_now;
    if (add_access)
    {
        time_now = smem_max_cycle++;

        if ((smem_params->activation_mode->get_value() == smem_param_container::act_base) &&
                (smem_params->base_update->get_value() == smem_param_container::bupt_incremental))
        {
            int64_t time_diff;

            for (std::set< int64_t >::iterator b = smem_params->base_incremental_threshes->set_begin(); b != smem_params->base_incremental_threshes->set_end(); b++)
            {
                if (*b > 0)
                {
                    time_diff = (time_now - *b);

                    if (time_diff > 0)
                    {
                        std::list< smem_lti_id > to_update;

                        smem_stmts->lti_get_t->bind_int(1, time_diff);
                        while (smem_stmts->lti_get_t->execute() == soar_module::row)
                        {
                            to_update.push_back(static_cast< smem_lti_id >(smem_stmts->lti_get_t->column_int(0)));
                        }
                        smem_stmts->lti_get_t->reinitialize();

                        for (std::list< smem_lti_id >::iterator it = to_update.begin(); it != to_update.end(); it++)
                        {
                            smem_lti_activate((*it), false);
                        }
                    }
                }
            }
        }
    }
    else
    {
        time_now = smem_max_cycle;

        smem_stats->act_updates->set_value(smem_stats->act_updates->get_value() + 1);
    }

    // access information
    uint64_t prev_access_n = 0;
    uint64_t prev_access_t = 0;
    uint64_t prev_access_1 = 0;
    {
        // get old (potentially useful below)
        {
            smem_stmts->lti_access_get->bind_int(1, lti);
            smem_stmts->lti_access_get->execute();

            prev_access_n = smem_stmts->lti_access_get->column_int(0);
            prev_access_t = smem_stmts->lti_access_get->column_int(1);
            prev_access_1 = smem_stmts->lti_access_get->column_int(2);

            smem_stmts->lti_access_get->reinitialize();
        }

        // set new
        if (add_access)
        {
            smem_stmts->lti_access_set->bind_int(1, (prev_access_n + 1));
            smem_stmts->lti_access_set->bind_int(2, time_now);
            smem_stmts->lti_access_set->bind_int(3, ((prev_access_n == 0) ? (time_now) : (prev_access_1)));
            smem_stmts->lti_access_set->bind_int(4, lti);
            smem_stmts->lti_access_set->execute(soar_module::op_reinit);
        }
    }

    // get new activation value (depends upon bias)
    double new_activation = 0.0;
    smem_param_container::act_choices act_mode = smem_params->activation_mode->get_value();
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
                smem_stmts->history_add->bind_int(1, lti);
                smem_stmts->history_add->bind_int(2, time_now);
                smem_stmts->history_add->execute(soar_module::op_reinit);
            }

            new_activation = 0;
        }
        else
        {
            if (add_access)
            {
                smem_stmts->history_push->bind_int(1, time_now);
                smem_stmts->history_push->bind_int(2, lti);
                smem_stmts->history_push->execute(soar_module::op_reinit);
            }

            new_activation = smem_lti_calc_base(lti, time_now + ((add_access) ? (1) : (0)), prev_access_n + ((add_access) ? (1) : (0)), prev_access_1);
        }
    }

    // get number of augmentations (if not supplied)
    if (num_edges == SMEM_ACT_MAX)
    {
        smem_stmts->act_lti_child_ct_get->bind_int(1, lti);
        smem_stmts->act_lti_child_ct_get->execute();

        num_edges = smem_stmts->act_lti_child_ct_get->column_int(0);

        smem_stmts->act_lti_child_ct_get->reinitialize();
    }

    // only if augmentation count is less than threshold do we associate with edges
    if (num_edges < static_cast<uint64_t>(smem_params->thresh->get_value()))
    {
        // activation_value=? WHERE lti=?
        smem_stmts->act_set->bind_double(1, new_activation);
        smem_stmts->act_set->bind_int(2, lti);
        smem_stmts->act_set->execute(soar_module::op_reinit);
    }

    // always associate activation with lti
    {
        // activation_value=? WHERE lti=?
        smem_stmts->act_lti_set->bind_double(1, new_activation);
        smem_stmts->act_lti_set->bind_int(2, lti);
        smem_stmts->act_lti_set->execute(soar_module::op_reinit);
    }

    ////////////////////////////////////////////////////////////////////////////
    smem_timers->act->stop();
    ////////////////////////////////////////////////////////////////////////////

    return new_activation;
}
