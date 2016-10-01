/*
 * cli_command_parameters.h
 *
 *  Created on: Oct 1, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_INTERFACE_CLI_COMMAND_PARAMETERS_H_
#define CORE_SOARKERNEL_SRC_INTERFACE_CLI_COMMAND_PARAMETERS_H_

#include "kernel.h"
#include "soar_module.h"
#include "sml_KernelSML.h"

class memory_param_container: public soar_module::param_container
{
    public:

        soar_module::boolean_param* allocate_cmd;
        soar_module::boolean_param* memories_cmd;

        soar_module::boolean_param* help_cmd;
        soar_module::boolean_param* qhelp_cmd;

        memory_param_container(agent* new_agent);

        void print_settings(agent* thisAgent);
        void print_summary(agent* thisAgent);
};



#endif /* CORE_SOARKERNEL_SRC_INTERFACE_CLI_COMMAND_PARAMETERS_H_ */
