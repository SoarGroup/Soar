#include "reinforcement_learning.h"

#include "agent.h"
#include "condition.h"
#include "decide.h"
#include "decision_manipulation.h"
#include "ebc.h"
#include "exploration.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "slot.h"
#include "test.h"
#include "working_memory.h"
#include "xml.h"

#include <cstdlib>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>

RL_Manager::RL_Manager(agent* myAgent)
{
    thisAgent = myAgent;
    thisAgent->RL = this;

    rl_init_count = 0;

    exploration_params[ EXPLORATION_PARAM_EPSILON ] = exploration_add_parameter(0.1, &exploration_validate_epsilon, "epsilon");
    exploration_params[ EXPLORATION_PARAM_TEMPERATURE ] = exploration_add_parameter(25, &exploration_validate_temperature, "temperature");

    rl_params = new rl_param_container(thisAgent);
    rl_stats = new rl_stat_container(thisAgent);
    rl_prods = new rl_production_memory();

    rl_initialize_template_tracking(thisAgent);

    // select initialization
    thisAgent->select = new select_info;
    select_init(thisAgent);

    // predict initialization
    thisAgent->prediction = new std::string();
    predict_init(thisAgent);

};

void RL_Manager::clean_up_for_agent_deletion()
{
    /* This is not in destructor because it may be called before other
     * deletion code that may need params, stats or timers to exist */
    // cleanup exploration

    for (int i = 0; i < EXPLORATION_PARAMS; i++)
    {
        delete exploration_params[ i ];
    }

    rl_params->apoptosis->set_value(rl_param_container::apoptosis_none);
    delete rl_prods;
    delete rl_params;
    delete rl_stats;
    rl_params = NULL; // apoptosis needs to know this for excise_all_productions later in agent deletion process

    select_init(thisAgent);
    delete thisAgent->select;
    delete thisAgent->prediction;

}

/////////////////////////////////////////////////////
// Parameters
/////////////////////////////////////////////////////

const std::vector<std::pair<std::string, param_accessor<double> *> >& rl_param_container::get_documentation_params()
{
    static std::vector<std::pair<std::string, param_accessor<double> *> > documentation_params;
    static bool initted = false;
    if (!initted)
    {
        initted = true;
        // Is it okay to use new here, because this is a static variable anyway,
        // so it's not going to happen more than once and shouldn't ever be cleaned up?
        documentation_params.push_back(std::make_pair("rl-updates", new rl_updates_accessor()));
        documentation_params.push_back(std::make_pair("delta-bar-delta-h", new rl_dbd_h_accessor()));
    }
    return documentation_params;
}

