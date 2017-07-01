#include "cmd_settings.h"

#include "agent.h"
#include "decider.h"
#include "decision_manipulation.h"
#include "exploration.h"
#include "output_manager.h"
#include "working_memory.h"
#include "working_memory_activation.h"

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
    outputManager->set_column_indent(1, 34);
    outputManager->set_column_indent(2, 70);

    outputManager->printa(thisAgent,    "=============================================================================\n");
    outputManager->printa(thisAgent,    "-                      Decide Sub-Commands and Options                      -\n");
    outputManager->printa(thisAgent,    "=============================================================================\n");
    outputManager->printa_sf(thisAgent, "decide %-[? | help]\n");
    outputManager->printa(thisAgent,    "-----------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "decide numeric-indifferent-mode %-[--avg --sum]\n");
    outputManager->printa(thisAgent,    "-----------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "decide indifferent-selection\n");
    outputManager->printa_sf(thisAgent, "decide indifferent-selection %-<policy>\n");
    outputManager->printa_sf(thisAgent, "                             %-<policy> = [--boltzmann | --epsilon-greedy |%-\n");
    outputManager->printa_sf(thisAgent, "                             %-            --first | --last | -- softmax ]%-\n");
    outputManager->printa_sf(thisAgent, "decide indifferent-selection %-<param> [value]\n");
    outputManager->printa_sf(thisAgent, "                             %-<param> = [--epsilon --temperature]\n");
    outputManager->printa_sf(thisAgent, "decide indifferent-selection %-[--reduction-policy | -p] <param> [<policy>]\n");
    outputManager->printa_sf(thisAgent, "decide indifferent-selection %-[--reduction-rate | -r] <param> <policy> [<rate>]\n");
    outputManager->printa_sf(thisAgent, "decide indifferent-selection %-[--auto-reduce] [setting]\n");
    outputManager->printa_sf(thisAgent, "decide indifferent-selection %-[--stats]\n");
    outputManager->printa(thisAgent,    "-----------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "decide predict\n");
    outputManager->printa_sf(thisAgent, "decide select %-<operator ID>\n");
    outputManager->printa(thisAgent,    "-----------------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "decide set-random-seed %-[<seed>] \n");
    outputManager->printa(thisAgent,    "-----------------------------------------------------------------------------\n\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of sub-commands:    help decide\n");
}

void decide_param_container::print_summary(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 55);

    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa_sf(thisAgent,    "                     Decide Summary\n");
    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Numeric indifference mode:",
        (thisAgent->numeric_indifferent_mode == NUMERIC_INDIFFERENT_MODE_AVG) ? "average" : "sum", 55).c_str());
    outputManager->printa(thisAgent,    "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Exploration Policy:",
        exploration_convert_policy(exploration_get_policy(thisAgent)), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Automatic Policy Parameter Reduction:",
        (thisAgent->Decider->settings[DECIDER_AUTO_REDUCE] ? ("on") : ("off")), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Epsilon:",
        std::to_string(exploration_get_parameter_value(thisAgent, EXPLORATION_PARAM_EPSILON)).c_str(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Epsilon Reduction Policy:",
        exploration_convert_reduction_policy(exploration_get_reduction_policy(thisAgent, EXPLORATION_PARAM_EPSILON)), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Temperature:",
        std::to_string(exploration_get_parameter_value(thisAgent, EXPLORATION_PARAM_TEMPERATURE)).c_str(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Temperature Reduction Policy:",
        exploration_convert_reduction_policy(exploration_get_reduction_policy(thisAgent, EXPLORATION_PARAM_TEMPERATURE)), 55).c_str());
    outputManager->printa(thisAgent,    "-------------------------------------------------------\n\n");
    outputManager->printa_sf(thisAgent, "Use 'decide ?' for a command overview or 'help decide' for the manual page.");
}

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
    outputManager->printa_sf(thisAgent, "load %-[? | help]\n");
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
    outputManager->printa(thisAgent,    "------------------------------------------------------------\n\n");
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
    chunks_cmd = new soar_module::boolean_param("chunks", on, new soar_module::f_predicate<boolean>());
    add(chunks_cmd);
    agent_cmd = new soar_module::boolean_param("agent", on, new soar_module::f_predicate<boolean>());
    add(agent_cmd);
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
    outputManager->printa_sf(thisAgent, "save [? | help]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "save agent %-<filename>\n");
    outputManager->printa_sf(thisAgent, "save chunks %-<filename>\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "save percepts %---open <filename>\n");
    outputManager->printa_sf(thisAgent, "save percepts %-[--close --flush]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "save rete-network %---save <filename>\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------\n\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of sub-commands:  help save\n");

}
void save_param_container::print_summary(agent* thisAgent)
{
    print_settings(thisAgent);
}

production_param_container::production_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    excise_cmd = new soar_module::boolean_param("excise", on, new soar_module::f_predicate<boolean>());
    add(excise_cmd);
    firing_counts_cmd = new soar_module::boolean_param("firing-counts", on, new soar_module::f_predicate<boolean>());
    add(firing_counts_cmd);
    matches_cmd = new soar_module::boolean_param("matches", on, new soar_module::f_predicate<boolean>());
    add(matches_cmd);
    memories_cmd = new soar_module::boolean_param("memory-usage", on, new soar_module::f_predicate<boolean>());
    add(memories_cmd);
    multi_attributes_cmd = new soar_module::boolean_param("optimize-attribute", on, new soar_module::f_predicate<boolean>());
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
    outputManager->set_column_indent(2, 70);

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
    outputManager->printa_sf(thisAgent, "                         %-[--task --templates --user --fired]\n");
    outputManager->printa_sf(thisAgent, "production firing-counts %-<prod-name>\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "production matches %-[--names --count  ]  <prod-name>\n");
    outputManager->printa_sf(thisAgent, "                   %-[--timetags --wmes]\n");
    outputManager->printa_sf(thisAgent, "production matches %-[--names --count  ] [--assertions ]\n");
    outputManager->printa_sf(thisAgent, "                   %-[--timetags --wmes] [--retractions]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "production memory-usage   %-[options] [max] %-\n");
    outputManager->printa_sf(thisAgent, "production memory-usage   %-<production_name> %-\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "production optimize-attribute [symbol [n]]\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "production watch %-[--disable --enable] <prod-name>\n");
    outputManager->printa(thisAgent,    "------------------------------------------------------------------\n\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of sub-commands:    help production\n");
}

void production_param_container::print_summary(agent* thisAgent)
{
    uint64_t totalProductions = thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];
    totalProductions += thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE];
    totalProductions += thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];
    totalProductions += thisAgent->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE];

    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 25);
    outputManager->set_column_indent(1, 58);
    outputManager->printa(thisAgent, "=======================================================\n");
    outputManager->printa(thisAgent, "-                     Productions                     -\n");
    outputManager->printa(thisAgent, "=======================================================\n");
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("User rules", std::to_string(thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE]), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Default rules", std::to_string(thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE]), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Chunks", std::to_string(thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE]), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Justifications", std::to_string(thisAgent->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE]), 55).c_str());
    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Total", std::to_string(totalProductions), 55).c_str());
    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "\nUse 'production ?' for a command overview or 'help production' for the manual page.");
}


