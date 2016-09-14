/*
 * decider_settings.cpp
 *
 *  Created on: Sep 11, 2016
 *      Author: mazzin
 */

#include "decider_settings.h"

#include "agent.h"
#include "decider.h"
#include "ebc.h"
#include "output_manager.h"

//#include "sml_Names.h"
//#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "sml_Events.h"

decider_param_container::decider_param_container(agent* new_agent, uint64_t pDecider_settings[]): soar_module::param_container(new_agent)
{
    pDecider_settings[DECIDER_MAX_GP] = 20000;
    pDecider_settings[DECIDER_MAX_DC_TIME] = 0;
    pDecider_settings[DECIDER_MAX_ELABORATIONS] = 100;
    pDecider_settings[DECIDER_MAX_GOAL_DEPTH] = 100;
    pDecider_settings[DECIDER_MAX_MEMORY_USAGE] = 100000000;
    pDecider_settings[DECIDER_MAX_NIL_OUTPUT_CYCLES] = 15;

    o_support_mode = new soar_module::constant_param<OSupportModes>("o-support-mode", OMode4, new soar_module::f_predicate<OSupportModes>());
    o_support_mode->add_mapping(OMode4, "4");
    o_support_mode->add_mapping(OMode3, "3");
    add(o_support_mode);

    stop_phase = new soar_module::constant_param<top_level_phase>("stop-phase", APPLY_PHASE, new soar_module::f_predicate<top_level_phase>());
    stop_phase->add_mapping(APPLY_PHASE, "apply");
    stop_phase->add_mapping(DECISION_PHASE, "decide");
    stop_phase->add_mapping(INPUT_PHASE, "input");
    stop_phase->add_mapping(OUTPUT_PHASE, "output");
    stop_phase->add_mapping(PROPOSE_PHASE, "propose");
    add(stop_phase);

    max_gp = new soar_module::integer_param("max-gp", pDecider_settings[DECIDER_MAX_GP], new soar_module::gt_predicate<int64_t>(1, true), new soar_module::f_predicate<int64_t>());
    add(max_gp);
    max_dc_time = new soar_module::integer_param("max-dc-time", pDecider_settings[DECIDER_MAX_DC_TIME], new soar_module::gt_predicate<int64_t>(0, true), new soar_module::f_predicate<int64_t>());
    add(max_dc_time);
    max_elaborations = new soar_module::integer_param("max-elaborations", pDecider_settings[DECIDER_MAX_ELABORATIONS], new soar_module::gt_predicate<int64_t>(0, true), new soar_module::f_predicate<int64_t>());
    add(max_elaborations);
    max_goal_depth = new soar_module::integer_param("max-goal-depth", pDecider_settings[DECIDER_MAX_GOAL_DEPTH], new soar_module::gt_predicate<int64_t>(1, true), new soar_module::f_predicate<int64_t>());
    add(max_goal_depth);
    max_memory_usage = new soar_module::integer_param("max-memory-usage", pDecider_settings[DECIDER_MAX_MEMORY_USAGE], new soar_module::gt_predicate<int64_t>(1, true), new soar_module::f_predicate<int64_t>());
    add(max_memory_usage);
    max_nil_output_cycles = new soar_module::integer_param("max-nil-output-cycles", pDecider_settings[DECIDER_MAX_NIL_OUTPUT_CYCLES], new soar_module::gt_predicate<int64_t>(1, true), new soar_module::f_predicate<int64_t>());
    add(max_nil_output_cycles);

    help_cmd = new soar_module::boolean_param("help", on, new soar_module::f_predicate<boolean>());
    add(help_cmd);
    qhelp_cmd = new soar_module::boolean_param("?", on, new soar_module::f_predicate<boolean>());
    add(qhelp_cmd);

}

