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

double SMem_Manager::lti_calc_base(uint64_t pLTI_ID, int64_t time_now, uint64_t n, uint64_t activations_first)
{
    double sum = 0.0;
    double d = settings->base_decay->get_value();
    uint64_t t_k;
    uint64_t t_n = (time_now - activations_first);
    unsigned int available_history = 0;

    if (n == 0)
    {
        SQL->lti_access_get->bind_int(1, pLTI_ID);
        SQL->lti_access_get->execute();

        n = SQL->lti_access_get->column_double(0);
        activations_first = SQL->lti_access_get->column_int(2);

        SQL->lti_access_get->reinitialize();
    }

    // get all history
    SQL->history_get->bind_int(1, pLTI_ID);
    SQL->history_get->execute();
    //int recent = 0;
    int64_t recent_time = 0;

    double small_n = 0;
    {
        while (SQL->history_get->column_int(available_history) != 0)
        {
            available_history++;
        }
        t_k = static_cast<uint64_t>(time_now - SQL->history_get->column_int(available_history - 1));

        for (int i = 0; i < available_history; i++)
        {
            small_n+=SQL->history_get->column_double(i+10);
            int64_t time_diff = (time_now - SQL->history_get->column_int(i));
            if (i == 0 && n > 0)
            {
                recent_time = time_diff;
            }
            /*if (time_diff < 3)
            {
                recent = time_diff;
            }*/
            sum += SQL->history_get->column_double(i+10)*pow(static_cast<double>(time_now - SQL->history_get->column_int(i)),
                       static_cast<double>(-d));
        }
    }
    SQL->history_get->reinitialize();

    // if available history was insufficient, approximate rest
    if (n > small_n && available_history == SMEM_ACT_HISTORY_ENTRIES)
    {
        if (t_n != t_k)
        {
            double apx_numerator = (static_cast<double>(n - SMEM_ACT_HISTORY_ENTRIES) * (pow(static_cast<double>(t_n), 1.0 - d) - pow(static_cast<double>(t_k), 1.0 - d)));
            double apx_denominator = ((1.0 - d) * static_cast<double>(t_n - t_k));
            sum += (apx_numerator / apx_denominator);
        }
        else
        {
            sum += (n - small_n)*pow(static_cast<double>(t_n),static_cast<double>(-d));
        }
    }
    //return ((sum > 0) ? (log(sum/(1+sum))) : (SMEM_ACT_LOW));
    //return (!recent ? ((sum > 0) ? (log(sum/(1+sum))) : (SMEM_ACT_LOW)) : recent-3);//doing log prob instead of log odds.//hack attempt at short-term inhibitory effects
    double inhibition_odds = 0;
    if (recent_time != 0 && settings->base_inhibition->get_value() == on )// && smem_in_wmem->find(pLTI_ID) != smem_in_wmem->end())
    {
        inhibition_odds = pow(1+pow(recent_time/10.0,-1.0),-1.0);
        return ((sum > 0) ? (log(sum/(1+sum)) + log(inhibition_odds/(1+inhibition_odds))) : (SMEM_ACT_LOW));
    }
    return ((sum > 0) ? (log(sum/(1+sum))) : (SMEM_ACT_LOW));//doing log prob instead of log odds.
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

    int64_t time_now;
    bool prohibited = false;

    //access information
    double prev_access_n = 0;
    uint64_t prev_access_t = 0;
    uint64_t prev_access_1 = 0;
    SQL->lti_access_get->bind_int(1,pLTI_ID);
    SQL->lti_access_get->execute();
    prev_access_n = SQL->lti_access_get->column_double(0);
    prev_access_t = SQL->lti_access_get->column_int(1);
    prev_access_1 = SQL->lti_access_get->column_int(2);
    SQL->lti_access_get->reinitialize();

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
        SQL->prohibit_check->bind_int(1,pLTI_ID);
        prohibited = SQL->prohibit_check->execute()==soar_module::row;
        bool dirty = false;
        if (prohibited)
        {
            dirty = SQL->prohibit_check->column_int(1)==1;
        }
        SQL->prohibit_check->reinitialize();
        if (prohibited)
        {
            //Find the number of touches from the most recent activation and remove that much touching.
            if (dirty)
            {
                SQL->history_get->bind_int(1,pLTI_ID);
                SQL->history_get->execute();
                prev_access_n-=SQL->history_get->column_double(10);
                SQL->history_get->reinitialize();
                SQL->history_remove->bind_int(1,pLTI_ID);
                SQL->history_remove->execute(soar_module::op_reinit);
            }
            SQL->prohibit_reset->bind_int(1,pLTI_ID);
            SQL->prohibit_reset->execute(soar_module::op_reinit);
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
                        std::list< uint64_t > to_update;

                        SQL->lti_get_t->bind_int(1, time_diff);
                        while (SQL->lti_get_t->execute() == soar_module::row)
                        {
                            to_update.push_back(static_cast< uint64_t >(SQL->lti_get_t->column_int(0)));
                        }
                        SQL->lti_get_t->reinitialize();

                        for (std::list< uint64_t >::iterator it = to_update.begin(); it != to_update.end(); it++)
                        {
                            lti_activate((*it), false);
                        }
                    }
                }
            }
        }
        statistics->act_updates->set_value(statistics->act_updates->get_value() + 1);
    }
    else
    {
        /*
         * If we are not adding an access, we need to remove the old history so that recalculation takes into account the prohibit having occurred.
         * The big difference is that we'll have to leave it prohibited, just not dirty. Only an access removes the prohibit.
         * */
        SQL->prohibit_check->bind_int(1,pLTI_ID);
        prohibited = SQL->prohibit_check->execute()==soar_module::row;
        bool dirty = false;
        if (prohibited)
        {
            dirty = SQL->prohibit_check->column_int(1)==1;
        }
        SQL->prohibit_check->reinitialize();
        if (prohibited && dirty)
        {
            //remove the touches from that prohibited access.
            SQL->history_get->bind_int(1, pLTI_ID);
            SQL->history_get->execute();
            prev_access_n-=SQL->history_get->column_double(10);
            SQL->history_get->reinitialize();
            //And remove the history entry as well.
            SQL->history_remove->bind_int(1,pLTI_ID);
            SQL->history_remove->execute(soar_module::op_reinit);
            SQL->prohibit_clean->bind_int(1,pLTI_ID);
            SQL->prohibit_clean->execute(soar_module::op_reinit);
        }
        time_now = smem_max_cycle;
        statistics->act_updates->set_value(statistics->act_updates->get_value() + 1);
    }
    {//Whether or not we added an access and whether or not we had previous accesses determines what updated form we give our access history.
        SQL->lti_access_set->bind_double(1, (prev_access_n + (add_access ? touches : 0.0)));
        SQL->lti_access_set->bind_int(2, add_access ? time_now : prev_access_t);
        SQL->lti_access_set->bind_int(3, prev_access_n == 0 ? (add_access ? time_now : 0) : prev_access_1);
        SQL->lti_access_set->bind_int(4, pLTI_ID);
        SQL->lti_access_set->execute(soar_module::op_reinit);
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
        if (prev_access_1 == 0)
        {
            if (add_access)
            {
                if (prohibited)
                {
                    SQL->history_push->bind_int(1,time_now);
                    SQL->history_push->bind_double(2,touches);
                    SQL->history_push->bind_int(3,pLTI_ID);
                    SQL->history_push->execute(soar_module::op_reinit);
                }
                else
                {
                    SQL->history_add->bind_int(1,pLTI_ID);
                    SQL->history_add->bind_int(2,time_now);
                    SQL->history_add->bind_double(3,touches);
                    SQL->history_add->execute(soar_module::op_reinit);
                }
            }
            new_activation = lti_calc_base(pLTI_ID, time_now + ((add_access) ? (1) : (0)), prev_access_n + ((add_access) ? (touches) : (0)), prev_access_1);
        }
        else
        {
            if (add_access)
            {
                SQL->history_push->bind_int(1, time_now);
                SQL->history_push->bind_double(2, touches);
                SQL->history_push->bind_int(3, pLTI_ID);
                SQL->history_push->execute(soar_module::op_reinit);
            }

            new_activation = lti_calc_base(pLTI_ID, time_now + ((add_access) ? (1) : (0)), prev_access_n + (add_access ? touches : 0), prev_access_1);
        }
    }
    // get number of augmentations (if not supplied)
    if (num_edges == SMEM_ACT_MAX)
    {
        SQL->act_lti_child_ct_get->bind_int(1, pLTI_ID);
        SQL->act_lti_child_ct_get->execute();

        num_edges = SQL->act_lti_child_ct_get->column_int(0);

        SQL->act_lti_child_ct_get->reinitialize();
    }

    //need a denominator for spreading:
    double baseline_denom = settings->spreading_continue_probability->get_value();
    double decay_const = baseline_denom;
    int depth_limit = settings->spreading_depth_limit->get_value();
    for (int i = 0; i < depth_limit; ++i)
    {//representing a maximum possible spreading value for comparison. It's an admissible heuristic. (fancy word!)
        baseline_denom = baseline_denom + baseline_denom*decay_const;
    }
    double spread = 0;
    double modified_spread = 0;
    double new_base;
    bool already_in_spread_table = false;
    std::unordered_map<uint64_t, int64_t>* spreaded_to = smem_spreaded_to;
    if (settings->spreading->get_value() == on && spreaded_to->find(pLTI_ID) != spreaded_to->end() && (*spreaded_to)[pLTI_ID] != 0)
    {
        already_in_spread_table = true;
        SQL->act_lti_fake_get->bind_int(1,pLTI_ID);
        SQL->act_lti_fake_get->execute();
        spread = SQL->act_lti_fake_get->column_double(1);
        SQL->act_lti_fake_get->reinitialize();
    }
    if (static_cast<double>(new_activation)==static_cast<double>(SMEM_ACT_LOW) || static_cast<double>(new_activation)==0)
    {//When we have nothing to go on, we pretend the base-level of the memory is equivalent to having been accessed once at the time the agent was created.
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
        SQL->act_lti_fake_set->bind_double(1, new_activation);
        SQL->act_lti_fake_set->bind_double(2, spread);
        SQL->act_lti_fake_set->bind_double(3, new_base + modified_spread);
        SQL->act_lti_fake_set->bind_int(4,pLTI_ID);
        SQL->act_lti_fake_set->execute(soar_module::op_reinit);
        //SQL->act_set->bind_double(1, SMEM_ACT_LOW);
        //SQL->act_set->bind_int(2, pLTI_ID);
        //SQL->act_set->execute(soar_module::op_reinit);
    }
    else
    {
        SQL->act_lti_set->bind_double(1, new_activation);
        SQL->act_lti_set->bind_double(2, 0.0);
        SQL->act_lti_set->bind_double(3, new_base);
        SQL->act_lti_set->bind_int(4, pLTI_ID);
        SQL->act_lti_set->execute(soar_module::op_reinit);
    }
    if (num_edges < static_cast<uint64_t>(settings->thresh->get_value()) && !already_in_spread_table)
    {
        SQL->act_set->bind_double(1, new_base+modified_spread);
        SQL->act_set->bind_int(2, pLTI_ID);
        SQL->act_set->execute(soar_module::op_reinit);
    }
    else if (num_edges >= static_cast<uint64_t>(settings->thresh->get_value()) && !already_in_spread_table)
    {
        SQL->act_set->bind_double(1, SMEM_ACT_MAX);
        SQL->act_set->bind_int(2, pLTI_ID);
        SQL->act_set->execute(soar_module::op_reinit);
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

    return new_base+modified_spread;
}