rl_param_container::rl_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    // learning
    learning = new rl_learning_param("learning", off, new soar_module::f_predicate<boolean>(), new_agent);
    add(learning);

    // discount-rate
    discount_rate = new soar_module::decimal_param("discount-rate", 0.9, new soar_module::btw_predicate<double>(0, 1, true), new soar_module::f_predicate<double>());
    add(discount_rate);

    // learning-rate
    learning_rate = new soar_module::decimal_param("learning-rate", 0.3, new soar_module::btw_predicate<double>(0, 1, true), new soar_module::f_predicate<double>());
    add(learning_rate);

    // step-size-parameter
    step_size_parameter = new soar_module::decimal_param("step-size-parameter", 1.0, new soar_module::btw_predicate<double>(0, 1, true), new soar_module::f_predicate<double>());
    add(step_size_parameter);

    // meta-learning-rate
    meta_learning_rate = new soar_module::decimal_param("meta-learning-rate", 0.1, new soar_module::btw_predicate<double>(0, 1, true), new soar_module::f_predicate<double>());
    add(meta_learning_rate);

    // learning-policy
    learning_policy = new soar_module::constant_param<learning_choices>("learning-policy", sarsa, new soar_module::f_predicate<learning_choices>());
    learning_policy->add_mapping(sarsa, "sarsa");
    learning_policy->add_mapping(q, "q-learning");
    learning_policy->add_mapping(on_policy_gql, "on-policy-gq-lambda");
    learning_policy->add_mapping(off_policy_gql, "off-policy-gq-lambda");
    add(learning_policy);

    // decay-mode
    decay_mode = new soar_module::constant_param<decay_choices>("decay-mode", normal_decay, new soar_module::f_predicate<decay_choices>());
    decay_mode->add_mapping(normal_decay, "normal");
    decay_mode->add_mapping(exponential_decay, "exp");
    decay_mode->add_mapping(logarithmic_decay, "log");
    decay_mode->add_mapping(delta_bar_delta_decay, "delta-bar-delta");
    add(decay_mode);

    // eligibility-trace-decay-rate
    et_decay_rate = new soar_module::decimal_param("eligibility-trace-decay-rate", 0, new soar_module::btw_predicate<double>(0, 1, true), new soar_module::f_predicate<double>());
    add(et_decay_rate);

    // eligibility-trace-tolerance
    et_tolerance = new soar_module::decimal_param("eligibility-trace-tolerance", 0.001, new soar_module::gt_predicate<double>(0, false), new soar_module::f_predicate<double>());
    add(et_tolerance);

    // temporal-extension
    temporal_extension = new soar_module::boolean_param("temporal-extension", on, new soar_module::f_predicate<boolean>());
    add(temporal_extension);

    // hrl-discount
    hrl_discount = new soar_module::boolean_param("hrl-discount", off, new soar_module::f_predicate<boolean>());
    add(hrl_discount);

    // temporal-discount
    temporal_discount = new soar_module::boolean_param("temporal-discount", on, new soar_module::f_predicate<boolean>());
    add(temporal_discount);

    // chunk-stop
    chunk_stop = new soar_module::boolean_param("chunk-stop", on, new soar_module::f_predicate<boolean>());
    add(chunk_stop);

    // meta
    meta = new soar_module::boolean_param("meta", off, new soar_module::f_predicate<boolean>());
    add(meta);

    // update-log-path
    update_log_path = new soar_module::string_param("update-log-path", "", new soar_module::predicate<const char*>(), new soar_module::f_predicate<const char*>());
    add(update_log_path);

    // apoptosis
    apoptosis = new rl_apoptosis_param("apoptosis", apoptosis_none, new soar_module::f_predicate<apoptosis_choices>(), thisAgent);
    apoptosis->add_mapping(apoptosis_none, "none");
    apoptosis->add_mapping(apoptosis_chunks, "chunks");
    apoptosis->add_mapping(apoptosis_rl, "rl-chunks");
    add(apoptosis);

    // apoptosis-decay
    apoptosis_decay = new soar_module::decimal_param("apoptosis-decay", 0.5, new soar_module::btw_predicate<double>(0, 1, true), new rl_apoptosis_predicate<double>(thisAgent));
    add(apoptosis_decay);

    // apoptosis-thresh
    apoptosis_thresh = new rl_apoptosis_thresh_param("apoptosis-thresh", -2.0, new soar_module::gt_predicate<double>(0, false), new rl_apoptosis_predicate<double>(thisAgent));
    add(apoptosis_thresh);

    // trace
    trace = new soar_module::boolean_param("trace", off, new soar_module::f_predicate<boolean>());
    add(trace);
};

//

void rl_reset_data(agent*);

