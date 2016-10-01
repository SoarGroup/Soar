#include "cmd_memory_settings.h"

#include "agent.h"
#include "output_manager.h"

//#include "sml_KernelSML.h"
//#include "sml_Events.h"

memory_param_container::memory_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    allocate_cmd = new soar_module::boolean_param("allocate", on, new soar_module::f_predicate<boolean>());
    add(allocate_cmd);
    memories_cmd = new soar_module::boolean_param("usage", on, new soar_module::f_predicate<boolean>());
    add(memories_cmd);
    help_cmd = new soar_module::boolean_param("help", on, new soar_module::f_predicate<boolean>());
    add(help_cmd);
    qhelp_cmd = new soar_module::boolean_param("?", on, new soar_module::f_predicate<boolean>());
    add(qhelp_cmd);
}

void memory_param_container::print_settings(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    outputManager->reset_column_indents();
//    outputManager->set_column_indent(0, 40);
    outputManager->set_column_indent(1, 20);
    outputManager->set_column_indent(2, 50);
    outputManager->printa(thisAgent,    "============= Memory Sub-Commands =============\n");
    outputManager->printa_sf(thisAgent, "memory %-[? | help] %-%s\n", "Print this help listing");
    outputManager->printa_sf(thisAgent, "memory allocate %-[pool blocks] %-%s\n", "Allocates additional memory to a memory pool");
    outputManager->printa_sf(thisAgent, "memory usage %-[options] [max] %-%s\n", "Prints partial memory usage for multiple rules");
    outputManager->printa_sf(thisAgent, "memory usage %-<production_name> %-%s\n", "Prints partial memory usage for a rule");

    outputManager->printa_sf(thisAgent, "For a detailed explanation of sub-commands:  %-%-help memory\n");

}
void memory_param_container::print_summary(agent* thisAgent)
{
    print_settings(thisAgent);
}
