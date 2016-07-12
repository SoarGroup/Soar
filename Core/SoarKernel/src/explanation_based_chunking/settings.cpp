#include "settings.h"
#include "agent.h"
#include "ebc.h"

#define setting_on(s) pEBC_settings[s] ? on : off

ebc_param_container::ebc_param_container(agent* new_agent, bool pEBC_settings[]): soar_module::param_container(new_agent)
{

    /* Set up the settings array that is used for quick access */
    pEBC_settings[SETTING_EBC_LEARNING_ON] = false;
    pEBC_settings[SETTING_EBC_ALWAYS] = false;
    pEBC_settings[SETTING_EBC_NEVER] = true;
    pEBC_settings[SETTING_EBC_ONLY] = false;
    pEBC_settings[SETTING_EBC_EXCEPT] = false;
    pEBC_settings[SETTING_EBC_BOTTOM_ONLY] = true;
    pEBC_settings[SETTING_EBC_INTERRUPT] = false;
    pEBC_settings[SETTING_EBC_IGNORE_DNB] = true;
    pEBC_settings[SETTING_EBC_IDENTITY_VRBLZ] = true;
    pEBC_settings[SETTING_EBC_CONSTRAINTS] = true;
    pEBC_settings[SETTING_EBC_RHS_VRBLZ] = true;
    pEBC_settings[SETTING_EBC_OSK] = false;
    pEBC_settings[SETTING_EBC_REPAIR_LHS] = true;
    pEBC_settings[SETTING_EBC_REPAIR_RHS] = true;
    pEBC_settings[SETTING_EBC_REPAIR_PROMOTION] = false;
    pEBC_settings[SETTING_EBC_MERGE] = true;
    pEBC_settings[SETTING_EBC_USER_SINGLETONS] = false;
    pEBC_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS] = true;
    pEBC_settings[SETTING_EBC_ALLOW_OSK] = true;
    pEBC_settings[SETTING_EBC_ALLOW_OPAQUE] = true;
    pEBC_settings[SETTING_EBC_ALLOW_PROB] = false;
    pEBC_settings[SETTING_EBC_ALLOW_CONFLATED] = true;
    pEBC_settings[SETTING_EBC_ALLOW_TEMPORAL_CONSTRAINT] = true;
    pEBC_settings[SETTING_EBC_ALLOW_LOCAL_PROMOTION] = true;

    // enabled
    enabled = new soar_module::constant_param<EBCLearnChoices>("learn", ebc_never, new soar_module::f_predicate<EBCLearnChoices>());
    enabled->add_mapping(ebc_always, "enabled");
    enabled->add_mapping(ebc_always, "on");
    enabled->add_mapping(ebc_always, "all");
    enabled->add_mapping(ebc_never, "disabled");
    enabled->add_mapping(ebc_never, "off");
    enabled->add_mapping(ebc_never, "none");
    enabled->add_mapping(ebc_only, "only");
    enabled->add_mapping(ebc_except, "all-except");
    add(enabled);

    bottom_level_only = new soar_module::boolean_param("bottom-only", setting_on(SETTING_EBC_BOTTOM_ONLY), new soar_module::f_predicate<boolean>());
    add(bottom_level_only);
    interrupt_on_chunk = new soar_module::boolean_param("interrupt", setting_on(SETTING_EBC_INTERRUPT), new soar_module::f_predicate<boolean>());
    add(interrupt_on_chunk);
    ignore_dnb = new soar_module::boolean_param("ignore-dnb", setting_on(SETTING_EBC_IGNORE_DNB), new soar_module::f_predicate<boolean>());
    add(ignore_dnb);

    // mechanisms
    mechanism_identity_analysis = new soar_module::boolean_param("variablize-identity", setting_on(SETTING_EBC_IDENTITY_VRBLZ), new soar_module::f_predicate<boolean>());
    add(mechanism_identity_analysis);
    mechanism_variablize_rhs_funcs = new soar_module::boolean_param("variablize-rhs-funcs", setting_on(SETTING_EBC_RHS_VRBLZ), new soar_module::f_predicate<boolean>());
    add(mechanism_variablize_rhs_funcs);
    mechanism_constraints = new soar_module::boolean_param("enforce-constraints", setting_on(SETTING_EBC_BOTTOM_ONLY), new soar_module::f_predicate<boolean>());
    add(mechanism_constraints);
    mechanism_OSK = new soar_module::boolean_param("add-osk", setting_on(SETTING_EBC_OSK), new soar_module::f_predicate<boolean>());
    add(mechanism_OSK);
    mechanism_repair_rhs = new soar_module::boolean_param("repair-rhs", setting_on(SETTING_EBC_REPAIR_RHS), new soar_module::f_predicate<boolean>());
    add(mechanism_repair_rhs);
    mechanism_repair_lhs = new soar_module::boolean_param("repair-lhs", setting_on(SETTING_EBC_REPAIR_LHS), new soar_module::f_predicate<boolean>());
    add(mechanism_repair_lhs);
    mechanism_promotion_tracking = new soar_module::boolean_param("repair-rhs-promotion", setting_on(SETTING_EBC_REPAIR_PROMOTION), new soar_module::f_predicate<boolean>());
    add(mechanism_promotion_tracking);
    mechanism_merge = new soar_module::boolean_param("merge", setting_on(SETTING_EBC_MERGE), new soar_module::f_predicate<boolean>());
    add(mechanism_merge);
    mechanism_user_singletons = new soar_module::boolean_param("user-singletons", setting_on(SETTING_EBC_USER_SINGLETONS), new soar_module::f_predicate<boolean>());
    add(mechanism_user_singletons);

    allow_missing_negative_reasoning = new soar_module::boolean_param("allow-local-negative-reasoning", setting_on(SETTING_EBC_ALLOW_LOCAL_NEGATIONS), new soar_module::f_predicate<boolean>());
    add(allow_missing_negative_reasoning);
    allow_missing_OSK = new soar_module::boolean_param("allow-missing-osk", setting_on(SETTING_EBC_ALLOW_OSK), new soar_module::f_predicate<boolean>());
    add(allow_missing_OSK);
    allow_smem_knowledge = new soar_module::boolean_param("allow-memory-subsystems", setting_on(SETTING_EBC_ALLOW_OPAQUE), new soar_module::f_predicate<boolean>());
    add(allow_smem_knowledge);
    allow_probabilistic_operators = new soar_module::boolean_param("allow-uncertain-operators", setting_on(SETTING_EBC_ALLOW_PROB), new soar_module::f_predicate<boolean>());
    add(allow_probabilistic_operators);
    allow_multiple_prefs = new soar_module::boolean_param("allow-multiple-prefs", setting_on(SETTING_EBC_ALLOW_CONFLATED), new soar_module::f_predicate<boolean>());
    add(allow_multiple_prefs);
    allow_temporal_constraint = new soar_module::boolean_param("allow-pre-existing-ltm", setting_on(SETTING_EBC_ALLOW_TEMPORAL_CONSTRAINT), new soar_module::f_predicate<boolean>());
    add(allow_temporal_constraint);
    allow_local_promotion = new soar_module::boolean_param("allow-local-promotion", setting_on(SETTING_EBC_ALLOW_LOCAL_PROMOTION), new soar_module::f_predicate<boolean>());
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

