#include "agent.h"
#include "output_manager.h"
#include "cmd_settings.h"

//#include "sml_KernelSML.h"
//#include "sml_Events.h"

load_param_container::load_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    file_cmd = new soar_module::boolean_param("file", on, new soar_module::f_predicate<boolean>());
    add(file_cmd);
    input_cmd = new soar_module::boolean_param("percepts", on, new soar_module::f_predicate<boolean>());
    add(input_cmd);
    library_cmd = new soar_module::boolean_param("library", on, new soar_module::f_predicate<boolean>());
    add(library_cmd);
    rete_cmd = new soar_module::boolean_param("rete-network", on, new soar_module::f_predicate<boolean>());
    add(rete_cmd);
    help_cmd = new soar_module::boolean_param("help", on, new soar_module::f_predicate<boolean>());
    add(help_cmd);
    qhelp_cmd = new soar_module::boolean_param("?", on, new soar_module::f_predicate<boolean>());
    add(qhelp_cmd);
}

void load_param_container::print_settings(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    outputManager->reset_column_indents();
    outputManager->set_column_indent(1, 33);
    outputManager->printa(thisAgent,    "============================================================\n");
    outputManager->printa(thisAgent,    "-               Load Sub-Commands and Options              -\n");
    outputManager->printa(thisAgent,    "============================================================\n");
    outputManager->printa_sf(thisAgent, "load %-[? | help] %-%s\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "load file %-[--all --disable] %-<filename>\n");
    outputManager->printa_sf(thisAgent, "load file %-[--verbose]     ]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "load library %-<filename> <args...>\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "load rete-network %---load <filename>\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "load percepts %---open <filename>\n");
    outputManager->printa_sf(thisAgent, "load percepts %---close\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of sub-commands:        help load\n");
}
void load_param_container::print_summary(agent* thisAgent)
{
    print_settings(thisAgent);
}

save_param_container::save_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    input_cmd = new soar_module::boolean_param("percepts", on, new soar_module::f_predicate<boolean>());
    add(input_cmd);
    rete_cmd = new soar_module::boolean_param("rete-network", on, new soar_module::f_predicate<boolean>());
    add(rete_cmd);
    help_cmd = new soar_module::boolean_param("help", on, new soar_module::f_predicate<boolean>());
    add(help_cmd);
    qhelp_cmd = new soar_module::boolean_param("?", on, new soar_module::f_predicate<boolean>());
    add(qhelp_cmd);
}

void save_param_container::print_settings(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    outputManager->reset_column_indents();
    outputManager->set_column_indent(1, 38);
    outputManager->printa(thisAgent,    "======================================================\n");
    outputManager->printa(thisAgent,    "-            Save Sub-Commands and Options           -\n");
    outputManager->printa(thisAgent,    "======================================================\n");
    outputManager->printa_sf(thisAgent, "save %-[? | help]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "save percepts %---open <filename>\n");
    outputManager->printa_sf(thisAgent, "save percepts %-[--close --flush]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "save rete-network %---save <filename>\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of sub-commands:  help save\n");

}
void save_param_container::print_summary(agent* thisAgent)
{
    print_settings(thisAgent);
}
