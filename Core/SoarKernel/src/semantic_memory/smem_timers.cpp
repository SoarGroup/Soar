/*
 * smem_timers.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#include "semantic_memory.h"
#include "smem_timers.h"
#include "smem_settings.h"

smem_timer_container::smem_timer_container(agent* new_agent): soar_module::timer_container(new_agent)
{
    // one

    total = new smem_timer("_total", thisAgent, soar_module::timer::one);
    add(total);

    // two

    storage = new smem_timer("smem_storage", thisAgent, soar_module::timer::two);
    add(storage);

    ncb_retrieval = new smem_timer("smem_ncb_retrieval", thisAgent, soar_module::timer::two);
    add(ncb_retrieval);

    query = new smem_timer("smem_query", thisAgent, soar_module::timer::two);
    add(query);

    api = new smem_timer("smem_api", thisAgent, soar_module::timer::two);
    add(api);

    init = new smem_timer("smem_init", thisAgent, soar_module::timer::two);
    add(init);

    hash = new smem_timer("smem_hash", thisAgent, soar_module::timer::two);
    add(hash);

    act = new smem_timer("three_activation", thisAgent, soar_module::timer::three);
    add(act);

    spreading = new smem_timer("spreading", thisAgent, soar_module::timer::three);
    add(spreading);
    spreading_wma_1 = new smem_timer("spreading-wma-1", thisAgent, soar_module::timer::three);
    add(spreading_wma_1);
    spreading_wma_2 = new smem_timer("spreading-wma-2", thisAgent, soar_module::timer::three);
    add(spreading_wma_2);
    spreading_wma_3 = new smem_timer("spreading-wma-3", thisAgent, soar_module::timer::three);
    add(spreading_wma_3);
    spreading_1 = new smem_timer("spreading-1", thisAgent, soar_module::timer::three);
    add(spreading_1);
    spreading_2 = new smem_timer("spreading-2", thisAgent, soar_module::timer::three);
    add(spreading_2);
    spreading_3 = new smem_timer("spreading-3", thisAgent, soar_module::timer::three);
    add(spreading_3);
    spreading_4 = new smem_timer("spreading-4", thisAgent, soar_module::timer::three);
    add(spreading_4);
    spreading_5 = new smem_timer("spreading-5", thisAgent, soar_module::timer::three);
    add(spreading_5);
    spreading_6 = new smem_timer("spreading-6", thisAgent, soar_module::timer::three);
    add(spreading_6);
    spreading_7 = new smem_timer("spreading-7", thisAgent, soar_module::timer::three);
    add(spreading_7);
    spreading_7_1 = new smem_timer("spreading-7-1", thisAgent, soar_module::timer::three);
    add(spreading_7_1);
    spreading_7_2 = new smem_timer("spreading-7-2", thisAgent, soar_module::timer::three);
    add(spreading_7_2);
    spreading_7_2_1 = new smem_timer("spreading-7-2-1", thisAgent, soar_module::timer::three);
    add(spreading_7_2_1);
    spreading_7_2_2 = new smem_timer("spreading-7-2-2", thisAgent, soar_module::timer::three);
    add(spreading_7_2_2);
    spreading_7_2_3 = new smem_timer("spreading-7-2-3", thisAgent, soar_module::timer::three);
    add(spreading_7_2_3);
    spreading_7_2_4 = new smem_timer("spreading-7-2-4", thisAgent, soar_module::timer::three);
    add(spreading_7_2_4);
    spreading_7_2_5 = new smem_timer("spreading-7-2-5", thisAgent, soar_module::timer::three);
    add(spreading_7_2_5);
    spreading_7_2_6 = new smem_timer("spreading-7-2-6", thisAgent, soar_module::timer::three);
    add(spreading_7_2_6);
    spreading_7_2_7 = new smem_timer("spreading-7-2-7", thisAgent, soar_module::timer::three);
    add(spreading_7_2_7);
    spreading_7_2_8 = new smem_timer("spreading-7-2-8", thisAgent, soar_module::timer::three);
    add(spreading_7_2_8);
}

smem_timer_level_predicate::smem_timer_level_predicate(agent* new_agent): soar_module::agent_predicate<soar_module::timer::timer_level>(new_agent) {}

bool smem_timer_level_predicate::operator()(soar_module::timer::timer_level val)
{
    return (thisAgent->SMem->settings->timers->get_value() >= val);
}

smem_timer::smem_timer(const char* new_name, agent* new_agent, soar_module::timer::timer_level new_level): soar_module::timer(new_name, new_agent, new_level, new smem_timer_level_predicate(new_agent)) {}


