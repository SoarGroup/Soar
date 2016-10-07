/*
 * explanation_settings.h
 *
 *  Created on: Sep 26, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_EXPLANATION_SETTINGS_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_EXPLANATION_SETTINGS_H_

#include "kernel.h"
#include "soar_module.h"

class Explainer_Parameters: public soar_module::param_container
{
    public:

        Explainer_Parameters(agent* new_agent);

        soar_module::boolean_param* all;
        soar_module::boolean_param* include_justifications;

        soar_module::boolean_param* list_all;
        soar_module::boolean_param* record_chunk;
        soar_module::boolean_param* explain_chunk;
        soar_module::boolean_param* explain_instantiation;
        soar_module::boolean_param* explain_contributors;
        soar_module::boolean_param* explanation_trace;
        soar_module::boolean_param* wm_trace;
        soar_module::boolean_param* formation;
        soar_module::boolean_param* constraints;
        soar_module::boolean_param* identity;
        soar_module::boolean_param* stats;

        soar_module::boolean_param* help_cmd;
        soar_module::boolean_param* qhelp_cmd;

        void print_explanation_settings(agent* thisAgent);

};



#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_EXPLANATION_SETTINGS_H_ */
