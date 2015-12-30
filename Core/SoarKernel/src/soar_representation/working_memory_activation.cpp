/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  wma.cpp
 *
 * =======================================================================
 * Description  :  Various functions for WMA
 * =======================================================================
 */

#include <soar_representation/agent.h>
#include <soar_representation/condition.h>
#include <soar_representation/working_memory_activation.h>
#include "decide.h"
#include "instantiations.h"
#include "misc.h"
#include "rete.h"
#include "prefmem.h"
#include "print.h"
#include "wmem.h"
#include "xml.h"

#include <set>
#include <cmath>
#include <cstdlib>


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Bookmark strings to help navigate the code
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// parameters                   wma::param
// stats                        wma::stats
// timers                       wma::timers
//
// initialization               wma::init
//
// decay                        wma::decay
// forgetting                   wma::forget
// update                       wma::update
//
// api                          wma::api


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parameter Functions (wma::params)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void wma_init(agent* thisAgent);
void wma_deinit(agent* thisAgent);


wma_activation_param::wma_activation_param(const char* new_name, boolean new_value, soar_module::predicate<boolean>* new_prot_pred, agent* new_agent): soar_module::boolean_param(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

void wma_activation_param::set_value(boolean new_value)
{
    if (new_value != value)
    {
        value = new_value;

        if (new_value == on)
        {
            wma_init(thisAgent);
        }
        else
        {
            wma_deinit(thisAgent);
        }
    }
}

//

wma_decay_param::wma_decay_param(const char* new_name, double new_value, soar_module::predicate<double>* new_val_pred, soar_module::predicate<double>* new_prot_pred): soar_module::decimal_param(new_name, new_value, new_val_pred, new_prot_pred) {}

void wma_decay_param::set_value(double new_value)
{
    value = -new_value;
}

//

template <typename T>
wma_activation_predicate<T>::wma_activation_predicate(agent* new_agent): soar_module::agent_predicate<T>(new_agent) {};

template <typename T>
bool wma_activation_predicate<T>::operator()(T /*val*/)
{
    return wma_enabled(this->thisAgent);
};

//

wma_param_container::wma_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    // WMA on/off
    activation = new wma_activation_param("activation", off, new soar_module::f_predicate<boolean>(), new_agent);
    add(activation);

    // decay-rate
    decay_rate = new wma_decay_param("decay-rate", -0.5, new soar_module::btw_predicate<double>(0, 1, true), new wma_activation_predicate<double>(thisAgent));
    add(decay_rate);

    // decay-thresh
    decay_thresh = new wma_decay_param("decay-thresh", -2.0, new soar_module::gt_predicate<double>(0, false), new wma_activation_predicate<double>(thisAgent));
    add(decay_thresh);

    // do we compute an approximation of the distant references?
    petrov_approx = new soar_module::boolean_param("petrov-approx", off, new wma_activation_predicate<boolean>(thisAgent));
    add(petrov_approx);

    // are WMEs removed from WM when activation gets too low?
    forgetting = new soar_module::constant_param<forgetting_choices>("forgetting", disabled, new wma_activation_predicate<forgetting_choices>(thisAgent));
    forgetting->add_mapping(disabled, "off");
    forgetting->add_mapping(naive, "naive");
    forgetting->add_mapping(bsearch, "bsearch");
    forgetting->add_mapping(approx, "on");
    add(forgetting);

    // which WMEs are removed?
    forget_wme = new soar_module::constant_param<forget_wme_choices>("forget-wme", all, new wma_activation_predicate<forget_wme_choices>(thisAgent));
    forget_wme->add_mapping(all, "all");
    forget_wme->add_mapping(lti, "lti");
    add(forget_wme);

    // fake forgetting?
    fake_forgetting = new soar_module::boolean_param("fake-forgetting", off, new wma_activation_predicate<boolean>(thisAgent));
    add(fake_forgetting);

    // timer level
    timers = new soar_module::constant_param< soar_module::timer::timer_level >("timers", soar_module::timer::zero, new soar_module::f_predicate< soar_module::timer::timer_level >());
    timers->add_mapping(soar_module::timer::zero, "off");
    timers->add_mapping(soar_module::timer::one, "one");
    add(timers);

    // max size of power cache
    max_pow_cache = new soar_module::integer_param("max-pow-cache", 10, new soar_module::gt_predicate< int64_t >(0, false), new wma_activation_predicate< int64_t >(thisAgent));
    add(max_pow_cache);
};

//