void SMem_Manager::child_spread(uint64_t lti_id, std::map<uint64_t, std::list<std::pair<uint64_t,double>>*>& lti_trajectories, int depth = 10)
{
    if (lti_trajectories.find(lti_id) == lti_trajectories.end())
    {//If we don't already have the children and their edge weights, we need to get them.
        soar_module::sqlite_statement* children_q = SQL->web_val_child;
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
            // bool prohibited = false;
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
                    children_q->bind_int(1, lti_id);
                    //children_q->bind_int(2, lti_id);
                    while (children_q->execute() == soar_module::row)
                    {
                        /*if (settings->spreading_loop_avoidance->get_value() == on && children_q->column_int(0) == lti_id)
                        {
                            continue;
                        }*///We actually do want the edge weight to a self-edge to adjust even if we don't use it.
                        old_edge_weight_map_for_children[(uint64_t)(children_q->column_int(0))] = children_q->column_double(1);
                        edge_weight_update_map_for_children[(uint64_t)(children_q->column_int(0))] = 0;
                    }
                    children_q->reinitialize();
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
            soar_module::sqlite_statement* update_edge = SQL->web_update_child_edge;
            for (updates_it = updates_begin; updates_it != old_edge_weight_map_for_children.end(); ++updates_it)
            {// args are edge weight, parent lti it, child lti id.
                update_edge->bind_double(1, updates_it->second);
                update_edge->bind_int(2, lti_id);
                update_edge->bind_int(3, updates_it->first);
                update_edge->execute(soar_module::op_reinit);
            }
            for (edge_it = edge_begin_it; edge_it != edge_updates->end(); ++edge_it)
            {
                delete (*edge_it);
            }
            smem_edges_to_update->erase(lti_id);
        }
        children_q->bind_int(1, lti_id);
        //children_q->bind_int(2, lti_id);
        lti_trajectories[lti_id] = new std::list<std::pair<uint64_t, double>>;
        while (children_q->execute() == soar_module::row)
        {
            /*if (children_q->column_int(0) == lti_id)
            {
                continue;
            }*/
            (lti_trajectories[lti_id])->push_back(std::make_pair((uint64_t)(children_q->column_int(0)),children_q->column_double(1)));
            children.push_back(children_q->column_int(0));
        }
        children_q->reinitialize();
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
        SQL->trajectory_remove_lti->bind_int(1,lti_id);
        SQL->trajectory_remove_lti->execute(soar_module::op_reinit);
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
    current_lti_list->emplace_back(lti_id, 1.0);
    double initial_activation = 1.0;
    lti_traversal_queue.emplace(initial_activation,current_lti_list);
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
                new_list->emplace_back(old_list_iterator->first,old_list_iterator->second);
                if (settings->spreading_loop_avoidance->get_value() == on)
                {
                    visited.insert(std::make_pair(old_list_iterator->first,old_list_iterator->second));//We we have loop avoidance on, we need to keep track of the old path elements.
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
            new_list->emplace_back(lti_iterator->first,lti_iterator->second);
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
            for (new_list_iterator = new_list_iterator_begin; new_list_iterator != new_list_iterator_end && depth < (depth_limit + 2); ++new_list_iterator)
            {
                SQL->trajectory_add->bind_int(++depth, new_list_iterator->first);
            }//We add the amount of traversal we have.
            while (depth < 11)
            {//And we pad unused columns with 0. This helps the indexing ignore these columns later. I could maybe do the same with NULL.
                //It depends on the specifics of partial indexing in sqlite... Point is - I know this works for that efficiency gain.
                SQL->trajectory_add->bind_int(++depth, 0);
            }
            SQL->trajectory_add->execute(soar_module::op_reinit);
            //For later use, this is bookkeeping:
            ever_added = true;
            ++count;
            //If we still have room for more, we add the new path to the p-queue for additional traversal.
            if (new_list->size() < depth_limit + 1 && count < limit &&  decay_prob*initial_activation*lti_iterator->second > baseline_prob)
            {//if we aren't at the depth limit, the total traversal size limit, and the activation is big enough
                lti_traversal_queue.emplace(initial_activation*lti_iterator->second,new_list);
            }
            else
            {
                delete new_list;//No need to keep the copy+1more otherwise.
                new_list = NULL;
            }
        }
        //lti_traversal_queue.pop();//Get rid of the old list.
        delete current_lti_list;//no longer need it.
    }
    //Once we've generated the full spread map of accumulated spread for recipients from this source, we record it.
    for (std::map<uint64_t,double>::iterator spread_map_it = spread_map.begin(); spread_map_it != spread_map.end(); ++spread_map_it)
    {
        SQL->likelihood_cond_count_insert->bind_int(1,lti_id);
        SQL->likelihood_cond_count_insert->bind_int(2,spread_map_it->first);
        SQL->likelihood_cond_count_insert->bind_double(3,spread_map_it->second);
        SQL->likelihood_cond_count_insert->execute(soar_module::op_reinit);
    }
    //In the special case where we don't ever add anything, we need to insert all zeros as the traversal.
    if (!ever_added)
    {
        SQL->trajectory_add->bind_int(1,lti_id);
        SQL->trajectory_add->bind_int(2,0);
        SQL->trajectory_add->bind_int(3,0);
        SQL->trajectory_add->bind_int(4,0);
        SQL->trajectory_add->bind_int(5,0);
        SQL->trajectory_add->bind_int(6,0);
        SQL->trajectory_add->bind_int(7,0);
        SQL->trajectory_add->bind_int(8,0);
        SQL->trajectory_add->bind_int(9,0);
        SQL->trajectory_add->bind_int(10,0);
        SQL->trajectory_add->bind_int(11,0);
        SQL->trajectory_add->bind_int(12,1);
        SQL->trajectory_add->execute(soar_module::op_reinit);
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
    attach();
    soar_module::sqlite_statement* lti_all = SQL->lti_all;
    uint64_t lti_id;
    //smem_delete_trajectory_indices();//This is for efficiency.
    //It's super inefficient to maintain the database indexing during this batch processing
    //It's way better to delete and rebuild. However, for testing and small DBs, it's fine. I'm testing... so... it's commented for now.
    // - scijones (Yell at me if you see this.)
    std::map<uint64_t, std::list<std::pair<uint64_t, double>>*> lti_trajectories;
    while (lti_all->execute() == soar_module::row)
    {//loop over all ltis.
        lti_id = lti_all->column_int(0);
        trajectory_construction(lti_id,lti_trajectories,0,true);
    }
    lti_all->reinitialize();
    //smem_create_trajectory_indices();//TODO: Fix this and the above commend about it. YELL AT ME.
    //Cleanup the map.
    for (std::map<uint64_t,std::list<std::pair<uint64_t, double>>*>::iterator to_delete = lti_trajectories.begin(); to_delete != lti_trajectories.end(); ++to_delete)
    {
        delete to_delete->second;
    }
    soar_module::sqlite_statement* lti_count_num_appearances = new soar_module::sqlite_statement(DB,
            "INSERT INTO smem_trajectory_num (lti_id, num_appearances) SELECT lti_j, SUM(num_appearances_i_j) FROM smem_likelihoods GROUP BY lti_j");
    lti_count_num_appearances->prepare();
    lti_count_num_appearances->execute(soar_module::op_reinit);
    delete lti_count_num_appearances;
}

