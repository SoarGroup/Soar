#include "explanation_memory.h"

#include "explanation_settings.h"
#include "dprint.h"
#include "cli_CommandLineInterface.h"
#include "soar_instance.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"

void Explanation_Memory::create_after_action_report()
{
    if (settings->after_action_report->get_value() == off) return;

    /* Get CLI and Agent SML so we can call CLI's logging stuff */
    Soar_Instance*              lSoarInstance   = &(Soar_Instance::Get_Soar_Instance());
    cli::CommandLineInterface*  lCLI            = lSoarInstance->Get_CLI();
    sml::AgentSML*              lAgentSML       = lSoarInstance->Get_Agent_Info(thisAgent->name);

    // Close log
    if (lCLI->IsLogOpen())
    {
        lCLI->DoCommand(0, lAgentSML, "output log --close", false, true, 0) ;
    }

    after_action_report_file = "report_";
    after_action_report_file += lCLI->Get_First_Sourced_File();
    after_action_report_file += ".txt";

    dprint(DT_DEBUG, "Creating after action report %s\n", after_action_report_file.c_str());

    std::string lCmd, lCmdBase;

    lCmdBase = "output command-to-file reports/";
    lCmdBase += after_action_report_file;

    lCmd = lCmdBase;
    lCmd += " chunk stats";
    lCLI->DoCommand(0, lAgentSML, lCmd.c_str(), false, false, 0) ;

    lCmdBase = "output command-to-file -a reports/";
    lCmdBase += after_action_report_file;

    lCmd = lCmdBase;
    lCmd += " stats";
    lCLI->DoCommand(0, lAgentSML, lCmd.c_str(), false, false, 0) ;

    lCmd = lCmdBase;
    lCmd += " stats -m";
    lCLI->DoCommand(0, lAgentSML, lCmd.c_str(), false, false, 0) ;

    lCmd = lCmdBase;
    lCmd += " stats -M";
    lCLI->DoCommand(0, lAgentSML, lCmd.c_str(), false, false, 0) ;

}

void Explanation_Memory::after_action_report_for_init()
{
    if (settings->after_action_report->get_value() == off) return;
    after_action_report_for_exit();
}

void Explanation_Memory::after_action_report_for_exit()
{
    if (settings->after_action_report->get_value() == off) return;

    /* Get CLI and Agent SML so we can call CLI's logging stuff */
    Soar_Instance*              lSoarInstance   = &(Soar_Instance::Get_Soar_Instance());
    cli::CommandLineInterface*  lCLI            = lSoarInstance->Get_CLI();
    sml::AgentSML*              lAgentSML       = lSoarInstance->Get_Agent_Info(thisAgent->name);

    if (after_action_report_file.empty())
    {
        create_after_action_report();
    }
    std::string lCmd, lCmdBase;

    lCmdBase = "output command-to-file -a reports/";
    lCmdBase += after_action_report_file;

    for (auto it = (*chunks).begin(); it != (*chunks).end(); ++it)
    {
        Symbol* d1 = it->first;
        chunk_record* d2 = it->second;
        discuss_chunk(d2);
        lCmd = lCmdBase;
        lCmd += " explain stats";
        lCLI->DoCommand(0, lAgentSML, lCmd.c_str(), false, false, 0) ;
    }
}
