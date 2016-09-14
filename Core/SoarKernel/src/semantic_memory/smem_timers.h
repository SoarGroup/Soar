/*
 * smem_timers.h
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_TIMERS_H_
#define CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_TIMERS_H_

#include "kernel.h"

#include "soar_module.h"

class smem_timer_container: public soar_module::timer_container
{
    public:
        soar_module::timer* total;
        soar_module::timer* storage;
        soar_module::timer* ncb_retrieval;
        soar_module::timer* query;
        soar_module::timer* api;
        soar_module::timer* init;
        soar_module::timer* hash;
        soar_module::timer* act;
        soar_module::timer* spreading;

        smem_timer_container(agent* thisAgent);
};

class smem_timer_level_predicate: public soar_module::agent_predicate<soar_module::timer::timer_level>
{
    public:
        smem_timer_level_predicate(agent* new_agent);
        bool operator()(soar_module::timer::timer_level val);
};

class smem_timer: public soar_module::timer
{
    public:
        smem_timer(const char* new_name, agent* new_agent, soar_module::timer::timer_level new_level);
};

#endif /* CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_TIMERS_H_ */
