/*
 * smem_settings.h
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_SETTINGS_H_
#define CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_SETTINGS_H_

#include "kernel.h"

#include "soar_module.h"

class smem_path_param;

class smem_param_container: public soar_module::param_container
{
    public:
        enum db_choices { memory, file };
        enum cache_choices { cache_S, cache_M, cache_L };
        enum page_choices { page_1k, page_2k, page_4k, page_8k, page_16k, page_32k, page_64k };
        enum opt_choices { opt_safety, opt_speed };
        enum act_choices { act_recency, act_frequency, act_base };

        soar_module::boolean_param* learning;
        soar_module::constant_param<db_choices>* database;
        smem_path_param* path;
        soar_module::boolean_param* lazy_commit;
        soar_module::boolean_param* append_db;

        soar_module::constant_param<soar_module::timer::timer_level>* timers;

        soar_module::constant_param<page_choices>* page_size;
        soar_module::integer_param* cache_size;
        soar_module::constant_param<opt_choices>* opt;

        soar_module::integer_param* thresh;

        soar_module::boolean_param* activate_on_query;
        soar_module::boolean_param* activate_on_add;
        soar_module::constant_param<act_choices>* activation_mode;
        soar_module::decimal_param* base_decay;

        enum base_update_choices { bupt_stable, bupt_naive, bupt_incremental };
        soar_module::constant_param<base_update_choices>* base_update;
        soar_module::integer_param* base_unused_age_offset;

        soar_module::int_set_param* base_incremental_threshes;

        soar_module::boolean_param* spreading;
        soar_module::integer_param* spreading_limit;
        soar_module::integer_param* spreading_depth_limit;
        soar_module::decimal_param* spreading_baseline;
        soar_module::decimal_param* spreading_continue_probability;
        soar_module::boolean_param* spreading_loop_avoidance;
        soar_module::boolean_param* spreading_edge_updating;
        soar_module::boolean_param* spreading_wma_source;
        soar_module::decimal_param* spreading_edge_update_factor;
        soar_module::boolean_param* base_inhibition;

        void print_settings(agent* thisAgent);
        void print_summary(agent* thisAgent);
        soar_module::unsigned_integer_param* initial_variable_id;

        smem_param_container(agent* new_agent);
};

class smem_path_param: public soar_module::string_param
{
    protected:
        agent* thisAgent;

    public:
        smem_path_param(const char* new_name, const char* new_value, soar_module::predicate<const char*>* new_val_pred, soar_module::predicate<const char*>* new_prot_pred, agent* new_agent);
        virtual void set_value(const char* new_value);
};

template <typename T>
class smem_db_predicate: public soar_module::agent_predicate<T>
{
    public:
        smem_db_predicate(agent* new_agent);
        bool operator()(T val);
};




#endif /* CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_SETTINGS_H_ */
