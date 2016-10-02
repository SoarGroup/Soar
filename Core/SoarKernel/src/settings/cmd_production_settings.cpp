#include "agent.h"
#include "output_manager.h"
#include "cmd_settings.h"

//#include "sml_KernelSML.h"
//#include "sml_Events.h"

production_param_container::production_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    excise_cmd = new soar_module::boolean_param("excise", on, new soar_module::f_predicate<boolean>());
    add(excise_cmd);
    firing_counts_cmd = new soar_module::boolean_param("firing-counts", on, new soar_module::f_predicate<boolean>());
    add(firing_counts_cmd);
    matches_cmd = new soar_module::boolean_param("matches", on, new soar_module::f_predicate<boolean>());
    add(matches_cmd);
    multi_attributes_cmd = new soar_module::boolean_param("multi-attributes", on, new soar_module::f_predicate<boolean>());
    add(multi_attributes_cmd);
    break_cmd = new soar_module::boolean_param("break", on, new soar_module::f_predicate<boolean>());
    add(break_cmd);
    find_cmd = new soar_module::boolean_param("find", on, new soar_module::f_predicate<boolean>());
    add(find_cmd);
    watch_cmd = new soar_module::boolean_param("watch", on, new soar_module::f_predicate<boolean>());
    add(watch_cmd);

    help_cmd = new soar_module::boolean_param("help", on, new soar_module::f_predicate<boolean>());
    add(help_cmd);
    qhelp_cmd = new soar_module::boolean_param("?", on, new soar_module::f_predicate<boolean>());
    add(qhelp_cmd);
}

void production_param_container::print_settings(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    outputManager->reset_column_indents();
    outputManager->set_column_indent(1, 31);
    outputManager->printa(thisAgent,    "==================================================================\n");
    outputManager->printa(thisAgent,    "-               Production Sub-Commands and Options              -\n");
    outputManager->printa(thisAgent,    "==================================================================\n");
    outputManager->printa_sf(thisAgent, "production %-[? | help]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "production break %-[--clear --print]\n");
    outputManager->printa_sf(thisAgent, "production break %---set <prod-name>\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "production excise %-<production-name>\n");
    outputManager->printa_sf(thisAgent, "production excise %-[--all --chunks --default ]\n");
    outputManager->printa_sf(thisAgent, "                  %-[--never-fired --rl       ]\n");
    outputManager->printa_sf(thisAgent, "                  %-[--task --templates --user]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "production find %-[--lhs --rhs         ] <pattern>\n");
    outputManager->printa_sf(thisAgent, "                %-[--show-bindings     ]\n");
    outputManager->printa_sf(thisAgent, "                %-[--chunks --nochunks ]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "production firing-counts %-[--all --chunks --default --rl]  [n]\n");
    outputManager->printa_sf(thisAgent, "                         %-[--task --templates --user    ]\n");
    outputManager->printa_sf(thisAgent, "production firing-counts %-<prod-name>\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "production matches %-[--names --count  ]  <prod-name>\n");
    outputManager->printa_sf(thisAgent, "                   %-[--timetags --wmes]\n");
    outputManager->printa_sf(thisAgent, "production matches %-[--names --count  ] [--assertions ]\n");
    outputManager->printa_sf(thisAgent, "                   %-[--timetags --wmes] [--retractions]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "production multi-attributes %-[symbol [n]]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "production watch %-[--disable --enable] <prod-name>\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of sub-commands:    help production\n");

}
void production_param_container::print_summary(agent* thisAgent)
{
    print_settings(thisAgent);
}
