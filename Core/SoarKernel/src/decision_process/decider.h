/*
 * decider.h
 *
 *  Created on: Sep 11, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_DECISION_PROCESS_DECIDER_H_
#define CORE_SOARKERNEL_SRC_DECISION_PROCESS_DECIDER_H_

#include "kernel.h"
#include "decider_settings.h"

#include <string>

class SoarDecider
{
        friend cli::CommandLineInterface;
        friend decider_param_container;

    public:

        /* General methods */

        SoarDecider(agent* myAgent);
        ~SoarDecider() {};

        void before_run();
        void get_run_result_string(std::string &runResultStr);
        void clean_up_for_agent_deletion();

        /* Settings and cli command related functions */
        decider_param_container*    params;
        uint64_t                    settings[num_decider_settings];

    private:

        agent*                      thisAgent;
        Output_Manager*             outputManager;
        uint64_t					last_dc;
        uint64_t					last_fc;
        uint64_t					last_cl;

        void get_enabled_module_strings(std::string &enabledStr, std::string &disabledStr);
        int get_state_stack_string(std::string &stateStackStr);
};

#endif /* CORE_SOARKERNEL_SRC_DECISION_PROCESS_DECIDER_H_ */
