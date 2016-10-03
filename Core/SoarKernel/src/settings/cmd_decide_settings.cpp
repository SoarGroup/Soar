#include "agent.h"
#include "output_manager.h"
#include "cmd_settings.h"

//#include "sml_KernelSML.h"
//#include "sml_Events.h"

decide_param_container::decide_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    indifferent_selection_cmd = new soar_module::boolean_param("indifferent-selection", on, new soar_module::f_predicate<boolean>());
    add(indifferent_selection_cmd);
    numeric_indifferent_mode_cmd = new soar_module::boolean_param("numeric-indifferent-mode", on, new soar_module::f_predicate<boolean>());
    add(numeric_indifferent_mode_cmd);
    predict_cmd = new soar_module::boolean_param("predict", on, new soar_module::f_predicate<boolean>());
    add(predict_cmd);
    select_cmd = new soar_module::boolean_param("select", on, new soar_module::f_predicate<boolean>());
    add(select_cmd);
    srand_cmd = new soar_module::boolean_param("set-random-seed", on, new soar_module::f_predicate<boolean>());
    add(srand_cmd);
    srand_bc_cmd = new soar_module::boolean_param("srand", on, new soar_module::f_predicate<boolean>());
    add(srand_bc_cmd);

    help_cmd = new soar_module::boolean_param("help", on, new soar_module::f_predicate<boolean>());
    add(help_cmd);
    qhelp_cmd = new soar_module::boolean_param("?", on, new soar_module::f_predicate<boolean>());
    add(qhelp_cmd);
}

void decide_param_container::print_settings(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    outputManager->reset_column_indents();
//    outputManager->set_column_indent(0, 40);
    outputManager->set_column_indent(1, 33);
    outputManager->set_column_indent(2, 50);
    outputManager->printa(thisAgent,    "============= Decide Sub-Commands =============\n");
    outputManager->printa_sf(thisAgent, "decide %-[? | help] %-%s\n", "Print this help listing");
    outputManager->printa_sf(thisAgent, "decide indifferent-selection %- %-%s\n", "Does stuff");
    outputManager->printa_sf(thisAgent, "decide numeric-indifferent-mode %- %-%s\n", "Does stuff");
    outputManager->printa_sf(thisAgent, "decide predict %- %-%s\n", "Does stuff");
    outputManager->printa_sf(thisAgent, "decide select %- %-%s\n", "Does stuff");
    outputManager->printa_sf(thisAgent, "decide set-random-seed %- %-%s\n", "Does stuff\n");

    outputManager->printa_sf(thisAgent, "For a detailed explanation of sub-commands:  %-%-help decide\n");

}
void decide_param_container::print_summary(agent* thisAgent)
{
    print_settings(thisAgent);
}