inline soar_module::sqlite_statement* SMem_Manager::setup_manual_web_crawl(smem_weighted_cue_element* el, uint64_t lti_id)
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
}

void SMem_Manager::calc_spread(std::set<uint64_t>* current_candidates, bool do_manual_crawl, smem_weighted_cue_list::iterator* cand_set)
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
            SQL->trajectory_check_invalid->bind_int(1,*it);
            SQL->trajectory_get->bind_int(1,*it);
            bool was_invalid = (SQL->trajectory_check_invalid->execute() == soar_module::row);
            //If the previous trajectory is no longer valid because of a change to memory or we don't have a trajectory, we might need to remove
            //the old one.
            bool no_trajectory = SQL->trajectory_get->execute() != soar_module::row;
            SQL->trajectory_check_invalid->reinitialize();
            SQL->trajectory_get->reinitialize();
            if (was_invalid || no_trajectory)
            {
                //We also need to make a new one.
                if (was_invalid)
                {
                    SQL->likelihood_cond_count_remove->bind_int(1,(*it));
                    SQL->likelihood_cond_count_remove->execute(soar_module::op_reinit);
                    SQL->lti_count_num_appearances_remove->bind_int(1,(*it));
                    SQL->lti_count_num_appearances_remove->execute(soar_module::op_reinit);
                }
                trajectory_construction(*it,lti_trajectories);
                //statistics->expansions->set_value(statistics->expansions->get_value() + 1);
                //smem_calc_likelihoods_for_det_trajectories(thisAgent, (*it));

                SQL->lti_count_num_appearances_insert->bind_int(1,(*it));
                SQL->lti_count_num_appearances_insert->execute(soar_module::op_reinit);
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
    soar_module::sqlite_statement* add_fingerprint = SQL->add_fingerprint;
    soar_module::sqlite_statement* select_fingerprint = SQL->select_fingerprint;
    for (std::set<uint64_t>::iterator it = smem_context_additions->begin(); it != smem_context_additions->end(); ++it)
    {//Now we add the walks/traversals we've done. //can imagine doing this as a batch process through a join on a list of the additions if need be.
        select_fingerprint->bind_int(1,(*it));
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
        select_fingerprint->reinitialize();
        //I need to split this into separate select and insert batches. The select will allow me to keep an in-memory record of
        //potential spread recipients. The insert is then the normal insert. A select/insert combo would be nice, but that doesn't
        //make sense with the sqlite api.

    }
    ////////////////////////////////////////////////////////////////////////////
    timers->spreading_3->stop();
    ////////////////////////////////////////////////////////////////////////////
    //soar_module::sqlite_statement* add_uncommitted_fingerprint = SQL->add_uncommitted_fingerprint;
    //soar_module::sqlite_statement* remove_fingerprint_reversal = SQL->remove_fingerprint_reversal;
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
    soar_module::sqlite_statement* delete_old_spread = SQL->delete_old_spread;
    // soar_module::sqlite_statement* delete_old_uncommitted_spread = SQL->delete_old_uncommitted_spread;
    // soar_module::sqlite_statement* reverse_old_committed_spread = SQL->reverse_old_committed_spread;
    //delete_old_spread->prepare();
    std::unordered_map<uint64_t,int64_t>* spreaded_to = smem_spreaded_to;
    std::set<uint64_t>::iterator recipient_it;
    std::set<uint64_t>::iterator recipient_begin;
    std::set<uint64_t>::iterator recipient_end;
    for (std::set<uint64_t>::iterator source_it = smem_context_removals->begin(); source_it != smem_context_removals->end(); ++source_it)
    {
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
                        SQL->act_lti_fake_get->bind_int(1,*recipient_it);
                        SQL->act_lti_fake_get->execute();
                        // double spread = SQL->act_lti_fake_get->column_double(1);//This is the spread before changes.
                        double prev_base = SQL->act_lti_fake_get->column_double(0);
                        SQL->act_lti_fake_get->reinitialize();
                        SQL->act_lti_fake_delete->bind_int(1, *recipient_it);
                        SQL->act_lti_fake_delete->execute(soar_module::op_reinit);
                        SQL->act_lti_set->bind_double(1, ((static_cast<double>(prev_base)==0) ? (SMEM_ACT_LOW):(prev_base)));
                        SQL->act_lti_set->bind_double(2, 0);
                        SQL->act_lti_set->bind_double(3, prev_base);
                        SQL->act_lti_set->bind_int(4, *recipient_it);
                        SQL->act_lti_set->execute(soar_module::op_reinit);
                        spreaded_to->erase(*recipient_it);
                        SQL->act_set->bind_double(1, prev_base);
                        SQL->act_set->bind_int(2, *recipient_it);
                        SQL->act_set->execute(soar_module::op_reinit);

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
        delete_old_spread->bind_int(1,(*source_it));
        delete_old_spread->execute(soar_module::op_reinit);
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
    double spread = 0;
    std::set<uint64_t> pruned_candidates;
    //soar_module::sqlite_statement* list_uncommitted_spread = SQL->list_uncommitted_spread;
    soar_module::sqlite_statement* list_current_spread = SQL->list_current_spread;
   //do_manual_crawl = true;
    if (do_manual_crawl)
    {//This means that the candidate set was quite large, so we instead manually check the sql store for candidacy.
        soar_module::sqlite_statement* q_manual;
        while (list_current_spread->execute() == soar_module::row)
        {//we loop over all spread sinks
            q_manual = setup_manual_web_crawl(**cand_set, list_current_spread->column_int(0));
            if (q_manual->execute() == soar_module::row)//and if the sink is a candidate, we will actually calculate on it later.
            {
                pruned_candidates.insert(list_current_spread->column_int(0));
            }
            q_manual->reinitialize();
        }
    }
    list_current_spread->reinitialize();
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

    soar_module::sqlite_statement* calc_current_spread = SQL->calc_current_spread;
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
            SQL->act_lti_fake_get->bind_int(1,*candidate);
            SQL->act_lti_fake_get->execute();
            // double spread = SQL->act_lti_fake_get->column_double(1);//This is the spread before changes.
            double prev_base = SQL->act_lti_fake_get->column_double(0);
            SQL->act_lti_fake_get->reinitialize();
            SQL->act_lti_fake_delete->bind_int(1, *candidate);
            SQL->act_lti_fake_delete->execute(soar_module::op_reinit);
            SQL->act_lti_set->bind_double(1, ((static_cast<double>(prev_base)==0) ? (SMEM_ACT_LOW):(prev_base)));
            SQL->act_lti_set->bind_double(2, 0);
            SQL->act_lti_set->bind_double(3, prev_base);
            SQL->act_lti_set->bind_int(4, *candidate);
            SQL->act_lti_set->execute(soar_module::op_reinit);
            //SQL->act_lti_fake_get->reinitialize();
            spreaded_to->erase(*candidate);
        }
        ////////////////////////////////////////////////////////////////////////////
        timers->spreading_7_1->stop();
        ////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////
        timers->spreading_7_2->start();
        ////////////////////////////////////////////////////////////////////////////
        calc_current_spread->bind_int(1,(*candidate));
        while (calc_current_spread->execute() == soar_module::row && calc_current_spread->column_double(2))
        {
            //First, I need to get the existing info for this lti_id.
            bool already_in_spread_table = false;

            bool addition = (((int)(calc_current_spread->column_int(3))) == 1);
            if (addition)
            {

                if (spreaded_to->find(*candidate) == spreaded_to->end())//(updated_candidates.find(*candidate) == updated_candidates.end())
                {
                    ////////////////////////////////////////////////////////////////////////////
                    timers->spreading_7_2_1->start();
                    ////////////////////////////////////////////////////////////////////////////
                    (*spreaded_to)[*candidate] = 1;
                    SQL->act_lti_get->bind_int(1,*candidate);
                    SQL->act_lti_get->execute();
                    spread = SQL->act_lti_get->column_double(1);//This is the spread before changes.
                    prev_base = SQL->act_lti_get->column_double(0);
                    SQL->act_lti_get->reinitialize();
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
                    SQL->act_lti_fake_get->bind_int(1,*candidate);
                    SQL->act_lti_fake_get->execute();
                    spread = SQL->act_lti_fake_get->column_double(1);//This is the spread before changes.
                    prev_base = SQL->act_lti_fake_get->column_double(0);
                    SQL->act_lti_fake_get->reinitialize();
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
                auto wmas = smem_wmas->equal_range(calc_current_spread->column_int(4));
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
                    raw_prob = wma_multiplicative_factor*(((double)(calc_current_spread->column_double(2)))/(calc_current_spread->column_double(1)));
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
                SQL->act_lti_child_ct_get->bind_int(1, *candidate);
                SQL->act_lti_child_ct_get->execute();

                SQL->act_lti_child_ct_get->reinitialize();
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
                    SQL->act_lti_fake_set->bind_double(1, ((static_cast<double>(prev_base)==0) ? (SMEM_ACT_LOW):(prev_base)));
                    SQL->act_lti_fake_set->bind_double(2, spread);
                    SQL->act_lti_fake_set->bind_double(3, modified_spread+ new_base);
                    SQL->act_lti_fake_set->bind_int(4, *candidate);
                    SQL->act_lti_fake_set->execute(soar_module::op_reinit);
                    ////////////////////////////////////////////////////////////////////////////
                    timers->spreading_7_2_7->stop();
                    ////////////////////////////////////////////////////////////////////////////
                }
                else
                {
                    ////////////////////////////////////////////////////////////////////////////
                    timers->spreading_7_2_8->start();
                    ////////////////////////////////////////////////////////////////////////////
                    SQL->act_lti_fake_insert->bind_int(1, *candidate);
                    SQL->act_lti_fake_insert->bind_double(2, ((static_cast<double>(prev_base)==0) ? (SMEM_ACT_LOW):(prev_base)));
                    SQL->act_lti_fake_insert->bind_double(3, spread);
                    SQL->act_lti_fake_insert->bind_double(4, modified_spread+ new_base);
                    SQL->act_lti_fake_insert->execute(soar_module::op_reinit);

                    //In order to prevent the activation from the augmentations table from coming into play after this has been given spread, we set the augmentations bla to be smemactlow
                    /*SQL->act_set->bind_double(1, SMEM_ACT_LOW);
                    SQL->act_set->bind_int(2, *candidate);
                    SQL->act_set->execute(soar_module::op_reinit);*/
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
        calc_current_spread->reinitialize();
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
            for (int i = 1; i < 11; i++)
            {
                SQL->trajectory_invalidate_from_lti->bind_int(i, lti_parent_id);
                //As it turns out, sqlite is smart about unioning ors that all have a single index.
            }
            SQL->trajectory_invalidate_from_lti->execute(soar_module::op_reinit);
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
        for (int i = 1; i < 11; i++)
        {
            SQL->trajectory_invalidate_edge->bind_int(2*i-1, lti_parent_id);
            SQL->trajectory_invalidate_edge->bind_int(1*i, negative_children->front());
        }
        SQL->trajectory_invalidate_edge->execute(soar_module::op_reinit);
        negative_children->pop_front();
    }
    delete negative_children;
}

void SMem_Manager::invalidate_from_lti(uint64_t invalid_parent)
{
    for (int i = 1; i < 11; i++)
    {//A changing edge weight is treated as an invalidation of cases that could have used that edge.
        SQL->trajectory_invalidate_from_lti->bind_int(i,invalid_parent);
    }
    SQL->trajectory_invalidate_from_lti->execute(soar_module::op_reinit);
}

void SMem_Manager::add_to_invalidate_from_lti_table(uint64_t invalid_parent)
{
    SQL->trajectory_invalidate_from_lti_add->bind_int(1, invalid_parent);
    SQL->trajectory_invalidate_from_lti_add->execute(soar_module::op_reinit);
}

void SMem_Manager::batch_invalidate_from_lti()
{
    if (SQL->trajectory_invalidation_check_for_rows->execute() == soar_module::row)
    {
        SQL->trajectory_invalidate_from_lti_table->execute(soar_module::op_reinit);
        SQL->trajectory_invalidate_from_lti_clear->execute(soar_module::op_reinit);
    }
}
