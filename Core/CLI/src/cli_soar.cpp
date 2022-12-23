/////////////////////////////////////////////////////////////////
// learn command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_AgentSML.h"
#include "sml_Names.h"
#include "sml_KernelSML.h"
#include "sml_Utils.h"

#include "agent.h"
#include "soarversion.h"
#include "build_time_date.h"
#include "decider.h"
#include "decider_settings.h"
#include "ebc.h"
#include "ebc_settings.h"
#include "explanation_memory.h"
#include "output_manager.h"
#include "print.h"
#include "xml.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSoar(const char pOp, const std::string* pArg1, const std::string* pArg2, const std::string* pArg3)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempStringStream;
    std::string tempString;

    if (!pOp)
    {
        thisAgent->Decider->params->print_status(thisAgent);
        return true;
    }
    else if (pOp == 'G')
    {
        /* Single command argument */
        soar_module::param* my_param = thisAgent->Decider->params->get(pArg1->c_str());
        if (!my_param)
        {
            return SetError("Invalid soar sub-command.  Use 'soar ?' to see a list of valid sub-commands and settings.");
        }

        /* Process dummy parameters that are stand-in for commands */
        if (my_param == thisAgent->Decider->params->init_cmd)
        {
            // Save the current result
            std::string oldResult = m_Result.str();

            SetTrapPrintCallbacks(false);

            bool ok = m_pAgentSML->Reinitialize() ;

            // S1 gets created during Reinitialize, clear its output from the trace buffers
            xml_invoke_callback(m_pAgentSML->GetSoarAgent());
            m_pAgentSML->FlushPrintOutput();

            SetTrapPrintCallbacks(true);

            // restore the old result, ignoring output from init-soar
            m_Result.str(oldResult);

            if (!ok)
            {
                return SetError("Agent failed to reinitialize.");
            }

            if (m_RawOutput)
            {
                m_Result << "\nAgent reinitialized.\n";
            }

            return ok;
        }
        else if (my_param == thisAgent->Decider->params->stop_cmd)
        {
            if (pArg2 && !pArg2->empty() && !strcmp(pArg2->c_str(),"self"))
            {
                m_pAgentSML->Interrupt(sml::sml_STOP_AFTER_DECISION_CYCLE);
            }
            else
            {
                // Make sure the system stop event will be fired at the end of the run.
                // We used to call FireSystemStop() in this function, but that's no good because
                // it comes before the agent has stopped because interrupt only stops at the next
                // phase or similar boundary (so could be a long time off).
                // So instead we set a flag and allow system stop to fire at the end of the run.
                m_pKernelSML->RequireSystemStop(true) ;
                m_pKernelSML->InterruptAllAgents(sml::sml_STOP_AFTER_DECISION_CYCLE);
            }
            return true;
        }
        else if (my_param == thisAgent->Decider->params->version_cmd)
        {
            std::ostringstream timedatestamp;
            timedatestamp << kDatestamp << " " << kTimestamp;
            std::string sTimeDateStamp = timedatestamp.str();

            if (m_RawOutput)
            {
                m_Result << sml_Names::kSoarVersionValue << "\n";
                m_Result << "Build date: " << sTimeDateStamp.c_str() << " " ;

            }
            else
            {
                std::string temp;
                int major = MAJOR_VERSION_NUMBER;
                int minor = MINOR_VERSION_NUMBER;
                int micro = MICRO_VERSION_NUMBER;
                AppendArgTagFast(sml_Names::kParamVersionMajor, sml_Names::kTypeInt, to_string(major, temp));
                AppendArgTagFast(sml_Names::kParamVersionMinor, sml_Names::kTypeInt, to_string(minor, temp));
                AppendArgTagFast(sml_Names::kParamVersionMicro, sml_Names::kTypeInt, to_string(micro, temp));
                AppendArgTag(sml_Names::kParamBuildDate, sml_Names::kTypeString, sTimeDateStamp);
            }
            return true;
        }
        else if ((my_param == thisAgent->Decider->params->help_cmd) || (my_param == thisAgent->Decider->params->qhelp_cmd))
        {
            thisAgent->Decider->params->print_settings(thisAgent);
        }
        else if (my_param == thisAgent->Decider->params->stop_phase)
        {
            if (m_RawOutput)
            {
                m_Result << "Stop before " << my_param->get_string();
            }
            else
            {
                smlPhase stopPhase = m_pKernelSML->GetStopBefore();
                std::ostringstream buffer;
                buffer << stopPhase;
                AppendArgTagFast(sml_Names::kParamPhase, sml_Names::kTypeInt, buffer.str());
            }
        }
        else {
            /* Command was a valid ebc_param name, so print it's value */
            tempStringStream << my_param->get_name() << " is" ;
            PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
        }
        return true;
    }
    else if (pOp == 'S')
    {
        soar_module::param* my_param = thisAgent->Decider->params->get(pArg1->c_str());
        if (!my_param)
        {
            return SetError("Invalid command.  Use 'soar ?' to see a list of valid settings.");
        }

        std::string lArg;
        lArg.assign(pArg2->c_str());
        if (!my_param->validate_string(pArg2->c_str()))
        {
            if (my_param->validate_string(pArg3->c_str()))
            {
                lArg.assign(pArg3->c_str());
            }
            else
            {
                return SetError("Invalid argument. Use 'soar ?' to see a list of valid settings.");
            }
        }

        bool result = my_param->set_string(lArg.c_str());

        if (!result)
        {
            return SetError("That parameter could not be changed.");
        }
        else
        {
        }

        const char* lCmdName = pArg1->c_str();
        if (my_param == thisAgent->Decider->params->stop_phase)
        {
            thisAgent->Decider->settings[DECIDER_STOP_PHASE] = thisAgent->Decider->params->stop_phase->get_value();

            if (thisAgent->Decider->params->stop_phase->get_value() == APPLY_PHASE)
            {
                m_pKernelSML->SetStopBefore(sml::sml_APPLY_PHASE) ;
                PrintCLIMessage("Soar will now stop before the apply phase.");

            }
            else if (thisAgent->Decider->params->stop_phase->get_value() == DECISION_PHASE)
            {
                m_pKernelSML->SetStopBefore(sml::sml_DECISION_PHASE) ;
                PrintCLIMessage("Soar will now stop before the decision phase.");
            }
            else if (thisAgent->Decider->params->stop_phase->get_value() == INPUT_PHASE)
            {
                m_pKernelSML->SetStopBefore(sml::sml_INPUT_PHASE) ;
                PrintCLIMessage("Soar will now stop before the input phase.");
            }
            else if (thisAgent->Decider->params->stop_phase->get_value() == OUTPUT_PHASE)
            {
                m_pKernelSML->SetStopBefore(sml::sml_OUTPUT_PHASE) ;
                PrintCLIMessage("Soar will now stop before the output phase.");
            }
            else if (thisAgent->Decider->params->stop_phase->get_value() == PROPOSE_PHASE)
            {
                m_pKernelSML->SetStopBefore(sml::sml_PROPOSE_PHASE) ;
                PrintCLIMessage("Soar will now stop before the propose phase.");
            }
        }
        else if (my_param == thisAgent->Decider->params->keep_all_top_oprefs)
        {
            thisAgent->Decider->settings[DECIDER_KEEP_TOP_OPREFS] = thisAgent->Decider->params->keep_all_top_oprefs->get_value();
            thisAgent->outputManager->sprint_sf(tempString, "Soar will now %s retain top level preferences for items that are already o-supported.", thisAgent->Decider->settings[DECIDER_KEEP_TOP_OPREFS] ? "always" : "not");
            PrintCLIMessage(tempString.c_str());
        }
        else if (my_param == thisAgent->Decider->params->wait_snc)
        {
            thisAgent->Decider->settings[DECIDER_WAIT_SNC] = thisAgent->Decider->params->wait_snc->get_value();
            thisAgent->outputManager->sprint_sf(tempString, "Soar will now %s when a state doesn't change.", thisAgent->Decider->settings[DECIDER_WAIT_SNC] ? "wait instead of impassing" : "impasse");
            PrintCLIMessage(tempString.c_str());
        }
        else if (my_param == thisAgent->Decider->params->timers_enabled)
        {
            thisAgent->timers_enabled = thisAgent->Decider->params->timers_enabled->get_value();
            thisAgent->outputManager->sprint_sf(tempString, "Timers are now %s.", thisAgent->timers_enabled ? "enabled" : "disabled");
            PrintCLIMessage(tempString.c_str());
        }
        else if (my_param == thisAgent->Decider->params->tcl_enabled)
        {
            if (thisAgent->Decider->params->tcl_enabled->get_value() == true)
            {
                if (Soar_Instance::Get_Soar_Instance().is_Tcl_on())
                {
                    PrintCLIMessage("Tcl mode is already on.");
                } else {
                    std::string result;
                    result = this->m_pKernelSML->FireCliExtensionMessageEvent("on");
                    if (result.size() != 0) return SetError(result);
                }
            } else
            {
                if (Soar_Instance::Get_Soar_Instance().is_Tcl_on())
                {
                    PrintCLIMessage("Soar does not currently support turning Tcl off after it has been enabled.");
                    thisAgent->Decider->params->tcl_enabled->set_value(on);
                }
                else
                {
                    PrintCLIMessage("Tcl mode is already off.");
                }
            }
            return true;
        }
        else if (my_param == thisAgent->Decider->params->max_gp)
        {
            if (thisAgent->Decider->settings[DECIDER_MAX_GP] != thisAgent->Decider->params->max_gp->get_value())
            {
                m_GPMax = static_cast<size_t>(thisAgent->Decider->params->max_gp->get_value());
            }
            thisAgent->Decider->settings[DECIDER_MAX_GP] = thisAgent->Decider->params->max_gp->get_value();
            thisAgent->outputManager->sprint_sf(tempString, "The maximum number of rules gp can generate is now %u.", thisAgent->Decider->settings[DECIDER_MAX_GP]);
            PrintCLIMessage(tempString.c_str());
        }
        else if (my_param == thisAgent->Decider->params->max_dc_time)
        {
            thisAgent->Decider->settings[DECIDER_MAX_DC_TIME] = thisAgent->Decider->params->max_dc_time->get_value();
            if (thisAgent->Decider->settings[DECIDER_MAX_DC_TIME] > 0)
            {
                thisAgent->outputManager->sprint_sf(tempString, "Soar will now interrupt decisions after %u seconds.", thisAgent->Decider->settings[DECIDER_MAX_DC_TIME]);
                PrintCLIMessage(tempString.c_str());
            } else {
                PrintCLIMessage("Soar will no longer interrupt based on how how long a decision takes. (default)");
            }
        }
        else if (my_param == thisAgent->Decider->params->max_elaborations)
        {
            thisAgent->Decider->settings[DECIDER_MAX_ELABORATIONS] = thisAgent->Decider->params->max_elaborations->get_value();
            thisAgent->outputManager->sprint_sf(tempString, "The maximum number of elaborations in a phase is now %u.", thisAgent->Decider->settings[DECIDER_MAX_ELABORATIONS]);
            PrintCLIMessage(tempString.c_str());
        }
        else if (my_param == thisAgent->Decider->params->max_goal_depth)
        {
            thisAgent->Decider->settings[DECIDER_MAX_GOAL_DEPTH] = thisAgent->Decider->params->max_goal_depth->get_value();
            thisAgent->outputManager->sprint_sf(tempString, "The maximum goal depth is now %u.", thisAgent->Decider->settings[DECIDER_MAX_GOAL_DEPTH]);
            PrintCLIMessage(tempString.c_str());
        }
        else if (my_param == thisAgent->Decider->params->max_memory_usage)
        {
            thisAgent->Decider->settings[DECIDER_MAX_MEMORY_USAGE] = thisAgent->Decider->params->max_memory_usage->get_value();
            if (thisAgent->Decider->settings[DECIDER_MAX_MEMORY_USAGE] > 0)
            {
                thisAgent->outputManager->sprint_sf(tempString, "Soar will now interrupt execution if more than %u bytes of memory are used.  (This requires a special build of Soar.  See manual for more information.)", thisAgent->Decider->settings[DECIDER_MAX_MEMORY_USAGE]);
                PrintCLIMessage(tempString.c_str());
            } else {
                PrintCLIMessage("Soar will not interrupt execution based on memory usage. (default)");
            }
        }
        else if (my_param == thisAgent->Decider->params->max_nil_output_cycles)
        {
            thisAgent->Decider->settings[DECIDER_MAX_NIL_OUTPUT_CYCLES] = thisAgent->Decider->params->max_nil_output_cycles->get_value();
            thisAgent->outputManager->sprint_sf(tempString, "The maximum number of decision cycles without output before interrupting is now %u.  (used with run --output)", thisAgent->Decider->settings[DECIDER_MAX_NIL_OUTPUT_CYCLES]);
            PrintCLIMessage(tempString.c_str());
        }
        return result;
    }

    return true;
}
