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

class decide_param_container: public soar_module::param_container
{
    public:

        soar_module::boolean_param* indifferent_selection_cmd;
        soar_module::boolean_param* numeric_indifferent_mode_cmd;
        soar_module::boolean_param* predict_cmd;
        soar_module::boolean_param* select_cmd;
        soar_module::boolean_param* srand_cmd;
        soar_module::boolean_param* srand_bc_cmd;

        soar_module::boolean_param* help_cmd;
        soar_module::boolean_param* qhelp_cmd;

        decide_param_container(agent* new_agent);

        void print_settings(agent* thisAgent);
        void print_summary(agent* thisAgent);
};

class load_param_container: public soar_module::param_container
{
    public:

        soar_module::boolean_param* input_cmd;
        soar_module::boolean_param* file_cmd;
        soar_module::boolean_param* rete_cmd;
        soar_module::boolean_param* library_cmd;

        soar_module::boolean_param* help_cmd;
        soar_module::boolean_param* qhelp_cmd;

        load_param_container(agent* new_agent);

        void print_settings(agent* thisAgent);
        void print_summary(agent* thisAgent);
};

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

class production_param_container: public soar_module::param_container
{
    public:

        soar_module::boolean_param* excise_cmd;
        soar_module::boolean_param* firing_counts_cmd;
        soar_module::boolean_param* matches_cmd;
        soar_module::boolean_param* multi_attributes_cmd;
        soar_module::boolean_param* break_cmd;
        soar_module::boolean_param* find_cmd;
        soar_module::boolean_param* watch_cmd;

        soar_module::boolean_param* help_cmd;
        soar_module::boolean_param* qhelp_cmd;

        production_param_container(agent* new_agent);

        void print_settings(agent* thisAgent);
        void print_summary(agent* thisAgent);
};
class save_param_container: public soar_module::param_container
{
    public:

        soar_module::boolean_param* input_cmd;
        soar_module::boolean_param* rete_cmd;

        soar_module::boolean_param* help_cmd;
        soar_module::boolean_param* qhelp_cmd;

        save_param_container(agent* new_agent);

        void print_settings(agent* thisAgent);
        void print_summary(agent* thisAgent);
};

class wm_param_container: public soar_module::param_container
{
    public:

        soar_module::boolean_param* add_cmd;
        soar_module::boolean_param* remove_cmd;
        soar_module::boolean_param* watch_cmd;
        soar_module::boolean_param* wma_cmd;

        soar_module::boolean_param* help_cmd;
        soar_module::boolean_param* qhelp_cmd;

        wm_param_container(agent* new_agent);

        void print_settings(agent* thisAgent);
        void print_summary(agent* thisAgent);
};

#endif /* CORE_SOARKERNEL_SRC_INTERFACE_CLI_COMMAND_PARAMETERS_H_ */
