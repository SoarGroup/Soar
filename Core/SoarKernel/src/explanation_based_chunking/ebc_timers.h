/*
 * ebc_timers.h
 *
 *  Created on: Feb 14, 2017
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_TIMERS_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_TIMERS_H_

#include "soar_module.h"

class ebc_timer_container: public soar_module::timer_container
{
    public:
        soar_module::timer* instantiation_creation;
        soar_module::timer* ebc_total;
        soar_module::timer* explainer_storage;
        soar_module::timer* dependency_analysis;
        soar_module::timer* dependency_analysis_osk;
        soar_module::timer* chunk_instantiation_creation;
        soar_module::timer* variablization_lhs;
        soar_module::timer* variablization_rhs;
        soar_module::timer* merging;
        soar_module::timer* repair;
        soar_module::timer* reorder;
        soar_module::timer* reinstantiate;
        soar_module::timer* add_to_rete;
        soar_module::timer* clean_up;
        soar_module::timer* osk_add;
        soar_module::timer* identity_add;
        soar_module::timer* identity_unification;
        soar_module::timer* identity_update;

        ebc_timer_container(agent* thisAgent);
};

class ebc_timer_level_predicate: public soar_module::agent_predicate<soar_module::timer::timer_level>
{
    public:
        ebc_timer_level_predicate(agent* new_agent);
        bool operator()(soar_module::timer::timer_level val);
};

class ebc_timer: public soar_module::timer
{
    public:
        ebc_timer(const char* new_name, agent* new_agent, timer_level new_level);
};
#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_TIMERS_H_ */
