/*
 * output_settings.cpp
 *
 *  Created on: Sep 14, 2016
 *      Author: mazzin
 */

#include "output_settings.h"

#include "agent.h"
#include "output_manager.h"

OM_Parameters::OM_Parameters(agent* new_agent, uint64_t pOutput_sysparams[]): soar_module::param_container(new_agent)
{
    pOutput_sysparams[OM_ECHO_COMMANDS] = 0;
    pOutput_sysparams[OM_WARNINGS]      = 1;
    pOutput_sysparams[OM_AGENT_WRITES]  = 1;
    pOutput_sysparams[OM_PRINT_DEPTH]   = 1;

    print_depth = new soar_module::integer_param("print-depth", pOutput_sysparams[OM_PRINT_DEPTH], new soar_module::gt_predicate<int64_t>(1, true), new soar_module::f_predicate<int64_t>());
    add(print_depth);

    echo_commands = new soar_module::boolean_param("echo-commands", pOutput_sysparams[OM_ECHO_COMMANDS] ? on : off, new soar_module::f_predicate<boolean>());
    add(echo_commands);
    warnings = new soar_module::boolean_param("warnings", pOutput_sysparams[OM_WARNINGS] ? on : off, new soar_module::f_predicate<boolean>());
    add(warnings);
    agent_writes = new soar_module::boolean_param("agent-writes", pOutput_sysparams[OM_AGENT_WRITES] ? on : off, new soar_module::f_predicate<boolean>());
    add(agent_writes);
    agent_traces = new soar_module::boolean_param("agent-trace",on, new soar_module::f_predicate<boolean>());
    add(agent_traces);

    /* Actual values initialized before printing because agent might not be created yet, and it is agent-specific */
    enabled = new soar_module::boolean_param("enabled", off, new soar_module::f_predicate<boolean>());
    add(enabled);
    stdout_enabled = new soar_module::boolean_param("console", off, new soar_module::f_predicate<boolean>());
    add(stdout_enabled);
    callback_enabled = new soar_module::boolean_param("callbacks", off, new soar_module::f_predicate<boolean>());
    add(callback_enabled);

    ctf = new soar_module::boolean_param("command-to-file", on, new soar_module::f_predicate<boolean>());
    add(ctf);
    clog = new soar_module::boolean_param("log", on, new soar_module::f_predicate<boolean>());
    add(clog);
    help_cmd = new soar_module::boolean_param("help", on, new soar_module::f_predicate<boolean>());
    add(help_cmd);
    qhelp_cmd = new soar_module::boolean_param("?", on, new soar_module::f_predicate<boolean>());
    add(qhelp_cmd);

}

void OM_Parameters::update_params_for_settings(agent* thisAgent)
{
    warnings->set_value(thisAgent->outputManager->settings[OM_WARNINGS] ? on : off);
    agent_writes->set_value(thisAgent->outputManager->settings[OM_AGENT_WRITES] ? on : off);
    stdout_enabled->set_value(thisAgent->outputManager->is_printing_to_stdout() ? on : off);
    callback_enabled->set_value(thisAgent->output_settings->callback_mode ? on : off);
    enabled->set_value(thisAgent->output_settings->print_enabled ? on : off);
}

void OM_Parameters::update_bool_setting(agent* thisAgent, soar_module::boolean_param* pChangedParam, sml::KernelSML* pKernelSML)
{
    if (pChangedParam == warnings)
    {
        thisAgent->outputManager->settings[OM_WARNINGS] = pChangedParam->get_value();
    }
    else if (pChangedParam == agent_writes)
    {
        thisAgent->outputManager->settings[OM_AGENT_WRITES] = pChangedParam->get_value();
    }

    else if (pChangedParam == stdout_enabled)
    {
        thisAgent->outputManager->set_printing_to_stdout(pChangedParam->get_value());
    }
    else if (pChangedParam == callback_enabled)
    {
        thisAgent->output_settings->callback_mode = pChangedParam->get_value();
    }
    else if (pChangedParam == enabled)
    {
        thisAgent->output_settings->print_enabled = pChangedParam->get_value();
    }
    else if (pChangedParam == echo_commands)
    {
        thisAgent->outputManager->settings[OM_ECHO_COMMANDS] = pChangedParam->get_value();
        pKernelSML->SetEchoCommands(thisAgent->outputManager->settings[OM_ECHO_COMMANDS]);
    }
}