rl_learning_param::rl_learning_param(const char* new_name, boolean new_value, soar_module::predicate<boolean>* new_prot_pred, agent* new_agent): soar_module::boolean_param(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

void rl_learning_param::set_value(boolean new_value)
{
    if (new_value != value)
    {
        if (new_value == off)
        {
            rl_reset_data(thisAgent);
        }

        value = new_value;
    }
}

//

rl_apoptosis_param::rl_apoptosis_param(const char* new_name, rl_param_container::apoptosis_choices new_value, soar_module::predicate<rl_param_container::apoptosis_choices>* new_prot_pred, agent* new_agent): soar_module::constant_param<rl_param_container::apoptosis_choices>(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

void rl_apoptosis_param::set_value(rl_param_container::apoptosis_choices new_value)
{
    if (value != new_value)
    {
        // from off to on (doesn't matter which)
        if (value == rl_param_container::apoptosis_none)
        {
            thisAgent->RL->rl_prods->set_decay_rate(thisAgent->RL->rl_params->apoptosis_decay->get_value());
            thisAgent->RL->rl_prods->set_decay_thresh(thisAgent->RL->rl_params->apoptosis_thresh->get_value());
            thisAgent->RL->rl_prods->initialize();
        }
        // from on to off
        else if (new_value == rl_param_container::apoptosis_none)
        {
            thisAgent->RL->rl_prods->teardown();
        }

        value = new_value;
    }
}

//

rl_apoptosis_thresh_param::rl_apoptosis_thresh_param(const char* new_name, double new_value, soar_module::predicate<double>* new_val_pred, soar_module::predicate<double>* new_prot_pred): soar_module::decimal_param(new_name, new_value, new_val_pred, new_prot_pred) {}

void rl_apoptosis_thresh_param::set_value(double new_value)
{
    value = -new_value;
}

//

template <typename T>
rl_apoptosis_predicate<T>::rl_apoptosis_predicate(agent* new_agent): soar_module::agent_predicate<T>(new_agent) {}

template <typename T>
bool rl_apoptosis_predicate<T>::operator()(T /*val*/)
{
    return (this->thisAgent->RL->rl_params->apoptosis->get_value() != rl_param_container::apoptosis_none);
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// Stats
/////////////////////////////////////////////////////

rl_stat_container::rl_stat_container(agent* new_agent): stat_container(new_agent)
{
    // update-error
    update_error = new soar_module::decimal_stat("update-error", 0, new soar_module::f_predicate<double>());
    add(update_error);

    // total-reward
    total_reward = new soar_module::decimal_stat("total-reward", 0, new soar_module::f_predicate<double>());
    add(total_reward);

    // global-reward
    global_reward = new soar_module::decimal_stat("global-reward", 0, new soar_module::f_predicate<double>());
    add(global_reward);
};


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// quick shortcut to determine if rl is enabled
bool rl_enabled(agent* thisAgent)
{
    return (thisAgent->RL->rl_params->learning->get_value() == on);
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

inline void rl_add_ref(Symbol* goal, production* prod)
{
    goal->id->rl_info->prev_op_rl_rules->push_back(prod);
    prod->rl_ref_count++;
}

inline void rl_remove_ref(Symbol* goal, production* prod)
{
    production_list* rules = goal->id->rl_info->prev_op_rl_rules;

    for (production_list::iterator p = rules->begin(); p != rules->end(); p++)
    {
        if (*p == prod)
        {
            prod->rl_ref_count--;
        }
    }

    rules->remove(prod);
    /* For use when we try using a vector with a new memory allocator that can handle variable size allocations */
    //production_list::iterator newEnd = std::remove(rules->begin(), rules->end(), prod);
    //rules->erase(newEnd, rules->end());
}

void rl_clear_refs(Symbol* goal)
{
    production_list* rules = goal->id->rl_info->prev_op_rl_rules;

    for (production_list::iterator p = rules->begin(); p != rules->end(); p++)
    {
        (*p)->rl_ref_count--;
    }

    rules->clear();
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// resets rl data structures
void rl_reset_data(agent* thisAgent)
{
    Symbol* goal = thisAgent->top_goal;
    while (goal)
    {
        rl_data* data = goal->id->rl_info;

        data->eligibility_traces->clear();
        rl_clear_refs(goal);

        data->previous_q = 0;
        data->reward = 0;
        data->rho = 1.0;

        data->gap_age = 0;
        data->hrl_age = 0;

        goal = goal->id->lower_goal;
    }
}

// removes rl references to a production (used for excise)
void rl_remove_refs_for_prod(agent* thisAgent, production* prod)
{
    for (Symbol* state = thisAgent->top_state; state; state = state->id->lower_goal)
    {
        state->id->rl_info->eligibility_traces->erase(prod);
        rl_remove_ref(state, prod);
    }
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// returns true if a template is valid
bool rl_valid_template(production* prod)
{
    bool numeric_pref = false;
    bool var_pref = false;
    int num_actions = 0;

    for (action* a = prod->action_list; a; a = a->next)
    {
        num_actions++;
        if (a->type == MAKE_ACTION)
        {
            if (a->preference_type == NUMERIC_INDIFFERENT_PREFERENCE_TYPE)
            {
                numeric_pref = true;
            }
            else if (a->preference_type == BINARY_INDIFFERENT_PREFERENCE_TYPE)
            {
                if (rhs_value_is_symbol(a->referent) && (rhs_value_to_symbol(a->referent)->is_variable()))
                {
                    var_pref = true;
                }
            }
        }
    }

    return ((num_actions == 1) && (numeric_pref || var_pref));
}

// returns true if an rl rule is valid
bool rl_valid_rule(production* prod)
{
    bool numeric_pref = false;
    int num_actions = 0;

    for (action* a = prod->action_list; a; a = a->next)
    {
        num_actions++;
        if (a->type == MAKE_ACTION)
        {
            if (a->preference_type == NUMERIC_INDIFFERENT_PREFERENCE_TYPE)
            {
                numeric_pref = true;
            }
        }
    }

    return (numeric_pref && (num_actions == 1));
}

// sets rl meta-data from a production documentation string
void rl_rule_meta(agent* thisAgent, production* prod)
{
    if (prod->documentation && (thisAgent->RL->rl_params->meta->get_value() == on))
    {
        std::string doc(prod->documentation);

        const std::vector<std::pair<std::string, param_accessor<double> *> >& documentation_params = thisAgent->RL->rl_params->get_documentation_params();
        for (std::vector<std::pair<std::string, param_accessor<double> *> >::const_iterator doc_params_it = documentation_params.begin();
                doc_params_it != documentation_params.end(); ++doc_params_it)
        {
            const std::string& param_name = doc_params_it->first;
            param_accessor<double>* accessor = doc_params_it->second;
            std::stringstream param_name_ss;
            param_name_ss << param_name << "=";
            std::string search_term = param_name_ss.str();
            size_t begin_index = doc.find(search_term);
            if (begin_index == std::string::npos)
            {
                continue;
            }
            begin_index += search_term.size();
            size_t end_index = doc.find(";", begin_index);
            if (end_index == std::string::npos)
            {
                continue;
            }
            std::string param_value_str = doc.substr(begin_index, end_index);
            accessor->set_param(prod, param_value_str);
        }

        /*
        std::string search( "rlupdates=" );

        if ( doc.length() > search.length() )
        {
            if ( doc.substr( 0, search.length() ).compare( search ) == 0 )
            {
                uint64_t val;
                from_string( val, doc.substr( search.length() ) );

                prod->rl_update_count = static_cast< double >( val );
            }
        }
        */
    }
}

/***************************************************************************
 * Function     : is_natural_number
 **************************************************************************/

bool is_whole_number(const char* str)
{
    if (!str || !*str)
    {
        return false;
    }

    do
    {
        if (isdigit(*str))
        {
            ++str;
        }
        else
        {
            return false;
        }
    }
    while (*str);

    return true;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// gets the auto-assigned id of a template instantiation
int rl_get_template_id(const char* prod_name)
{
    std::string temp = prod_name;

    // has to be at least "rl*a*#" (where a is a single letter/number/etc)
    if (temp.length() < 6)
    {
        return -1;
    }

    // check first three letters are "rl*"
    if (temp.compare(0, 3, "rl*"))
    {
        return -1;
    }

    // find last * to isolate id
    std::string::size_type last_star = temp.find_last_of('*');
    if (last_star == std::string::npos)
    {
        return -1;
    }

    // make sure there's something left after last_star
    if (last_star == (temp.length() - 1))
    {
        return -1;
    }

    // make sure id is a valid natural number

    std::string id_str = temp.substr(last_star + 1);
    if (!is_whole_number(id_str.c_str()))
    {
        return -1;
    }

    // convert id
    int id;
    from_string(id, id_str);
    return id;
}

// initializes the max rl template counter
void rl_initialize_template_tracking(agent* thisAgent)
{
    thisAgent->RL->rl_template_count = 1;
}

// updates rl template counter for a rule
void rl_update_template_tracking(agent* thisAgent, const char* rule_name)
{
    int new_id = rl_get_template_id(rule_name);

    if ((new_id != -1) && (new_id > thisAgent->RL->rl_template_count))
    {
        thisAgent->RL->rl_template_count = (new_id + 1);
    }
}

// gets the next template-assigned id
int rl_next_template_id(agent* thisAgent)
{
    return (thisAgent->RL->rl_template_count++);
}

// gives back a template-assigned id (on auto-retract)
void rl_revert_template_id(agent* thisAgent)
{
    thisAgent->RL->rl_template_count--;
}

// builds a template instantiation
Symbol* rl_build_template_instantiation(agent* thisAgent, instantiation* my_template_instance, struct token_struct* tok, wme* w, action* rhs_actions)
{
    Symbol* new_name_symbol = NULL;

    if (my_template_instance->prod->rl_template_conds == NIL)
    {
        condition* c_top;
        condition* c_bottom;

        p_node_to_conditions_and_rhs(thisAgent, my_template_instance->prod->p_node, NIL, NIL, &(c_top), &(c_bottom), NIL, WM_Trace_w_Inequalities);
        my_template_instance->prod->rl_template_conds = c_top;
    }

        production* my_template = my_template_instance->prod;
        char first_letter;
        double init_value = 0;
        condition* cond_top, *cond_bottom;

        // make unique production name
        std::string new_name = "";
        std::string empty_string = "";
        std::string temp_id;
        int new_id;
        do
        {
            new_id = rl_next_template_id(thisAgent);
            to_string(new_id, temp_id);
            new_name = ("rl*" + empty_string + my_template->name->sc->name + "*" + temp_id);
        }
        while (thisAgent->symbolManager->find_str_constant(new_name.c_str()) != NIL);
        new_name_symbol = thisAgent->symbolManager->make_str_constant(new_name.c_str());

        copy_condition_list(thisAgent, my_template_instance->top_of_instantiated_conditions, &cond_top, &cond_bottom, false, false, true);

        thisAgent->symbolManager->reset_variable_generator(cond_top, NIL);
        thisAgent->explanationBasedChunker->set_rule_type(ebc_template);
        rl_add_goal_or_impasse_tests_to_conds(thisAgent, cond_top);
        thisAgent->explanationBasedChunker->variablize_rl_condition_list(cond_top);
        action* new_action = thisAgent->explanationBasedChunker->variablize_rl_action(rhs_actions, tok, w, init_value);

        // make new production
        thisAgent->name_of_production_being_reordered = new_name_symbol->sc->name;
        if (new_action && reorder_and_validate_lhs_and_rhs(thisAgent, &cond_top, &new_action, false))
        {
            production* new_production = make_production(thisAgent, USER_PRODUCTION_TYPE, new_name_symbol, my_template->name->sc->name, &cond_top, &new_action, false, NULL);

            // set initial expected reward values
            new_production->rl_ecr = 0.0;
            new_production->rl_efr = init_value;
            new_production->rl_gql = 0.0;

            // attempt to add to rete, remove if duplicate
            production* duplicate_rule = NULL;
            if (add_production_to_rete(thisAgent, new_production, cond_top, NULL, false, duplicate_rule, true) == DUPLICATE_PRODUCTION)
            {
                excise_production(thisAgent, new_production, false, false);
                rl_revert_template_id(thisAgent);
                new_name_symbol = NULL;
            }
        }
        else
        {
            thisAgent->name_of_production_being_reordered = NULL;
            rl_revert_template_id(thisAgent);
            thisAgent->symbolManager->symbol_remove_ref(&new_name_symbol);
            new_name_symbol = NULL;
        }

        thisAgent->explanationBasedChunker->clear_symbol_identity_map();
        thisAgent->explanationBasedChunker->set_rule_type(ebc_no_rule);
        deallocate_condition_list(thisAgent, cond_top);

    return new_name_symbol;
}

void rl_add_goal_or_impasse_tests_to_conds(agent* thisAgent, condition* all_conds)
{
    // mark each id as we add a test for it, so we don't add a test for the same id in two different places
    Symbol* id_sym;
    test t;
    tc_number tc = get_new_tc_number(thisAgent);

    for (condition* cond = all_conds; cond != NIL; cond = cond->next)
    {
        if (cond->type != POSITIVE_CONDITION)
        {
            continue;
        }

        id_sym = cond->data.tests.id_test->eq_test->data.referent;
        if ((id_sym->id->isa_goal || id_sym->id->isa_impasse) && (id_sym->tc_num != tc))
        {
            t = make_test(thisAgent, NIL, ((id_sym->id->isa_goal) ? GOAL_ID_TEST : IMPASSE_ID_TEST));
            add_test(thisAgent, &(cond->data.tests.id_test), t);
            id_sym->tc_num = tc;
        }
    }
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// gathers discounted reward for a state
void rl_tabulate_reward_value_for_goal(agent* thisAgent, Symbol* goal)
{
    rl_data* data = goal->id->rl_info;

    if (!data->prev_op_rl_rules->empty())
    {
        slot* s = find_slot(goal->id->rl_info->rl_link_wme->value, thisAgent->symbolManager->soarSymbols.rl_sym_reward);
        slot* t;
        wme* w, *x;

        double reward = 0.0;
        double discount_rate = thisAgent->RL->rl_params->discount_rate->get_value();

        if (s)
        {
            for (w = s->wmes; w; w = w->next)
            {
                if (w->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
                {
                    t = find_slot(w->value, thisAgent->symbolManager->soarSymbols.rl_sym_value);
                    if (t)
                    {
                        for (x = t->wmes; x; x = x->next)
                        {
                            if ((x->value->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) || (x->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE))
                            {
                                reward += get_number_from_symbol(x->value);
                            }
                        }
                    }
                }
            }

            // if temporal_discount is off, don't discount for gaps
            unsigned int effective_age = data->hrl_age;
            if (thisAgent->RL->rl_params->temporal_discount->get_value() == on)
            {
                effective_age += data->gap_age;
            }

            data->reward += (reward * pow(discount_rate, static_cast< double >(effective_age)));
        }

        // update stats
        double global_reward = thisAgent->RL->rl_stats->global_reward->get_value();
        thisAgent->RL->rl_stats->total_reward->set_value(reward);
        thisAgent->RL->rl_stats->global_reward->set_value(global_reward + reward);

        if ((goal != thisAgent->bottom_goal) && (thisAgent->RL->rl_params->hrl_discount->get_value() == on))
        {
            data->hrl_age++;
        }
    }
}

// gathers reward for all states
void rl_tabulate_reward_values(agent* thisAgent)
{
    Symbol* goal = thisAgent->top_goal;

    while (goal)
    {
        rl_tabulate_reward_value_for_goal(thisAgent, goal);
        goal = goal->id->lower_goal;
    }
}

// stores rl info for a state w.r.t. a selected operator
void rl_store_data(agent* thisAgent, Symbol* goal, preference* cand)
{
    rl_data* data = goal->id->rl_info;
    Symbol* op = cand->value;

    bool using_gaps = (thisAgent->RL->rl_params->temporal_extension->get_value() == on);

    // Make list of just-fired prods
    unsigned int just_fired = 0;
    for (preference* pref = goal->id->operator_slot->preferences[ NUMERIC_INDIFFERENT_PREFERENCE_TYPE ]; pref; pref = pref->next)
    {
        if ((op == pref->value) && pref->inst->prod->rl_rule)
        {
            if ((just_fired == 0) && !data->prev_op_rl_rules->empty())
            {
                rl_clear_refs(goal);
            }

            rl_add_ref(goal, pref->inst->prod);
            just_fired++;
        }
    }

    if (just_fired)
    {
        data->previous_q = cand->numeric_value;
        data->rho = cand->rl_rho;
    }
    else
    {
        if (thisAgent->trace_settings[ TRACE_RL_SYSPARAM ] && using_gaps &&
                (data->gap_age == 0) && !data->prev_op_rl_rules->empty())
        {
            char buf[256];
            SNPRINTF(buf, 254, "gap started (%c%llu)", goal->id->name_letter, static_cast<long long unsigned>(goal->id->name_number));

            thisAgent->outputManager->printa(thisAgent,  buf);
            xml_generate_warning(thisAgent, buf);
        }

        if (!using_gaps)
        {
            if (!data->prev_op_rl_rules->empty())
            {
                rl_clear_refs(goal);
            }

            data->previous_q = cand->numeric_value;
            data->rho = 1.0;
        }
        else
        {
            if (!data->prev_op_rl_rules->empty())
            {
                data->gap_age++;
            }
        }
    }
}

// performs the rl update at a state
void rl_perform_update(agent* thisAgent, double op_value, bool op_rl, Symbol* goal, bool update_efr)
{
    bool using_gaps = (thisAgent->RL->rl_params->temporal_extension->get_value() == on);

    if (!using_gaps || op_rl)
    {
        rl_data* data = goal->id->rl_info;

        if (!data->prev_op_rl_rules->empty())
        {
            rl_et_map::iterator iter;
            double alpha = thisAgent->RL->rl_params->learning_rate->get_value();
            double eta = thisAgent->RL->rl_params->step_size_parameter->get_value();
            double lambda = thisAgent->RL->rl_params->et_decay_rate->get_value();
            double gamma = thisAgent->RL->rl_params->discount_rate->get_value();
            double tolerance = thisAgent->RL->rl_params->et_tolerance->get_value();
            double theta = thisAgent->RL->rl_params->meta_learning_rate->get_value();

            // if temporal_discount is off, don't discount for gaps
            unsigned int effective_age = data->hrl_age + 1;
            if (thisAgent->RL->rl_params->temporal_discount->get_value() == on)
            {
                effective_age += data->gap_age;
            }

            double discount = pow(gamma, static_cast< double >(effective_age));

            // notify of gap closure
            if (data->gap_age && using_gaps && thisAgent->trace_settings[ TRACE_RL_SYSPARAM ])
            {
                char buf[256];
                SNPRINTF(buf, 254, "gap ended (%c%llu)", goal->id->name_letter, static_cast<long long unsigned>(goal->id->name_number));

                thisAgent->outputManager->printa(thisAgent,  buf);
                xml_generate_warning(thisAgent, buf);
            }

            // Iterate through eligibility_traces, decay traces. If less than TOLERANCE, remove from map.
            if (lambda == 0)
            {
                if (!data->eligibility_traces->empty())
                {
                    data->eligibility_traces->clear();
                }
            }
            else
            {
                for (iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end();)
                {
                    iter->second *= lambda;
                    iter->second *= discount;
                    if (iter->second < tolerance)
                    {
                        data->eligibility_traces->erase(iter++);
                    }
                    else
                    {
                        ++iter;
                    }
                }
            }

            if (thisAgent->RL->rl_params->learning_policy->get_value() & rl_param_container::gql)
            {
                for (iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); iter++)
                {
                    iter->second *= data->rho;
                }
            }

            // Update trace for just fired prods
            double sum_old_ecr = 0.0;
            double sum_old_efr = 0.0;
            double dot_w_phi = 0.0;
            if (!data->prev_op_rl_rules->empty())
            {
                /// I = 0.0 for non-terminal states when using hierarchical reinforcement learning
                const double I = goal->id->lower_goal ? 0.0 : 1.0;
                double trace_increment = I * (1.0 / static_cast<double>(data->prev_op_rl_rules->size()));
                production_list::iterator p;

                for (p = data->prev_op_rl_rules->begin(); p != data->prev_op_rl_rules->end(); p++)
                {
                    sum_old_ecr += (*p)->rl_ecr;
                    sum_old_efr += (*p)->rl_efr;

                    iter = data->eligibility_traces->find((*p));

                    if (iter != data->eligibility_traces->end())
                    {
                        iter->second += trace_increment;
                    }
                    else
                    {
                        (*data->eligibility_traces)[(*p) ] = trace_increment;
                    }

                    dot_w_phi += (*p)->rl_gql;
                }
            }

            // For each prod with a trace, perform update
            {
                double old_ecr = 0.0, old_efr = 0.0, old_gql = 0.0;
                double delta_ecr = 0.0, delta_efr = 0.0;
                double new_combined = 0.0, new_ecr = 0.0, new_efr = 0.0, new_gql = 0.0;
                double delta_t = (data->reward + discount * op_value) - (sum_old_ecr + sum_old_efr);

                double dot_w_e = 0.0;
                for (iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); iter++)
                {
                    dot_w_e += iter->first->rl_gql * iter->second;
                }

                for (iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); iter++)
                {
                    production* prod = iter->first;

                    // get old vals
                    old_ecr = prod->rl_ecr;
                    old_efr = prod->rl_efr;
                    old_gql = prod->rl_gql;

                    // Adjust alpha based on decay policy
                    // Miller 11/14/2011
                    double adjusted_alpha;
                    switch (thisAgent->RL->rl_params->decay_mode->get_value())
                    {
                        case rl_param_container::exponential_decay:
                            adjusted_alpha = 1.0 / (prod->rl_update_count + 1.0);
                            break;
                        case rl_param_container::logarithmic_decay:
                            adjusted_alpha = 1.0 / (log(prod->rl_update_count + 1.0) + 1.0);
                            break;
                        case rl_param_container::delta_bar_delta_decay:
                        {
                            // Note that in this case, x_i = 1.0 for all productions that are being updated.
                            // Those values have been included here for consistency with the algorithm as described in the delta bar delta paper.
                            prod->rl_delta_bar_delta_beta = prod->rl_delta_bar_delta_beta + theta * delta_t * 1.0 * prod->rl_delta_bar_delta_h;
                            adjusted_alpha = exp(prod->rl_delta_bar_delta_beta);
                            double decay_term = 1.0 - adjusted_alpha * 1.0 * 1.0;
                            if (decay_term < 0.0)
                            {
                                decay_term = 0.0;
                            }
                            prod->rl_delta_bar_delta_h = prod->rl_delta_bar_delta_h * decay_term + adjusted_alpha * delta_t * 1.0;
                            break;
                        }
                        case rl_param_container::normal_decay:
                        default:
                            adjusted_alpha = alpha;
                            break;
                    }

                    // calculate updates
                    delta_ecr = (adjusted_alpha * iter->second * (data->reward - sum_old_ecr));

                    if (update_efr)
                    {
                        delta_efr = (adjusted_alpha * iter->second * ((discount * op_value) - sum_old_efr));
                    }
                    else
                    {
                        delta_efr = 0.0;
                    }

                    // calculate new vals
                    new_ecr = (old_ecr + delta_ecr);
                    new_efr = (old_efr + delta_efr);
                    new_combined = (new_ecr + new_efr);
                    new_gql = old_gql + eta * (delta_ecr + delta_efr);

                    // print as necessary
                    if (thisAgent->trace_settings[ TRACE_RL_SYSPARAM ])
                    {
                        std::ostringstream ss;
                        ss << "RL update " << prod->name->sc->name << " "
                           << old_ecr << " " << old_efr << " " << old_ecr + old_efr << " -> "
                           << new_ecr << " " << new_efr << " " << new_combined ;

                        std::string temp_str(ss.str());
                        thisAgent->outputManager->printa_sf(thisAgent,  "%s\n", temp_str.c_str());
                        xml_generate_message(thisAgent, temp_str.c_str());

                        // Log update to file if the log file has been set
                        std::string log_path = thisAgent->RL->rl_params->update_log_path->get_value();
                        if (!log_path.empty())
                        {
                            std::ofstream file(log_path.c_str(), std::ios_base::app);
                            file << ss.str() << std::endl;
                            file.close();
                        }
                    }

                    // Change value of rule
                    Symbol* lSym = rhs_value_to_symbol(prod->action_list->referent);
                    deallocate_rhs_value(thisAgent, prod->action_list->referent);
                    prod->action_list->referent = allocate_rhs_value_for_symbol_no_refcount(thisAgent, thisAgent->symbolManager->make_float_constant(new_combined), 0, 0);

                    prod->rl_update_count += 1;
                    prod->rl_ecr = new_ecr;
                    prod->rl_efr = new_efr;
                    prod->rl_gql = new_gql;
                }

                if (thisAgent->RL->rl_params->learning_policy->get_value() & rl_param_container::gql)
                {
                    for (preference* pref = goal->id->operator_slot->preferences[ NUMERIC_INDIFFERENT_PREFERENCE_TYPE ]; pref; pref = pref->next)
                    {
                        pref->inst->prod->rl_efr -= alpha * gamma * (1 - lambda) * dot_w_e;
                    }

                    for (production_list::iterator p = data->prev_op_rl_rules->begin(); p != data->prev_op_rl_rules->end(); p++)
                    {
                        (*p)->rl_gql -= alpha * eta * dot_w_phi;
                    }
                }

                for (iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); iter++)
                {
                    production* prod = iter->first;

                    // change documentation
                    if (thisAgent->RL->rl_params->meta->get_value() == on)
                    {
                        if (prod->documentation)
                        {
                            free_memory_block_for_string(thisAgent, prod->documentation);
                        }
                        std::stringstream doc_ss;
                        const std::vector<std::pair<std::string, param_accessor<double> *> >& documentation_params = thisAgent->RL->rl_params->get_documentation_params();
                        for (std::vector<std::pair<std::string, param_accessor<double> *> >::const_iterator doc_params_it = documentation_params.begin();
                                doc_params_it != documentation_params.end(); ++doc_params_it)
                        {
                            doc_ss << doc_params_it->first << "=" << doc_params_it->second->get_param(prod) << ";";
                        }
                        prod->documentation = make_memory_block_for_string(thisAgent, doc_ss.str().c_str());

                        /*
                        std::string rlupdates( "rlupdates=" );
                        std::string val;
                        to_string( static_cast< uint64_t >( prod->rl_update_count ), val );
                        rlupdates.append( val );

                        prod->documentation = make_memory_block_for_string( thisAgent, rlupdates.c_str() );
                        */
                    }

                    // Change value of preferences generated by current instantiations of this rule
                    if (prod->instantiations)
                    {
                        for (instantiation* inst = prod->instantiations; inst; inst = inst->next)
                        {
                            for (preference* pref = inst->preferences_generated; pref; pref = pref->inst_next)
                            {
                                thisAgent->symbolManager->symbol_remove_ref(&pref->referent);
                                pref->referent = thisAgent->symbolManager->make_float_constant(new_combined);
                            }
                        }
                    }
                }
            }
        }

        data->gap_age = 0;
        data->hrl_age = 0;
        data->reward = 0.0;
    }
}

// clears eligibility traces
void rl_watkins_clear(agent* /*thisAgent*/, Symbol* goal)
{
    goal->id->rl_info->eligibility_traces->clear();
}
