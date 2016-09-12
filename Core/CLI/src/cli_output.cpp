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
#include "ebc.h"
#include "explanation_memory.h"

#include "print.h"
#include "ebc_settings.h"
#include "output_manager.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoOutput(const char pOp, const std::string* pAttr, const std::string* pVal)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempStringStream;
    std::string tempString;

    if (!pOp)
    {
        thisAgent->explanationBasedChunker->print_chunking_summary();
        return true;
    }
    else if (pOp == 'G')
    {
        /* Single command argument */
        soar_module::param* my_param = thisAgent->explanationBasedChunker->ebc_params->get(pAttr->c_str());
        if (!my_param)
        {
            /* Command was not a valid ebc_param name, so it must be a single word command */
            /* Check if it's one of the four chunk enable commands.  (Means no ebc_param name
             * can be named enabled, on off, all disabled, none only all-except*/
            if(thisAgent->explanationBasedChunker->ebc_params->chunk_in_states->validate_string(pAttr->c_str()))
            {
                thisAgent->explanationBasedChunker->ebc_params->chunk_in_states->set_string(pAttr->c_str());
                tempStringStream << "Learns rules in states: " << pAttr->c_str();
                PrintCLIMessage(&tempStringStream);
                thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent, NULL);
                return true;
            } else {
                return SetError("Invalid chunking command.  Use 'chunk ?' to see a list of valid settings.");
            }
        }
        if (my_param == thisAgent->explanationBasedChunker->ebc_params->stats_cmd)
        {
            thisAgent->explanationMemory->print_global_stats();

        }
        else if (my_param == thisAgent->explanationBasedChunker->ebc_params->history_cmd)
        {
            PrintCLIMessage_Header("Chunking History", 60);
        }
        else if ((my_param == thisAgent->explanationBasedChunker->ebc_params->help_cmd) || (my_param == thisAgent->explanationBasedChunker->ebc_params->qhelp_cmd))
        {
            thisAgent->explanationBasedChunker->print_chunking_settings();
        }
        else {
            /* Command was a valid ebc_param name, so print it's value */
            tempStringStream << "Chunking parameter change:" << pAttr->c_str() << " =" ;
            PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
        }
        return true;
    }
    else if (pOp == 'S')
    {
        soar_module::param* my_param = thisAgent->explanationBasedChunker->ebc_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid chunking command.  Use 'chunk ?' to see a list of valid settings.");
        }

        if (!my_param->validate_string(pVal->c_str()))
        {
            return SetError("Invalid argument for chunking command. Use 'chunk ?' to see a list of valid settings.");
        }

        bool result = my_param->set_string(pVal->c_str());

        if (!result)
        {
            return SetError("The chunking parameter could not be changed.");
        }
        else
        {
            tempStringStream << "Chunking parameter " << pAttr->c_str() << " = " << pVal->c_str();
            PrintCLIMessage(&tempStringStream);
        }
        /* The following code assumes that all parameters except learn are boolean */
        if (!strcmp(pAttr->c_str(), "learn"))
        {
            thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent);
        } else if (!strcmp(pAttr->c_str(), "max-chunks") || !strcmp(pAttr->c_str(), "max-dupes")) {
            thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent, NULL, static_cast<soar_module::integer_param*>(my_param));
        } else {
            thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent, static_cast<soar_module::boolean_param*>(my_param));
        }
        return result;
    }
    else if (pOp == 's')
    {
        thisAgent->explanationMemory->print_global_stats();
        return true;
    }
    else if (pOp == 'h')
    {
        PrintCLIMessage_Header("History", 40);
        return true;
    }

    return true;
}
