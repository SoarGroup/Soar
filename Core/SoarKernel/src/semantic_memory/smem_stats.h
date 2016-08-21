/*
 * smem_stats.h
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_STATS_H_
#define CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_STATS_H_

#include "kernel.h"

//#include "soar_db.h"

class smem_db_lib_version_stat;
class smem_mem_usage_stat;
class smem_mem_high_stat;

class smem_stat_container: public soar_module::stat_container
{
    public:
        smem_db_lib_version_stat* db_lib_version;
        smem_mem_usage_stat* mem_usage;
        smem_mem_high_stat* mem_high;

        soar_module::integer_stat* expansions;
        soar_module::integer_stat* cbr;
        soar_module::integer_stat* stores;
        soar_module::integer_stat* act_updates;
        soar_module::integer_stat* mirrors;

        soar_module::integer_stat* chunks;
        soar_module::integer_stat* slots;

        smem_stat_container(agent* thisAgent);
};

//

class smem_db_lib_version_stat: public soar_module::primitive_stat< const char* >
{
    protected:
        agent* thisAgent;

    public:
        smem_db_lib_version_stat(agent* new_agent, const char* new_name, const char* new_value, soar_module::predicate< const char* >* new_prot_pred);
        const char* get_value();
};

//

class smem_mem_usage_stat: public soar_module::integer_stat
{
    protected:
        agent* thisAgent;

    public:
        smem_mem_usage_stat(agent* new_agent, const char* new_name, int64_t new_value, soar_module::predicate<int64_t>* new_prot_pred);
        int64_t get_value();
};

//

class smem_mem_high_stat: public soar_module::integer_stat
{
    protected:
        agent* thisAgent;

    public:
        smem_mem_high_stat(agent* new_agent, const char* new_name, int64_t new_value, soar_module::predicate<int64_t>* new_prot_pred);
        int64_t get_value();
};



#endif /* CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_STATS_H_ */
