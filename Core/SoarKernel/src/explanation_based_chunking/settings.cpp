#include "settings.h"
#include "agent.h"
#include "ebc.h"

ebc_param_container::ebc_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    // enabled
    enabled = new soar_module::constant_param<EBCLearnChoices>("learn", ebc_never, new soar_module::f_predicate<EBCLearnChoices>());
    enabled->add_mapping(ebc_always, "all");
    enabled->add_mapping(ebc_never, "never");
    enabled->add_mapping(ebc_only, "only");
    enabled->add_mapping(ebc_except, "all-except");
    add(enabled);

    bottom_level_only = new soar_module::boolean_param("bottom-only", on, new soar_module::f_predicate<boolean>());
    add(bottom_level_only);
    interrupt_on_chunk = new soar_module::boolean_param("interrupt", on, new soar_module::f_predicate<boolean>());
    add(interrupt_on_chunk);
    ignore_dnb = new soar_module::boolean_param("ignore-dnb", on, new soar_module::f_predicate<boolean>());
    add(ignore_dnb);

    // mechanisms
    mechanism_identity_analysis = new soar_module::boolean_param("variablize-identity", on, new soar_module::f_predicate<boolean>());
    add(mechanism_identity_analysis);
    mechanism_variablize_rhs_funcs = new soar_module::boolean_param("variablize-rhs-funcs", on, new soar_module::f_predicate<boolean>());
    add(mechanism_variablize_rhs_funcs);
    mechanism_constraints = new soar_module::boolean_param("enforce-constraints", on, new soar_module::f_predicate<boolean>());
    add(mechanism_constraints);
    mechanism_OSK = new soar_module::boolean_param("add-osk", off, new soar_module::f_predicate<boolean>());
    add(mechanism_OSK);
    mechanism_repair_rhs = new soar_module::boolean_param("repair-rhs", on, new soar_module::f_predicate<boolean>());
    add(mechanism_repair_rhs);
    mechanism_repair_lhs = new soar_module::boolean_param("repair-lhs", on, new soar_module::f_predicate<boolean>());
    add(mechanism_repair_lhs);
    mechanism_promotion_tracking = new soar_module::boolean_param("repair-rhs-promotion", on, new soar_module::f_predicate<boolean>());
    add(mechanism_promotion_tracking);
    mechanism_merge = new soar_module::boolean_param("merge", on, new soar_module::f_predicate<boolean>());
    add(mechanism_merge);
    mechanism_user_singletons = new soar_module::boolean_param("user-singletons", on, new soar_module::f_predicate<boolean>());
    add(mechanism_user_singletons);

    allow_missing_negative_reasoning = new soar_module::boolean_param("allow-local-negative-reasoning", on, new soar_module::f_predicate<boolean>());
    add(allow_missing_negative_reasoning);
    allow_missing_OSK = new soar_module::boolean_param("allow-missing-osk", on, new soar_module::f_predicate<boolean>());
    add(allow_missing_OSK);
    allow_smem_knowledge = new soar_module::boolean_param("allow-smem", on, new soar_module::f_predicate<boolean>());
    add(allow_smem_knowledge);
    allow_probabilistic_operators = new soar_module::boolean_param("allow-uncertain-operators", off, new soar_module::f_predicate<boolean>());
    add(allow_probabilistic_operators);
    allow_multiple_prefs = new soar_module::boolean_param("allow-multiple-prefs", on, new soar_module::f_predicate<boolean>());
    add(allow_multiple_prefs);
    allow_temporal_constraint = new soar_module::boolean_param("allow-pre-existing-ltm", on, new soar_module::f_predicate<boolean>());
    add(allow_temporal_constraint);
    allow_local_promotion = new soar_module::boolean_param("allow-local-promotion", on, new soar_module::f_predicate<boolean>());
    add(allow_local_promotion);

}

void ebc_param_container::update_ebc_settings(agent* thisAgent, soar_module::boolean_param* pChangedParam)
{
    if (!pChangedParam)
    {
        if (enabled->get_value() == ebc_always)
        {
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALWAYS] = true;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_NEVER] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = true;
        }
        else if (enabled->get_value() == ebc_never)
        {
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALWAYS] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_NEVER] = true;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = false;
        }
        else if (enabled->get_value() == ebc_only)
        {
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALWAYS] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_NEVER] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY] = true;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = true;
        }
        else if (enabled->get_value() == ebc_except)
        {
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALWAYS] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_NEVER] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY] = false;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT] = true;
            thisAgent->ebChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = true;
        }
        thisAgent->ebChunker->update_learning_on();
    }
    else if (pChangedParam == bottom_level_only)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_BOTTOM_ONLY] = pChangedParam->get_value();
    }
    else if (pChangedParam == interrupt_on_chunk)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_INTERRUPT] = pChangedParam->get_value();
    }
    else if (pChangedParam == ignore_dnb)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_IGNORE_DNB] = pChangedParam->get_value();
    }
    else if (pChangedParam == mechanism_identity_analysis)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_IDENTITY_VRBLZ] = pChangedParam->get_value();
    }
    else if (pChangedParam == mechanism_variablize_rhs_funcs)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_RHS_VRBLZ] = pChangedParam->get_value();
    }
    else if (pChangedParam == mechanism_constraints)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_CONSTRAINTS] = pChangedParam->get_value();
    }
    else if (pChangedParam == mechanism_OSK)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_OSK] = pChangedParam->get_value();
    }
    else if (pChangedParam == mechanism_repair_rhs)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_REPAIR_LHS] = pChangedParam->get_value();
    }
    else if (pChangedParam == mechanism_repair_lhs)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_REPAIR_RHS] = pChangedParam->get_value();
    }
    else if (pChangedParam == mechanism_promotion_tracking)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_REPAIR_PROMOTION] = pChangedParam->get_value();
    }
    else if (pChangedParam == mechanism_merge)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_MERGE] = pChangedParam->get_value();
    }
    else if (pChangedParam == mechanism_user_singletons)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_USER_SINGLETONS] = pChangedParam->get_value();
    }
    else if (pChangedParam == allow_missing_negative_reasoning)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS] = pChangedParam->get_value();
    }
    else if (pChangedParam == allow_missing_OSK)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALLOW_OSK] = pChangedParam->get_value();
    }
    else if (pChangedParam == allow_smem_knowledge)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALLOW_OPAQUE] = pChangedParam->get_value();
    }
    else if (pChangedParam == allow_probabilistic_operators)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALLOW_PROB] = pChangedParam->get_value();
    }
    else if (pChangedParam == allow_multiple_prefs)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALLOW_CONFLATED] = pChangedParam->get_value();
    }
    else if (pChangedParam == allow_temporal_constraint)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALLOW_TEMPORAL_CONSTRAINT] = pChangedParam->get_value();
    }
    else if (pChangedParam == allow_local_promotion)
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALLOW_LOCAL_PROMOTION] = pChangedParam->get_value();
    }
}