bool wma_enabled(agent* thisAgent)
{
    return (thisAgent->wma_params->activation->get_value() == on);
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statistic Functions (wma::stats)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

wma_stat_container::wma_stat_container(agent* new_agent): soar_module::stat_container(new_agent)
{
    // forgotten-wmes
    forgotten_wmes = new soar_module::integer_stat("forgotten-wmes", 0, new soar_module::f_predicate<int64_t>());
    add(forgotten_wmes);
};

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Timer Functions (wma::timers)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

wma_timer_container::wma_timer_container(agent* new_agent): soar_module::timer_container(new_agent)
{
    // one
    history = new wma_timer("wma_history", thisAgent, soar_module::timer::one);
    add(history);

    forgetting = new wma_timer("wma_forgetting", thisAgent, soar_module::timer::one);
    add(forgetting);
}

//

wma_timer_level_predicate::wma_timer_level_predicate(agent* new_agent): soar_module::agent_predicate< soar_module::timer::timer_level >(new_agent) {}

bool wma_timer_level_predicate::operator()(soar_module::timer::timer_level val)
{
    return (thisAgent->wma_params->timers->get_value() >= val);
}

//

wma_timer::wma_timer(const char* new_name, agent* new_agent, soar_module::timer::timer_level new_level): soar_module::timer(new_name, new_agent, new_level, new wma_timer_level_predicate(new_agent)) {}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Initialization Functions (wma::init)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void wma_init(agent* thisAgent)
{
    if (thisAgent->wma_initialized)
    {
        return;
    }

    double decay_rate = thisAgent->wma_params->decay_rate->get_value();
    double decay_thresh = thisAgent->wma_params->decay_thresh->get_value();
    int64_t max_pow_cache = thisAgent->wma_params->max_pow_cache->get_value();

    // Pre-compute the integer powers of the decay exponent in order to avoid
    // repeated calls to pow() at runtime
    {
        // determine cache size
        {
            // computes how many powers to compute
            // basic idea: solve for the time that would just fall below the decay threshold, given decay rate and assumption of max references/decision
            // t = e^( ( thresh - ln( max_refs ) ) / -decay_rate )
            double cache_full = static_cast<double>(exp((decay_thresh - log(static_cast<double>(WMA_REFERENCES_PER_DECISION))) / decay_rate));

            // we bound this by the max-pow-cache parameter to control the space vs. time tradeoff the cache supports
            // max-pow-cache is in MB, so do the conversion:
            // MB * 1024 bytes/KB * 1024 KB/MB
            double cache_bound = (static_cast<unsigned int>(max_pow_cache * 1024 * 1024) / static_cast<unsigned int>(sizeof(double)));

            thisAgent->wma_power_size = static_cast< unsigned int >(ceil((cache_full > cache_bound) ? (cache_bound) : (cache_full)));
        }

        thisAgent->wma_power_array = new double[ thisAgent->wma_power_size ];

        thisAgent->wma_power_array[0] = 0.0;
        for (unsigned int i = 1; i < thisAgent->wma_power_size; i++)
        {
            thisAgent->wma_power_array[ i ] = pow(static_cast<double>(i), decay_rate);
        }
    }

    // calculate the pre-log'd forgetting threshold, to avoid most
    // calls to log
    thisAgent->wma_thresh_exp = exp(decay_thresh);

    // approximation cache
    if (thisAgent->wma_params->forgetting->get_value() == wma_param_container::approx)
    {
        thisAgent->wma_approx_array = new wma_d_cycle[ WMA_REFERENCES_PER_DECISION ];

        thisAgent->wma_approx_array[0] = 0;
        for (int i = 1; i < WMA_REFERENCES_PER_DECISION; i++)
        {
            thisAgent->wma_approx_array[i] = static_cast< wma_d_cycle >(ceil(exp(static_cast<double>(decay_thresh - log(static_cast<double>(i))) / static_cast<double>(decay_rate))));
        }
    }

    // note initialization
    thisAgent->wma_initialized = true;
}

void wma_deinit(agent* thisAgent)
{
    if (!thisAgent->wma_initialized)
    {
        return;
    }

    // release power array memory
    delete[] thisAgent->wma_power_array;

    // release approximation array memory (if applicable)
    if (thisAgent->wma_params->forgetting->get_value() == wma_param_container::approx)
    {
        delete[] thisAgent->wma_approx_array;
    }

    // clear touched
    thisAgent->wma_touched_elements->clear();
    thisAgent->wma_touched_sets->clear();

    // clear forgetting priority queue
    for (wma_forget_p_queue::iterator pq_p = thisAgent->wma_forget_pq->begin(); pq_p != thisAgent->wma_forget_pq->end(); pq_p++)
    {
        pq_p->second->~wma_decay_set();
        thisAgent->memoryManager->free_with_pool(MP_wma_decay_set, pq_p->second);
    }
    thisAgent->wma_forget_pq->clear();

    thisAgent->wma_initialized = false;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Decay Functions (wma::decay)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline unsigned int wma_history_next(unsigned int current)
{
    return ((current == (WMA_DECAY_HISTORY - 1)) ? (0) : (current + 1));
}

inline unsigned int wma_history_prev(unsigned int current)
{
    return ((current == 0) ? (WMA_DECAY_HISTORY - 1) : (current - 1));
}

inline bool wma_should_have_decay_element(wme* w)
{
    return ((w->preference) && (w->preference->reference_count) && (w->preference->o_supported));
}

inline double wma_pow(agent* thisAgent, wma_d_cycle cycle_diff)
{
    if (cycle_diff < thisAgent->wma_power_size)
    {
        return thisAgent->wma_power_array[ cycle_diff ];
    }
    else
    {
        return pow(static_cast<double>(cycle_diff), thisAgent->wma_params->decay_rate->get_value());
    }
}

inline double wma_sum_history(agent* thisAgent, wma_history* history, wma_d_cycle current_cycle)
{
    double return_val = 0.0;

    unsigned int p = history->next_p;
    unsigned int counter = history->history_ct;
    wma_d_cycle cycle_diff = 0;

    //

    while (counter)
    {
        p = wma_history_prev(p);

        cycle_diff = (current_cycle - history->access_history[ p ].d_cycle);
        assert(cycle_diff > 0);

        return_val += (history->access_history[ p ].num_references * wma_pow(thisAgent, cycle_diff));

        counter--;
    }

    // see (Petrov, 2006)
    if (thisAgent->wma_params->petrov_approx->get_value() == on)
    {
        // if ( n > k )
        if (history->total_references > history->history_references)
        {
            // ( n - k ) * ( tn^(1-d) - tk^(1-d) )
            // -----------------------------------
            // ( 1 - d ) * ( tn - tk )

            // decay_rate is negated (for nice printing)
            double d_inv = (1 + thisAgent->wma_params->decay_rate->get_value());

            return_val += (((history->total_references - history->history_references) * (pow(static_cast<double>(current_cycle - history->first_reference), d_inv) - pow(static_cast<double>(cycle_diff), d_inv))) /
                           (d_inv * ((current_cycle - history->first_reference) - cycle_diff)));
        }
    }

    return return_val;
}

inline double wma_calculate_decay_activation(agent* thisAgent, wma_decay_element* decay_el, wma_d_cycle current_cycle, bool log_result)
{
    wma_history* history = &(decay_el->touches);

    if (history->history_ct)
    {
        double history_sum = wma_sum_history(thisAgent, history, current_cycle);

        if (!log_result)
        {
            return history_sum;
        }

        if (history_sum > 0.0)
        {
            return log(history_sum);
        }
        else
        {
            return WMA_ACTIVATION_LOW;
        }
    }
    else
    {
        return ((log_result) ? (WMA_ACTIVATION_LOW) : (0.0));
    }
}

inline wma_reference wma_calculate_initial_boost(agent* thisAgent, wme* w)
{
    wma_reference return_val = 0;
    preference* pref;
    condition* cond;
    wme* cond_wme;
    wma_pooled_wme_set::iterator wme_p;

    tc_number tc = (thisAgent->wma_tc_counter++);

    uint64_t num_cond_wmes = 0;
    double combined_time_sum = 0.0;

    for (pref = w->preference->slot->preferences[ACCEPTABLE_PREFERENCE_TYPE]; pref; pref = pref->next)
    {
        if ((pref->value == w->value) && (pref->o_supported))
        {
            for (cond = pref->inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
            {
                if ((cond->type == POSITIVE_CONDITION) && (cond->bt.wme_->wma_tc_value != tc))
                {
                    cond_wme = cond->bt.wme_;
                    cond_wme->wma_tc_value = tc;

                    if (cond_wme->wma_decay_el)
                    {
                        if (!cond_wme->wma_decay_el->just_created)
                        {
                            num_cond_wmes++;
                            combined_time_sum += wma_get_wme_activation(thisAgent, cond_wme, false);
                        }
                    }
                    else if (cond_wme->preference)
                    {
                        if (cond_wme->preference->wma_o_set)
                        {
                            for (wme_p = cond_wme->preference->wma_o_set->begin(); wme_p != cond_wme->preference->wma_o_set->end(); wme_p++)
                            {
                                if (((*wme_p)->wma_tc_value != tc) && (!(*wme_p)->wma_decay_el || !(*wme_p)->wma_decay_el->just_created))
                                {
                                    num_cond_wmes++;
                                    combined_time_sum += wma_get_wme_activation(thisAgent, (*wme_p), false);

                                    (*wme_p)->wma_tc_value = tc;
                                }
                            }
                        }
                    }
                    else
                    {
                        num_cond_wmes++;
                        combined_time_sum += wma_get_wme_activation(thisAgent, cond_wme, false);
                    }
                }
            }
        }
    }

    if (num_cond_wmes)
    {
        return_val = static_cast<wma_reference>(floor(combined_time_sum / num_cond_wmes));
    }

    return return_val;
}

void wma_activate_wme(agent* thisAgent, wme* w, wma_reference num_references, wma_pooled_wme_set* o_set, bool o_only)
{
    // o-supported, non-architectural WME
    if (wma_should_have_decay_element(w))
    {
        wma_decay_element* temp_el = w->wma_decay_el;

        // if decay structure doesn't exist, create it
        if (!temp_el)
        {
            thisAgent->memoryManager->allocate_with_pool(MP_wma_decay_element, &temp_el);

            temp_el->this_wme = w;
            temp_el->just_removed = false;

            temp_el->just_created = true;
            temp_el->num_references = wma_calculate_initial_boost(thisAgent, w);

            temp_el->touches.history_ct = 0;
            temp_el->touches.next_p = 0;

            for (int i = 0; i < WMA_DECAY_HISTORY; i++)
            {
                temp_el->touches.access_history[ i ].d_cycle = 0;
                temp_el->touches.access_history[ i ].num_references = 0;
            }

            temp_el->touches.history_references = 0;
            temp_el->touches.total_references = 0;
            temp_el->touches.first_reference = 0;

            // prevents confusion with delayed forgetting
            temp_el->forget_cycle = static_cast< wma_d_cycle >(-1);

            w->wma_decay_el = temp_el;

            if (thisAgent->sysparams[ TRACE_WMA_SYSPARAM ])
            {
                std::string msg("WMA @");
                std::string temp;

                to_string(thisAgent->d_cycle_count, temp);
                msg.append(temp);
                msg.append(": ");

                msg.append("add ");

                to_string(w->timetag, temp);
                msg.append(temp);
                msg.append(" ");

                to_string(w->id->id->name_letter, temp);
                msg.append(temp);

                to_string(w->id->id->name_number, temp);
                msg.append(temp);
                msg.append(" ");

                switch (w->attr->symbol_type)
                {
                    case INT_CONSTANT_SYMBOL_TYPE:
                        to_string(w->attr->ic->value, temp);
                        break;

                    case FLOAT_CONSTANT_SYMBOL_TYPE:
                        to_string(w->attr->fc->value, temp);
                        break;

                    case STR_CONSTANT_SYMBOL_TYPE:
                        to_string(w->attr->sc->name, temp);
                        break;
                }

                msg.append(temp);
                msg.append(" ");

                switch (w->value->symbol_type)
                {
                    case INT_CONSTANT_SYMBOL_TYPE:
                        to_string(w->value->ic->value, temp);
                        break;

                    case FLOAT_CONSTANT_SYMBOL_TYPE:
                        to_string(w->value->fc->value, temp);
                        break;

                    case STR_CONSTANT_SYMBOL_TYPE:
                        to_string(w->value->sc->name, temp);
                        break;
                }

                msg.append(temp);
                msg.append("\n");

                print(thisAgent,  msg.c_str());
                xml_generate_warning(thisAgent, msg.c_str());
            }
        }

        // add to o_set if necessary
        if (o_set)
        {
            o_set->insert(w);
        }
        // otherwise update the decay element
        else
        {
            temp_el->num_references += num_references;
            thisAgent->wma_touched_elements->insert(w);
        }
    }
    // i-supported, non-architectural WME
    else if (!o_only && (w->preference) && (w->preference->reference_count))
    {
        wma_pooled_wme_set* my_o_set = w->preference->wma_o_set;
        wma_pooled_wme_set::iterator wme_p;

        // if doesn't have an o_set, populate
        if (!my_o_set)
        {
            thisAgent->memoryManager->allocate_with_pool(MP_wma_wme_oset, &my_o_set);
#ifdef USE_MEM_POOL_ALLOCATORS
            my_o_set = new(my_o_set) wma_pooled_wme_set(std::less< wme* >(), soar_module::soar_memory_pool_allocator< wme* >());
#else
            my_o_set = new(my_o_set) wma_pooled_wme_set();
#endif

            w->preference->wma_o_set = my_o_set;

            for (condition* c = w->preference->inst->top_of_instantiated_conditions; c; c = c->next)
            {
                if (c->type == POSITIVE_CONDITION)
                {
                    wma_activate_wme(thisAgent, c->bt.wme_, 0, my_o_set);
                }
            }

            for (wme_p = my_o_set->begin(); wme_p != my_o_set->end(); wme_p++)
            {
                // add a ref to wmes on this list
                wme_add_ref((*wme_p));
            }
        }

        // iterate over the o_set
        for (wme_p = my_o_set->begin(); wme_p != my_o_set->end(); wme_p++)
        {
            // if populating o_set, add
            if (o_set)
            {
                o_set->insert((*wme_p));
            }
            // otherwise, "activate" the wme if it is
            // non-architectural (avoids dereferencing
            // the wme preference)
            else
            {
                if ((*wme_p)->wma_decay_el)
                {
                    (*wme_p)->wma_decay_el->num_references += num_references;
                    thisAgent->wma_touched_elements->insert((*wme_p));
                }
            }
        }
    }
    // architectural
    else if (!o_only && !w->preference && (w->reference_count != 0))
    {
        // only action is to add it to the o_set
        if (o_set)
        {
            o_set->insert(w);
        }
    }
}

inline void wma_forgetting_remove_from_p_queue(agent* thisAgent, wma_decay_element* decay_el);
void wma_deactivate_element(agent* thisAgent, wme* w)
{
    wma_decay_element* temp_el = w->wma_decay_el;

    if (temp_el)
    {
        if (!temp_el->just_removed)
        {
            thisAgent->wma_touched_elements->erase(w);

            if ((thisAgent->wma_params->forgetting->get_value() == wma_param_container::approx) || (thisAgent->wma_params->forgetting->get_value() == wma_param_container::bsearch))
            {
                wma_forgetting_remove_from_p_queue(thisAgent, temp_el);
            }

            temp_el->just_removed = true;
        }
    }
}

void wma_remove_decay_element(agent* thisAgent, wme* w)
{
    wma_decay_element* temp_el = w->wma_decay_el;

    if (temp_el)
    {
        // Deactivate the wme first
        if (!temp_el->just_removed)
        {
            wma_deactivate_element(thisAgent, w);
        }

        // log
        if (thisAgent->sysparams[ TRACE_WMA_SYSPARAM ])
        {
            std::string msg("WMA @");
            std::string temp;

            to_string(thisAgent->d_cycle_count, temp);
            msg.append(temp);
            msg.append(": ");

            msg.append("remove ");

            to_string(w->timetag, temp);
            msg.append(temp);

            msg.append("\n");

            print(thisAgent,  msg.c_str());
            xml_generate_warning(thisAgent, msg.c_str());
        }

        thisAgent->memoryManager->free_with_pool(MP_wma_decay_element, temp_el);
        w->wma_decay_el = NULL;
    }
}

void wma_remove_pref_o_set(agent* thisAgent, preference* pref)
{
    if (pref && pref->wma_o_set)
    {
        wma_pooled_wme_set* victim = pref->wma_o_set;
        pref->wma_o_set = NULL;

        for (wma_pooled_wme_set::iterator p = victim->begin(); p != victim->end(); p++)
        {
            wme_remove_ref(thisAgent, (*p));
        }

        victim->~wma_pooled_wme_set();
        thisAgent->memoryManager->free_with_pool(MP_wma_wme_oset, victim);
    }
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Forgetting Functions (wma::forget)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline void wma_forgetting_add_to_p_queue(agent* thisAgent, wma_decay_element* decay_el, wma_d_cycle new_cycle)
{
    if (decay_el)
    {
        decay_el->forget_cycle = new_cycle;

        wma_forget_p_queue::iterator pq_p = thisAgent->wma_forget_pq->find(new_cycle);
        if (pq_p == thisAgent->wma_forget_pq->end())
        {
            wma_decay_set* newbie;
            thisAgent->memoryManager->allocate_with_pool(MP_wma_decay_set, &newbie);
#ifdef USE_MEM_POOL_ALLOCATORS
            newbie = new(newbie) wma_decay_set(std::less< wma_decay_element* >(), soar_module::soar_memory_pool_allocator< wma_decay_element* >(thisAgent));
#else
            newbie = new(newbie) wma_decay_set();
#endif
            newbie->insert(decay_el);

            thisAgent->wma_forget_pq->insert(std::make_pair(new_cycle, newbie));
        }
        else
        {
            pq_p->second->insert(decay_el);
        }
    }
}

inline void wma_forgetting_remove_from_p_queue(agent* thisAgent, wma_decay_element* decay_el)
{
    if (decay_el)
    {
        // try to find set for the element per cycle
        wma_forget_p_queue::iterator pq_p = thisAgent->wma_forget_pq->find(decay_el->forget_cycle);
        if (pq_p != thisAgent->wma_forget_pq->end())
        {
            wma_decay_set::iterator d_p = pq_p->second->find(decay_el);
            if (d_p != pq_p->second->end())
            {
                pq_p->second->erase(d_p);

                if (pq_p->second->empty())
                {
                    thisAgent->wma_touched_sets->insert(pq_p->first);
                }
            }
        }
    }
}

inline void wma_forgetting_move_in_p_queue(agent* thisAgent, wma_decay_element* decay_el, wma_d_cycle new_cycle)
{
    if (decay_el && (decay_el->forget_cycle != new_cycle))
    {
        wma_forgetting_remove_from_p_queue(thisAgent, decay_el);
        wma_forgetting_add_to_p_queue(thisAgent, decay_el, new_cycle);
    }
}

inline wma_d_cycle wma_forgetting_estimate_cycle(agent* thisAgent, wma_decay_element* decay_el, bool fresh_reference)
{
    wma_d_cycle return_val = static_cast<wma_d_cycle>(thisAgent->wma_d_cycle_count);
    wma_param_container::forgetting_choices forgetting = thisAgent->wma_params->forgetting->get_value();

    if (fresh_reference && (forgetting == wma_param_container::approx))
    {
        wma_d_cycle to_add = 0;

        wma_history* history = &(decay_el->touches);
        unsigned int p = history->next_p;
        unsigned int counter = history->history_ct;
        wma_d_cycle cycle_diff = 0;
        wma_reference approx_ref;

        //

        while (counter)
        {
            p = wma_history_prev(p);

            cycle_diff = (return_val - history->access_history[ p ].d_cycle);

            approx_ref = ((history->access_history[ p ].num_references < WMA_REFERENCES_PER_DECISION) ? (history->access_history[ p ].num_references) : (WMA_REFERENCES_PER_DECISION - 1));
            if (thisAgent->wma_approx_array[ approx_ref ] > cycle_diff)
            {
                to_add += (thisAgent->wma_approx_array[ approx_ref ] - cycle_diff);
            }

            counter--;
        }

        return_val += to_add;
    }

    if (return_val == static_cast<wma_d_cycle>(thisAgent->wma_d_cycle_count))
    {
        double my_thresh = thisAgent->wma_thresh_exp;

        // binary parameter search
        {
            wma_d_cycle to_add = 1;
            double act = wma_calculate_decay_activation(thisAgent, decay_el, (return_val + to_add), false);

            if (act >= my_thresh)
            {
                while (act >= my_thresh)
                {
                    to_add *= 2;
                    act = wma_calculate_decay_activation(thisAgent, decay_el, (return_val + to_add), false);
                }

                //

                wma_d_cycle upper_bound = to_add;
                wma_d_cycle lower_bound, mid;
                if (to_add < 4)
                {
                    lower_bound = upper_bound;
                }
                else
                {
                    lower_bound = (to_add / 2);
                }

                while (lower_bound != upper_bound)
                {
                    mid = ((lower_bound + upper_bound) / 2);
                    act = wma_calculate_decay_activation(thisAgent, decay_el, (return_val + mid), false);

                    if (act < my_thresh)
                    {
                        upper_bound = mid;

                        if (upper_bound - lower_bound <= 1)
                        {
                            lower_bound = mid;
                        }
                    }
                    else
                    {
                        lower_bound = mid;

                        if (upper_bound - lower_bound <= 1)
                        {
                            lower_bound = upper_bound;
                        }
                    }
                }

                to_add = upper_bound;
            }

            return_val += to_add;
        }
    }

    return return_val;
}

inline bool wma_forgetting_forget_wme(agent* thisAgent, wme* w)
{
    bool return_val = false;
    bool fake = (thisAgent->wma_params->fake_forgetting->get_value() == on);

    if (w->preference && w->preference->slot)
    {
        preference* p = w->preference->slot->all_preferences;
        preference* next_p;

        while (p)
        {
            next_p = p->all_of_slot_next;

            if (p->o_supported && p->in_tm && (p->value == w->value))
            {
                if (!fake)
                {
                    remove_preference_from_tm(thisAgent, p);
                    return_val = true;
                }
            }

            p = next_p;
        }
    }

    return return_val;
}

inline bool wma_forgetting_update_p_queue(agent* thisAgent)
{
    bool return_val = false;
    bool do_forget = false;
    slot* s;
    wme* w;

    if (!thisAgent->wma_forget_pq->empty())
    {
        wma_forget_p_queue::iterator pq_p = thisAgent->wma_forget_pq->begin();
        wma_d_cycle current_cycle = thisAgent->wma_d_cycle_count;
        double decay_thresh = thisAgent->wma_thresh_exp;
        bool forget_only_lti = (thisAgent->wma_params->forget_wme->get_value() == wma_param_container::lti);

        if (pq_p->first == current_cycle)
        {
            wma_decay_set::iterator d_p = pq_p->second->begin();
            wma_decay_set::iterator current_p;

            while (d_p != pq_p->second->end())
            {
                current_p = d_p++;

                if (wma_calculate_decay_activation(thisAgent, (*current_p), current_cycle, false) < decay_thresh)
                {
                    (*current_p)->forget_cycle = WMA_FORGOTTEN_CYCLE;

                    if (!forget_only_lti || ((*current_p)->this_wme->id->id->smem_lti != NIL))
                    {
                        do_forget = true;

                        // implements all-or-nothing check for lti mode
                        if (forget_only_lti)
                        {
                            for (s = (*current_p)->this_wme->id->id->slots; (s && do_forget); s = s->next)
                            {
                                for (w = s->wmes; (w && do_forget); w = w->next)
                                {
                                    if (w->preference->o_supported && (!w->wma_decay_el || (w->wma_decay_el->forget_cycle != WMA_FORGOTTEN_CYCLE)))
                                    {
                                        do_forget = false;
                                    }
                                }
                            }
                        }

                        if (do_forget)
                        {
                            if (forget_only_lti)
                            {
                                // implements all-or-nothing forget for lti mode
                                for (s = (*current_p)->this_wme->id->id->slots; (s && do_forget); s = s->next)
                                {
                                    for (w = s->wmes; (w && do_forget); w = w->next)
                                    {
                                        if (wma_forgetting_forget_wme(thisAgent, w))
                                        {
                                            return_val = true;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (wma_forgetting_forget_wme(thisAgent, (*current_p)->this_wme))
                                {
                                    return_val = true;
                                }
                            }
                        }
                    }
                }
                else
                {
                    wma_forgetting_move_in_p_queue(thisAgent, (*current_p), wma_forgetting_estimate_cycle(thisAgent, (*current_p), false));
                }
            }

            // clean up decay set
            thisAgent->wma_touched_sets->insert(pq_p->first);
            pq_p->second->clear();
        }

        // clean up touched sets
        for (wma_decay_cycle_set::iterator touched_it = thisAgent->wma_touched_sets->begin(); touched_it != thisAgent->wma_touched_sets->end(); touched_it++)
        {
            pq_p = thisAgent->wma_forget_pq->find(*touched_it);

            if ((pq_p != thisAgent->wma_forget_pq->end()) && (pq_p->second->empty()))
            {
                pq_p->second->~wma_decay_set();
                thisAgent->memoryManager->free_with_pool(MP_wma_decay_set, pq_p->second);

                thisAgent->wma_forget_pq->erase(pq_p);
            }
        }
        thisAgent->wma_touched_sets->clear();
    }

    return return_val;
}

inline bool wma_forgetting_naive_sweep(agent* thisAgent)
{
    wma_d_cycle current_cycle = thisAgent->wma_d_cycle_count;
    double decay_thresh = thisAgent->wma_thresh_exp;
    bool forget_only_lti = (thisAgent->wma_params->forget_wme->get_value() == wma_param_container::lti);
    bool return_val = false;

    for (wme* w = thisAgent->all_wmes_in_rete; w; w = w->rete_next)
    {
        if (w->wma_decay_el && (!forget_only_lti || (w->id->id->smem_lti != NIL)))
        {
            // to be forgotten, wme must...
            // - have been accessed (can't imagine why not, but just in case)
            // - not have been accessed this cycle (i.e. no decay)
            // - have activation less than threshold
            if ((w->wma_decay_el->touches.total_references > 0) &&
                    (w->wma_decay_el->touches.access_history[ wma_history_prev(w->wma_decay_el->touches.next_p) ].d_cycle < current_cycle) &&
                    (wma_calculate_decay_activation(thisAgent, w->wma_decay_el, current_cycle, false) < decay_thresh))
            {
                if (wma_forgetting_forget_wme(thisAgent, w))
                {
                    return_val = true;
                }
            }
        }
    }

    return return_val;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Activation Update Functions (wma::update)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void wma_activate_wmes_in_pref(agent* thisAgent, preference* pref)
{
    wme* w;

    if (pref->type == ACCEPTABLE_PREFERENCE_TYPE)
    {
        w = pref->slot->wmes;
        while (w)
        {
            // id and attr should already match so just compare the value
            if (w->value == pref->value)
            {
                wma_activate_wme(thisAgent, w);
            }

            w = w->next;
        }
    }
}

void wma_activate_wmes_tested_in_prods(agent* thisAgent)
{
    ms_change* msc;
    token temp_token, *t;

    for (msc = thisAgent->ms_o_assertions; msc != NIL; msc = msc->next)
    {
        temp_token.parent = msc->tok;
        temp_token.w = msc->w;
        t = &temp_token;

        while (t != thisAgent->dummy_top_token)
        {
            if (t->w != NIL)
            {
                wma_activate_wme(thisAgent, t->w);
            }

            t = t->parent;
        }
    }

    for (msc = thisAgent->ms_i_assertions; msc != NIL; msc = msc->next)
    {
        temp_token.parent = msc->tok;
        temp_token.w = msc->w;
        t = &temp_token;

        while (t != thisAgent->dummy_top_token)
        {
            if (t->w != NIL)
            {
                wma_activate_wme(thisAgent, t->w);
            }

            t = t->parent;
        }
    }
}

inline void wma_update_decay_histories(agent* thisAgent)
{
    wma_pooled_wme_set::iterator wme_p;
    wma_decay_element* temp_el;
    wma_d_cycle current_cycle = thisAgent->wma_d_cycle_count;
    bool forgetting = ((thisAgent->wma_params->forgetting->get_value() == wma_param_container::approx) || (thisAgent->wma_params->forgetting->get_value() == wma_param_container::bsearch));

    // add to history for changed elements
    for (wme_p = thisAgent->wma_touched_elements->begin(); wme_p != thisAgent->wma_touched_elements->end(); wme_p++)
    {
        temp_el = (*wme_p)->wma_decay_el;

        // update number of references in the current history
        // (has to come before history overwrite)
        temp_el->touches.history_references += (temp_el->num_references - temp_el->touches.access_history[ temp_el->touches.next_p ].num_references);

        // set history
        temp_el->touches.access_history[ temp_el->touches.next_p ].d_cycle = current_cycle;
        temp_el->touches.access_history[ temp_el->touches.next_p ].num_references = temp_el->num_references;

        // log
        if (thisAgent->sysparams[ TRACE_WMA_SYSPARAM ])
        {
            std::string msg("WMA @");
            std::string temp;

            to_string(thisAgent->d_cycle_count, temp);
            msg.append(temp);
            msg.append(": ");

            msg.append("activate ");

            to_string(temp_el->this_wme->timetag, temp);
            msg.append(temp);
            msg.append(" ");

            to_string(temp_el->num_references, temp);
            msg.append(temp);

            msg.append("\n");

            print(thisAgent,  msg.c_str());
            xml_generate_warning(thisAgent, msg.c_str());
        }

        // keep track of first reference
        if (temp_el->touches.total_references == 0)
        {
            temp_el->touches.first_reference = current_cycle;
        }

        // update counters
        if (temp_el->touches.history_ct < WMA_DECAY_HISTORY)
        {
            temp_el->touches.history_ct++;
        }
        temp_el->touches.next_p = wma_history_next(temp_el->touches.next_p);
        temp_el->touches.total_references += temp_el->num_references;

        // reset cycle counter
        temp_el->num_references = 0;

        // update forgetting stuff as needed
        if (forgetting)
        {
            if (temp_el->just_created)
            {
                wma_forgetting_add_to_p_queue(thisAgent, temp_el, wma_forgetting_estimate_cycle(thisAgent, temp_el, true));
            }
            else
            {
                wma_forgetting_move_in_p_queue(thisAgent, temp_el, wma_forgetting_estimate_cycle(thisAgent, temp_el, true));
            }
        }

        temp_el->just_created = false;
    }
    thisAgent->wma_touched_elements->clear();
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// API Functions (wma::api)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

double wma_get_wme_activation(agent* thisAgent, wme* w, bool log_result)
{
    double return_val = static_cast<double>((log_result) ? (WMA_ACTIVATION_NONE) : (WMA_TIME_SUM_NONE));

    if (w->wma_decay_el)
    {
        return_val = wma_calculate_decay_activation(thisAgent, w->wma_decay_el, thisAgent->wma_d_cycle_count, log_result);
    }

    return return_val;
}

inline void _wma_ref_to_str(wma_cycle_reference& ref, wma_d_cycle current_cycle, std::string& str)
{
    std::string temp;
    wma_d_cycle cycle_diff = (current_cycle - ref.d_cycle);

    to_string(ref.num_references, temp);
    str.append(temp);

    str.append(" @ d");

    to_string(ref.d_cycle, temp);
    str.append(temp);

    str.append(" (-");

    to_string(cycle_diff, temp);
    str.append(temp);

    str.append(")");
}

void wma_get_wme_history(agent* thisAgent, wme* w, std::string& buffer)
{
    if (w->wma_decay_el)
    {
        wma_history* history = &(w->wma_decay_el->touches);
        unsigned int p = history->next_p;
        unsigned int counter = history->history_ct;
        wma_d_cycle current_cycle = thisAgent->wma_d_cycle_count;

        //

        buffer.append("history (");

        {
            std::string temp;

            to_string(history->history_references, temp);
            buffer.append(temp);

            buffer.append("/");

            to_string(history->total_references, temp);
            buffer.append(temp);

            buffer.append(", first @ d");

            to_string(history->first_reference, temp);
            buffer.append(temp);
        }

        buffer.append("):");

        //

        while (counter)
        {
            p = wma_history_prev(p);
            counter--;

            buffer.append("\n ");
            _wma_ref_to_str(history->access_history[ p ], current_cycle, buffer);
        }

        //

        wma_param_container::forgetting_choices forget = thisAgent->wma_params->forgetting->get_value();

        if ((forget == wma_param_container::bsearch) || (forget == wma_param_container::approx))
        {
            buffer.append("\n\n");
            buffer.append("considering WME for decay @ d");

            std::string temp;
            to_string(w->wma_decay_el->forget_cycle, temp);
            buffer.append(temp);
        }
    }
    else
    {
        buffer.assign("WME has no decay history");
    }
}

void wma_go(agent* thisAgent, wma_go_action go_action)
{
    // update history for all touched elements
    if (go_action == wma_histories)
    {
        thisAgent->wma_timers->history->start();

        wma_update_decay_histories(thisAgent);

        thisAgent->wma_timers->history->stop();
    }
    // check forgetting queue
    else if (go_action == wma_forgetting)
    {
        wma_param_container::forgetting_choices forgetting = thisAgent->wma_params->forgetting->get_value();

        if (forgetting != wma_param_container::disabled)
        {
            thisAgent->wma_timers->forgetting->start();

            bool forgot_something = false;

            if (forgetting == wma_param_container::naive)
            {
                forgot_something = wma_forgetting_naive_sweep(thisAgent);
            }
            else
            {
                forgot_something = wma_forgetting_update_p_queue(thisAgent);
            }

            if (forgot_something)
            {
                if (thisAgent->sysparams[ TRACE_WM_CHANGES_SYSPARAM ])
                {
                    const char* msg = "\n\nWMA: BEGIN FORGOTTEN WME LIST\n\n";

                    print(thisAgent,  const_cast<char*>(msg));
                    xml_generate_message(thisAgent, const_cast<char*>(msg));
                }

                uint64_t wm_removal_diff = thisAgent->wme_removal_count;
                {
                    do_working_memory_phase(thisAgent);
                }
                wm_removal_diff = (thisAgent->wme_removal_count - wm_removal_diff);

                if (wm_removal_diff > 0)
                {
                    thisAgent->wma_stats->forgotten_wmes->set_value(thisAgent->wma_stats->forgotten_wmes->get_value() + static_cast< int64_t >(wm_removal_diff));
                }

                if (thisAgent->sysparams[ TRACE_WM_CHANGES_SYSPARAM ])
                {
                    const char* msg = "\nWMA: END FORGOTTEN WME LIST\n\n";

                    print(thisAgent,  const_cast<char*>(msg));
                    xml_generate_message(thisAgent, const_cast<char*>(msg));
                }
            }

            thisAgent->wma_timers->forgetting->stop();
        }
    }
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