void OM_Parameters::update_int_setting(agent* thisAgent, soar_module::integer_param* pChangedParam)
{
    if (pChangedParam == print_depth)
    {
        thisAgent->outputManager->settings[OM_PRINT_DEPTH] = pChangedParam->get_value();
    }
}

std::string concatJustified(const char* left_string, std::string right_string, int pWidth);

void OM_Parameters::print_output_summary(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    enabled->set_value(thisAgent->output_settings->print_enabled ? on : off);

    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 25);
    outputManager->set_column_indent(1, 58);
    outputManager->printa(thisAgent, "=======================================================\n");
    outputManager->printa(thisAgent, "-                   Output Status                     -\n");
    outputManager->printa(thisAgent, "=======================================================\n");
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Printing enabled", (thisAgent->output_settings->print_enabled ? "Yes" : "No"), 55).c_str());
    if (thisAgent->outputManager->is_printing_to_stdout())
    {
        outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Printing to std::out", (thisAgent->outputManager->is_printing_to_stdout() ? "Yes" : "No"), 55).c_str());
    }
    outputManager->printa(thisAgent, get_agent_channel_string(thisAgent).c_str());
    outputManager->printa(thisAgent, "\n");
    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Warnings", warnings->get_string(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Agent RHS write output", agent_writes->get_string(), 55).c_str());
#ifndef SOAR_RELEASE_VERSION
    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Soar release compilation", "OFF", 55).c_str());
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Debug printing", "ON", 55).c_str());
#endif
    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "To enable specific types of trace messages, use the 'watch' command.\n");
    outputManager->printa_sf(thisAgent, "Use 'output ?' for a command overview or 'help output' for the manual page.");
}

const std::string OM_Parameters::get_agent_channel_string(agent* thisAgent)
{
    std::ostringstream tempStringStream;
    bool allEnabled = true;
    for (int i=0; i < maxAgentTraces; ++i)
    {
        if (!thisAgent->output_settings->agent_traces_enabled[i])
        {
            allEnabled = false;
            break;
        }
    }
    if (allEnabled)
    {
        tempStringStream << "All agent trace channels enabled.";
    } else {
        tempStringStream << "All agent trace channels enabled except ";
        bool isFirst = true;
        for (int i=0; i < maxAgentTraces; ++i)
        {
            if (!thisAgent->output_settings->agent_traces_enabled[i])
            {
                if (!isFirst)
                {
                    if (i != (maxAgentTraces - 1))
                    {
                        tempStringStream << ", " << (i+1);
                    }
                } else {
                    tempStringStream << i;
                    isFirst = false;
                }
            }
        }
        tempStringStream << ".";
    }

    return tempStringStream.str();
}

void OM_Parameters::print_output_settings(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    /* These need to be updated here because they are agent-specific */
    enabled->set_value(thisAgent->output_settings->print_enabled ? on : off);
    stdout_enabled->set_value(thisAgent->outputManager->is_printing_to_stdout() ? on : off);
    callback_enabled->set_value(thisAgent->output_settings->callback_mode ? on : off);

    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 25);
    outputManager->set_column_indent(1, 58);
    outputManager->printa(thisAgent, "=======================================================\n");
    outputManager->printa(thisAgent, "-           Output Sub-Commands and Options           -\n");
    outputManager->printa(thisAgent, "=======================================================\n");
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("output", "[? | help]", 55).c_str());
    outputManager->printa_sf(thisAgent, "%s   %-Controls whether agent trace channel prints\n", concatJustified("agent-trace", "<channel-number> [on | off]", 55).c_str());
    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("enabled",enabled->get_string(), 55).c_str(), "Globally turn off all output");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("console", stdout_enabled->get_string(), 55).c_str(), "Send output to std::out for debugging");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("callbacks", callback_enabled->get_string(), 55).c_str(), "Send output to standard agent print callback");
    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("output log", "[--append | -A] <filename>", 55).c_str(), "Log all output to file");
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("output log", "--add <string>", 55).c_str());
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("output log", "[--close]", 55).c_str());
    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("output command-to-file", "[-a] <file> <cmd> [args]", 55).c_str(), "Log execution of single command");
    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("print-depth", print_depth->get_string(), 55).c_str(), "Default print depth for 'print'");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("agent-writes", agent_writes->get_string(), 55).c_str(), "Print output from RHS (write) function");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("warnings", warnings->get_string(), 55).c_str(), "Print all warnings");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("echo-commands", echo_commands->get_string(), 55).c_str(), "Echo commands to debugger");
    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "To view/change a setting: %-%- output <setting> [<value>]\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of these settings:  %-%- help output\n");
}
