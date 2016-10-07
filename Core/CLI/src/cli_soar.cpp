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

bool CommandLineInterface::DoSoar(const char pOp, const std::string* pAttr, const std::string* pVal)
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
        soar_module::param* my_param = thisAgent->Decider->params->get(pAttr->c_str());
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
            if (pVal && !pVal->empty() && !strcmp(pVal->c_str(),"self"))
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
        else {
            /* Command was a valid ebc_param name, so print it's value */
            tempStringStream << my_param->get_name() << " is" ;
            PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
        }
        return true;
    }
    else if (pOp == 'S')
    {
        soar_module::param* my_param = thisAgent->Decider->params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid command.  Use 'soar ?' to see a list of valid settings.");
        }

        if (!my_param->validate_string(pVal->c_str()))
        {
            return SetError("Invalid argument. Use 'soar ?' to see a list of valid settings.");
        }

        bool result = my_param->set_string(pVal->c_str());

        if (!result)
        {
            return SetError("That parameter could not be changed.");
        }
        else
        {
            tempStringStream << my_param->get_name() << " is now " << pVal->c_str();
            PrintCLIMessage(&tempStringStream);
        }
        const char* lCmdName = pAttr->c_str();
        if (!strcmp(lCmdName, "stop-phase"))
        {
            thisAgent->Decider->params->update_enum_setting(thisAgent, my_param, m_pKernelSML);
        } else if (
            !strcmp(lCmdName, "max-gp") ||
            !strcmp(lCmdName, "max-dc-time") ||
            !strcmp(lCmdName, "max-elaborations") ||
            !strcmp(lCmdName, "max-goal-depth") ||
            !strcmp(lCmdName, "max-memory-usage") ||
            !strcmp(lCmdName, "max-nil-output-cycles")) {
            uint64_t old_max_gp = thisAgent->Decider->settings[DECIDER_MAX_GP];
            thisAgent->Decider->params->update_int_setting(thisAgent, static_cast<soar_module::integer_param*>(my_param));
            if (old_max_gp != thisAgent->Decider->settings[DECIDER_MAX_GP])
            {
                m_GPMax = static_cast<size_t>(thisAgent->Decider->settings[DECIDER_MAX_GP]);
            }
        } else {
            thisAgent->Decider->params->update_bool_setting(thisAgent, static_cast<soar_module::boolean_param*>(my_param));
        }
        return result;
    }

    return true;
}