wm_param_container::wm_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    add_cmd = new soar_module::boolean_param("add", on, new soar_module::f_predicate<boolean>());
    add(add_cmd);
    remove_cmd = new soar_module::boolean_param("remove", on, new soar_module::f_predicate<boolean>());
    add(remove_cmd);

//    find_cmd = new soar_module::boolean_param("find", on, new soar_module::f_predicate<boolean>());
//    add(find_cmd);

    watch_cmd = new soar_module::boolean_param("watch", on, new soar_module::f_predicate<boolean>());
    add(watch_cmd);
    wma_cmd = new soar_module::boolean_param("activation", on, new soar_module::f_predicate<boolean>());
    add(wma_cmd);

    help_cmd = new soar_module::boolean_param("help", on, new soar_module::f_predicate<boolean>());
    add(help_cmd);
    qhelp_cmd = new soar_module::boolean_param("?", on, new soar_module::f_predicate<boolean>());
    add(qhelp_cmd);
}

void wm_param_container::print_settings(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    outputManager->reset_column_indents();
    outputManager->set_column_indent(1, 17);
    outputManager->set_column_indent(2, 51);
    outputManager->set_column_indent(3, 61);
    outputManager->printa(thisAgent,    "=========================================================\n");
    outputManager->printa(thisAgent,    "-               WM Sub-Commands and Options             -\n");
    outputManager->printa(thisAgent,    "=========================================================\n");
    outputManager->printa_sf(thisAgent, "wm %-[? | help]\n");
    outputManager->printa(thisAgent,    "---------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "wm add    %-<id> [^]<attribute> <value> [+]\n");
    outputManager->printa_sf(thisAgent, "wm remove %-<timetag>\n");
    outputManager->printa(thisAgent,    "---------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "wm activation %---get <parameter>         \n");
    outputManager->printa_sf(thisAgent, "              %---set <parameter>     %-<value> \n");
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("                      activation", capitalizeOnOff(thisAgent->WM->wma_params->activation->get_value()), 57).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("                      petrov-approx", capitalizeOnOff(thisAgent->WM->wma_params->petrov_approx->get_value()), 57).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("                      forgetting", capitalizeOnOff(!strcmp(thisAgent->WM->wma_params->forgetting->get_cstring(), "off")), 57).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("                      fake-forgetting", capitalizeOnOff(thisAgent->WM->wma_params->fake_forgetting->get_value()), 57).c_str());
    outputManager->printa_sf(thisAgent, "%s%-%s\n", concatJustified("                      forget-wme",  thisAgent->WM->wma_params->forget_wme->get_cstring(), 57).c_str(), "[all, lti]");
    outputManager->printa_sf(thisAgent, "%s%-%s\n", concatJustified("                      decay-rate",  thisAgent->WM->wma_params->decay_rate->get_cstring(), 57).c_str(), "[0 to 1]");
    outputManager->printa_sf(thisAgent, "%s%-%s\n", concatJustified("                      decay-thresh",  thisAgent->WM->wma_params->decay_thresh->get_cstring(), 57).c_str(), "[0 to infinity]");
    outputManager->printa_sf(thisAgent, "%s%-%s\n", concatJustified("                      max-pow-cache",  thisAgent->WM->wma_params->max_pow_cache->get_cstring(), 57).c_str(), "MB");
    outputManager->printa_sf(thisAgent, "%s%-%s\n", concatJustified("                      timers",  thisAgent->WM->wma_params->timers->get_cstring(), 57).c_str(), "[off, one]");
    outputManager->printa_sf(thisAgent, "              %---history <timetag>\n");
    outputManager->printa_sf(thisAgent, "              %---stats             %-%-Prints forgetting stats\n");
    outputManager->printa_sf(thisAgent, "              %---timers [<timer>]  %-%-Prints timing results\n");
    outputManager->printa_sf(thisAgent, "                  <timer> = wma_forgetting or wma_history\n");
    outputManager->printa(thisAgent,    "---------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "wm watch %---add-filter    --type <t>  pattern\n");
    outputManager->printa_sf(thisAgent, "         %---remove-filter --type <t>  pattern\n");
    outputManager->printa_sf(thisAgent, "         %---list-filter  [--type <t>]\n");
    outputManager->printa_sf(thisAgent, "         %---reset-filter [--type <t>]\n");
    outputManager->printa_sf(thisAgent, "                              <t> = adds, removes or both\n");
    outputManager->printa(thisAgent,    "---------------------------------------------------------\n\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of sub-commands:       help wm\n");
}
void wm_param_container::print_summary(agent* thisAgent)
{
    print_settings(thisAgent);
}
