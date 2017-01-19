/*
 * explanation_settings.cpp
 *
 *  Created on: Sep 26, 2016
 *      Author: mazzin
 */

#include "explanation_settings.h"

#include "output_manager.h"

Explainer_Parameters::Explainer_Parameters(agent* new_agent): soar_module::param_container(new_agent)
{
    all = new soar_module::boolean_param("all", off, new soar_module::f_predicate<boolean>());
    include_justifications = new soar_module::boolean_param("justifications", off, new soar_module::f_predicate<boolean>());
    only_print_chunk_identities = new soar_module::boolean_param("only-chunk-identities", on, new soar_module::f_predicate<boolean>());
    list_chunks = new soar_module::boolean_param("list-chunks", on, new soar_module::f_predicate<boolean>());
    list_justifications = new soar_module::boolean_param("list-justifications", on, new soar_module::f_predicate<boolean>());
    record_chunk = new soar_module::boolean_param("record", on, new soar_module::f_predicate<boolean>());
    explain_chunk = new soar_module::boolean_param("chunk", on, new soar_module::f_predicate<boolean>());
    explain_instantiation = new soar_module::boolean_param("instantiations", on, new soar_module::f_predicate<boolean>());
    explain_contributors = new soar_module::boolean_param("contributors", on, new soar_module::f_predicate<boolean>());
    explanation_trace = new soar_module::boolean_param("explanation-trace", on, new soar_module::f_predicate<boolean>());
    wm_trace = new soar_module::boolean_param("wm-trace", on, new soar_module::f_predicate<boolean>());
    formation = new soar_module::boolean_param("formation", on, new soar_module::f_predicate<boolean>());
    constraints = new soar_module::boolean_param("constraints", on, new soar_module::f_predicate<boolean>());
    identity = new soar_module::boolean_param("identity", on, new soar_module::f_predicate<boolean>());
    stats = new soar_module::boolean_param("stats", on, new soar_module::f_predicate<boolean>());

    help_cmd = new soar_module::boolean_param("help", on, new soar_module::f_predicate<boolean>());
    qhelp_cmd = new soar_module::boolean_param("?", on, new soar_module::f_predicate<boolean>());

    add(all);
    add(include_justifications);
    add(only_print_chunk_identities);
    add(list_chunks);
    add(list_justifications);
    add(record_chunk);
    add(explain_chunk);
    add(explain_instantiation);
    add(explain_contributors);
    add(explanation_trace);
    add(wm_trace);
    add(formation);
    add(constraints);
    add(identity);
    add(stats);
    add(help_cmd);
    add(qhelp_cmd);
}

void Explainer_Parameters::print_explanation_settings(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 40);
    outputManager->set_column_indent(1, 55);
    outputManager->printa_sf(thisAgent, "======= Explainer Commands and Settings =======\n");
    outputManager->printa_sf(thisAgent, "explain ? %-%-%s\n", "Print this help listing");
    outputManager->printa_sf(thisAgent, "---------------- What to Record ---------------\n");
    outputManager->printa_sf(thisAgent, "all                        %-%s%-%s\n", capitalizeOnOff(all->get_value()), "Whether to record all rules that are learned");
    outputManager->printa_sf(thisAgent, "justifications             %-%s%-%s\n", capitalizeOnOff(include_justifications->get_value()), "Whether to record justifications");
    outputManager->printa_sf(thisAgent, "record <chunk-name>        %-%-%s\n", "Record any chunks formed from a specific rule");
    outputManager->printa_sf(thisAgent, "list-chunks                %-%-%s\n", "List all rules learned");
    outputManager->printa_sf(thisAgent, "list-justifications        %-%-%s\n", "List all justifications learned");
    outputManager->printa_sf(thisAgent, "----------- Starting an Explanation -----------\n");
    outputManager->printa_sf(thisAgent, "chunk [<chunk name> | <chunk id> ]     %-%-%s\n", "Start discussing chunk");
    outputManager->printa_sf(thisAgent, "formation                  %-%-%s\n", "Describe initial formation of chunk");
    outputManager->printa_sf(thisAgent, "----------- Browsing an Explanation -----------\n");
    outputManager->printa_sf(thisAgent, "instantiation <inst id>    %-%-%s\n", "Explain instantiation");
    outputManager->printa_sf(thisAgent, "explanation-trace          %-%-%s\n", "Switch to explanation trace inspection");
    outputManager->printa_sf(thisAgent, "wm-trace                   %-%-%s\n", "Switch to working memory trace inspection");
    outputManager->printa_sf(thisAgent, "-------------- Supporting Analysis ----------------\n");
    outputManager->printa_sf(thisAgent, "constraints                %-%-%s\n", "Display extra transitive constraints required by problem-solving");
    outputManager->printa_sf(thisAgent, "identity                   %-%-%s\n", "Display identity to identity set mappings");
    outputManager->printa_sf(thisAgent, "only-chunk-identities      %-%s%-%s\n", capitalizeOnOff(only_print_chunk_identities->get_value()), "Identity analysis only prints identities sets found in chunk");
    outputManager->printa_sf(thisAgent, "stats                      %-%-%s\n", "Display statistics about currently discussed chunk");

    outputManager->printa_sf(thisAgent, "\nTo change a setting: %-%- explain <setting> [<value>]\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of these settings:  %-%-help explain\n");

}