void decider_param_container::update_bool_setting(agent* thisAgent, soar_module::boolean_param* pChangedParam )
{

}
void decider_param_container::update_int_setting(agent* thisAgent, soar_module::integer_param* pChangedParam)
{

    if (pChangedParam == max_gp)
    {
        thisAgent->Decider->settings[DECIDER_MAX_GP] = pChangedParam->get_value();
    }
    else if (pChangedParam == max_dc_time)
    {
        thisAgent->Decider->settings[DECIDER_MAX_DC_TIME] = pChangedParam->get_value();
    }
    else if (pChangedParam == max_elaborations)
    {
        thisAgent->Decider->settings[DECIDER_MAX_ELABORATIONS] = pChangedParam->get_value();
    }
    else if (pChangedParam == max_goal_depth)
    {
        thisAgent->Decider->settings[DECIDER_MAX_GOAL_DEPTH] = pChangedParam->get_value();
    }
    else if (pChangedParam == max_memory_usage)
    {
        thisAgent->Decider->settings[DECIDER_MAX_MEMORY_USAGE] = pChangedParam->get_value();
    }
    else if (pChangedParam == max_nil_output_cycles)
    {
        thisAgent->Decider->settings[DECIDER_MAX_NIL_OUTPUT_CYCLES] = pChangedParam->get_value();
    }

}
void decider_param_container::update_enum_setting(agent* thisAgent, soar_module::param* pChangedParam, sml::KernelSML* pKernelSML)
{
    if (pChangedParam == o_support_mode)
    {
        if (o_support_mode->get_value() == OMode4)
        {
            thisAgent->Decider->settings[DECIDER_O_SUPPORT_MODE] = 4;
        }
        else if (o_support_mode->get_value() == OMode3)
        {
            thisAgent->Decider->settings[DECIDER_O_SUPPORT_MODE] = 3;
        }
    }
    else if (pChangedParam == stop_phase)
    {
        thisAgent->Decider->settings[DECIDER_STOP_PHASE] = stop_phase->get_value();

        if (stop_phase->get_value() == APPLY_PHASE)
        {
            pKernelSML->SetStopBefore(sml::sml_APPLY_PHASE) ;
        }
        else if (stop_phase->get_value() == DECISION_PHASE)
        {
            pKernelSML->SetStopBefore(sml::sml_DECISION_PHASE) ;
        }
        else if (stop_phase->get_value() == INPUT_PHASE)
        {
            pKernelSML->SetStopBefore(sml::sml_INPUT_PHASE) ;
        }
        else if (stop_phase->get_value() == OUTPUT_PHASE)
        {
            pKernelSML->SetStopBefore(sml::sml_OUTPUT_PHASE) ;
        }
        else if (stop_phase->get_value() == PROPOSE_PHASE)
        {
            pKernelSML->SetStopBefore(sml::sml_PROPOSAL_PHASE) ;
        }
    }
}

void decider_param_container::print_soar_settings(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 40);
    outputManager->set_column_indent(1, 50);
    outputManager->printa(thisAgent, "=========== Soar General Settings ===========\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("stop-phase", stop_phase->get_string(), 45).c_str(), "Phase before which Soar will stop");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("max-elaborations", max_elaborations->get_string(), 45).c_str(), "Maximum elaboration in a phase");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("max-goal-depth", max_goal_depth->get_string(), 45).c_str(), "Maximum goal stack depth");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("max-nil-output-cycles", max_nil_output_cycles->get_string(), 45).c_str(), "Used with run --out");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("max-dc-time", max_dc_time->get_string(), 45).c_str(), "Maximum time per decision");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("max-memory-usage", max_memory_usage->get_string(), 45).c_str(), "Maximum memory usage");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("max-gp", max_gp->get_string(), 45).c_str(), "Maximum rules gp can generate");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("o-support-mode", o_support_mode->get_string(), 45).c_str(), "O-Support Mode");
    outputManager->printa(thisAgent, "---------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "\nTo change a setting: %-%- soar <setting> [<value>]\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of these settings:  %-%-help soar\n");
}
