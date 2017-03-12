/*
 * output_settings.h
 *
 *  Created on: Jul 14, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_OUTPUT_MANAGER_OUTPUT_SETTINGS_H_
#define CORE_SOARKERNEL_SRC_OUTPUT_MANAGER_OUTPUT_SETTINGS_H_

#include "kernel.h"
#include "soar_module.h"
#include "soar_db.h"
#include "sml_KernelSML.h"

class OM_Parameters: public soar_module::param_container
{
    public:

        OM_Parameters(agent* new_agent, uint64_t pOutput_sysparams[]);
        ~OM_Parameters();

        soar_module::integer_param* print_depth;
        soar_module::boolean_param* agent_writes;
        soar_module::boolean_param* agent_traces;
        soar_module::boolean_param* warnings;
        soar_module::boolean_param* echo_commands;

        soar_module::boolean_param* enabled;
        soar_module::boolean_param* callback_enabled;
        soar_module::boolean_param* stdout_enabled;

        soar_module::boolean_param* ctf;
        soar_module::boolean_param* clog;
        soar_module::boolean_param* help_cmd;
        soar_module::boolean_param* qhelp_cmd;

        void print_output_settings(agent* thisAgent);
        void print_output_summary(agent* thisAgent);

        void update_bool_setting(agent* thisAgent, soar_module::boolean_param* pChangedParam, sml::KernelSML* pKernelSML);
        void update_int_setting(agent* thisAgent, soar_module::integer_param* pChangedParam);
        void update_params_for_settings(agent* thisAgent);
        const std::string get_agent_channel_string(agent* thisAgent);
};


#endif /* CORE_SOARKERNEL_SRC_OUTPUT_MANAGER_OUTPUT_SETTINGS_H_ */
