/*
 * ebc_settings.h
 *
 *  Created on: Jul 12, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_SETTINGS_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_SETTINGS_H_

#include "kernel.h"
#include "soar_module.h"

class ebc_param_container: public soar_module::param_container
{
    public:

        soar_module::constant_param<EBCLearnChoices>* enabled;
        soar_module::boolean_param* bottom_level_only;
        soar_module::boolean_param* interrupt_on_chunk;
        soar_module::boolean_param* ignore_dnb;

        soar_module::boolean_param* mechanism_identity_analysis;
        soar_module::boolean_param* mechanism_variablize_rhs_funcs;
        soar_module::boolean_param* mechanism_constraints;
        soar_module::boolean_param* mechanism_OSK;
        soar_module::boolean_param* mechanism_repair_rhs;
        soar_module::boolean_param* mechanism_repair_lhs;
        soar_module::boolean_param* mechanism_promotion_tracking;
        soar_module::boolean_param* mechanism_merge;
        soar_module::boolean_param* mechanism_user_singletons;

        soar_module::boolean_param* allow_missing_negative_reasoning;
        soar_module::boolean_param* allow_missing_OSK;
        soar_module::boolean_param* allow_smem_knowledge;
        soar_module::boolean_param* allow_probabilistic_operators;
        soar_module::boolean_param* allow_multiple_prefs;
        soar_module::boolean_param* allow_temporal_constraint;
        soar_module::boolean_param* allow_local_promotion;

        ebc_param_container(agent* new_agent);
        void update_ebc_settings(agent* thisAgent, soar_module::boolean_param* pChangedParam);
};


#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_SETTINGS_H_ */
