/*
 * ebc_settings.h
 *
 *  Created on: Jul 12, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_SETTINGS_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_SETTINGS_H_

#include "kernel.h"

#include "soar_module.h"

class ebc_param_container: public soar_module::param_container
{
    public:

        soar_module::constant_param<EBCLearnChoices>* chunk_in_states;
        soar_module::constant_param<chunkNameFormats>* naming_style;
        soar_module::constant_param<singleton_element_type>* element_type;

        /* Parameters used as commands */
        soar_module::boolean_param* stats_cmd;
        soar_module::boolean_param* help_cmd;
        soar_module::boolean_param* qhelp_cmd;
        soar_module::boolean_param* always_cmd;
        soar_module::boolean_param* never_cmd;
        soar_module::boolean_param* flagged_cmd;
        soar_module::boolean_param* unflagged_cmd;
        soar_module::boolean_param* singleton;

        /* Settings */
        soar_module::integer_param* max_chunks;
        soar_module::integer_param* max_dupes;
        soar_module::boolean_param* bottom_level_only;
        soar_module::boolean_param* interrupt_on_chunk;
        soar_module::boolean_param* interrupt_on_warning;
        soar_module::boolean_param* interrupt_on_watched;

        /* Mechanisms */
        soar_module::boolean_param* mechanism_add_OSK;
        soar_module::boolean_param* mechanism_add_ltm_links;
        soar_module::boolean_param* mechanism_repair_rhs;
        soar_module::boolean_param* mechanism_repair_lhs;
        soar_module::boolean_param* mechanism_merge;
        soar_module::boolean_param* mechanism_user_singletons;

        /* Correctness filters */
        soar_module::boolean_param* allow_missing_negative_reasoning;
        soar_module::boolean_param* allow_opaque_knowledge;

        ebc_param_container(agent* new_agent, bool pEBC_settings[], uint64_t& pMaxChunks, uint64_t& pMaxDupes);
        void update_params(bool pEBC_settings[]);
        void update_ebc_settings(agent* thisAgent, soar_module::boolean_param* pChangedParam = NULL, soar_module::integer_param* pChangedIntParam = NULL);
};

#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_SETTINGS_H_ */
