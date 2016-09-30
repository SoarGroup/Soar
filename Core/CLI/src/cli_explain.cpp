/////////////////////////////////////////////////////////////////
// explain command file.
//
// Author: Mazin Assanie
// Date  : 2015
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "sml_Names.h"
#include "sml_Utils.h"

#include "agent.h"
#include "condition.h"
#include "explanation_memory.h"
#include "misc.h"
#include "output_manager.h"

#include <string>

using namespace cli;
using namespace sml;

bool is_explain_id(const std::string* pStringParameter)
{
    return false;
}

bool CommandLineInterface::DoExplain(const std::string* pArg1, const std::string* pArg2)
{

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempStringStream;

    if (!pArg1)
    {
        thisAgent->explanationMemory->print_explain_summary();
        return true;
    }
    else
    {
        /* Single command argument */
        soar_module::param* my_param = thisAgent->explanationMemory->settings->get(pArg1->c_str());
        if (!my_param)
        {
            return SetError("Invalid explain sub-command.  Use 'explain ?' to see a list of valid sub-commands and settings.");
        }
        else if (my_param == thisAgent->explanationMemory->settings->all)
        {
            if (!pArg2)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "The explainer is%scurrently recording all rules formed.\n", thisAgent->explanationMemory->enabled() ? " " : " not ");

            } else {
                if (!my_param->validate_string(pArg2->c_str()) || (!my_param->set_string(pArg2->c_str())))
                {
                    return SetError("Invalid argument for explain command. Use 'explain ?' to see a list of valid sub-commands and settings.");
                }
                if (thisAgent->explanationMemory->settings->all->get_value())
                {
                    thisAgent->explanationMemory->set_enabled(true);
                    thisAgent->outputManager->printa_sf(thisAgent, "Will monitor all chunks created.\n");
                } else {
                    thisAgent->explanationMemory->set_enabled(false);
                    thisAgent->outputManager->printa_sf(thisAgent, "Will only monitor specific chunks.\n");
                }
            }
            return true;
        }
        else if (my_param == thisAgent->explanationMemory->settings->include_justifications)
        {
            if (!pArg2)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "The explainer is%scurrently recording justifications created.\n", thisAgent->explanationMemory->isRecordingJustifications() ? " " : " not ");

            } else {
                if (!my_param->validate_string(pArg2->c_str()) || (!my_param->set_string(pArg2->c_str())))
                {
                    return SetError("Invalid argument for explain command. Use 'explain ?' to see a list of valid sub-commands and settings.");
                }
                if (thisAgent->explanationMemory->settings->include_justifications->get_value())
                {
                    thisAgent->explanationMemory->set_justifications_enabled(true);
                    thisAgent->outputManager->printa_sf(thisAgent, "Will record justifications created.\n");
                } else {
                    thisAgent->explanationMemory->set_justifications_enabled(false);
                    thisAgent->outputManager->printa_sf(thisAgent, "Will not record justifications created.\n");
                }
            }
            return true;
        }
        else if (my_param == thisAgent->explanationMemory->settings->list_all)
        {
            thisAgent->explanationMemory->print_all_chunks();
            return true;
        }
        else if (my_param == thisAgent->explanationMemory->settings->record_chunk)
        {
            if (!pArg2)
            {
                thisAgent->explanationMemory->print_all_watched_rules();
                return true;
            } else {
                return thisAgent->explanationMemory->watch_rule(pArg2);
            }
        }
        else if (my_param == thisAgent->explanationMemory->settings->explain_chunk)
        {
            if (!pArg2)
            {
                return SetError("Please specify the chunk using either the chunk's name or its ID.");
            } else {
                return thisAgent->explanationMemory->explain_chunk(pArg2);
            }
        }
        else if (my_param == thisAgent->explanationMemory->settings->explain_instantiation)
        {
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to discuss with the command 'explain chunk [chunk-name | chunk ID]'.");
            }
            if (!pArg2)
            {
                return SetError("Please specify the instantiation id that you'd like explained.");
            } else {
                return thisAgent->explanationMemory->explain_instantiation(pArg2);
            }
        }
        else if (my_param == thisAgent->explanationMemory->settings->explain_contributors)
        {
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to discuss with the command 'explain chunk [chunk-name | chunk ID]'.");
            }
        }
        else if (my_param == thisAgent->explanationMemory->settings->explanation_trace)
        {
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to discuss with the command 'explain chunk [chunk-name | chunk ID]'.");
            }
            thisAgent->explanationMemory->switch_to_explanation_trace(true);
            return true;
        }
        else if (my_param == thisAgent->explanationMemory->settings->wm_trace)
        {
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to discuss with the command 'explain chunk [chunk-name | chunk ID]'.");
            }
            thisAgent->explanationMemory->switch_to_explanation_trace(false);
            return true;
        }
        else if (my_param == thisAgent->explanationMemory->settings->formation)
        {
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to discuss with the command 'explain chunk [chunk-name | chunk ID]'.");
            }
            thisAgent->explanationMemory->print_formation_explanation();
            return true;
        }
        else if (my_param == thisAgent->explanationMemory->settings->constraints)
        {
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to discuss with the command 'explain chunk [chunk-name | chunk ID]'.");
            }
            thisAgent->explanationMemory->print_constraints_enforced();
            return true;
        }
        else if (my_param == thisAgent->explanationMemory->settings->identity)
        {
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to discuss with the command 'explain chunk [chunk-name | chunk ID]'.");
            }
            thisAgent->explanationMemory->print_identity_set_explanation();
            return true;

        }
        else if (my_param == thisAgent->explanationMemory->settings->stats)
        {
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to discuss with the command 'explain chunk [chunk-name | chunk ID]'.");
            }
            thisAgent->explanationMemory->print_chunk_stats();
            return true;

        }
        else if ((my_param == thisAgent->explanationMemory->settings->help_cmd) || (my_param == thisAgent->explanationMemory->settings->qhelp_cmd))
        {
            thisAgent->explanationMemory->settings->print_explanation_settings(thisAgent);
            return true;
        }
        else {
            /* Done with dummy command parameters.  Remaining parameters are all settings*/
            if (!pArg2)
            {
                tempStringStream << my_param->get_name() << " =" ;
                PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
                return true;
            } else {
                if (!my_param->validate_string(pArg2->c_str()))
                {
                    return SetError("Invalid argument for explain command. Use 'explain ?' to see a list of valid sub-commands and settings.");
                }

                bool result = my_param->set_string(pArg2->c_str());

                if (!result)
                {
                    return SetError("The explain parameter could not be changed.");
                }
                else
                {
                    tempStringStream << my_param->get_name() << " = " << pArg2->c_str();
                    PrintCLIMessage(&tempStringStream);
                }
                return result;
            }
        }
    }
//    /* Handle options that enable/disable recording of chunk formation */
//    if (options.test(EXPLAIN_ALL))
//    {
//        thisAgent->explanationMemory->set_enabled(true);
//        thisAgent->outputManager->printa_sf(thisAgent, "Will monitor all chunks created.\n");
//        return true;
//    }
//    if (options.test(EXPLAIN_ONLY_SPECIFIC))
//    {
//        thisAgent->explanationMemory->set_enabled(false);
//        thisAgent->outputManager->printa_sf(thisAgent, "Will only monitor specific chunks.\n");
//        return true;
//    }
//    if (options.test(EXPLAIN_JUSTIFICATIONS))
//    {
//        if (pStringParameter->empty())
//        {
//            thisAgent->outputManager->printa_sf(thisAgent, "Soar is%scurrently recording justifications.\n", thisAgent->explanationMemory->isRecordingJustifications() ? " " : " not ");
//        } else {
//            if (pStringParameter->at(0) == 'y')
//            {
//                thisAgent->explanationMemory->set_justifications_enabled(true);
//            } else if (pStringParameter->at(0) == 'n')
//            {
//                thisAgent->explanationMemory->set_justifications_enabled(true);
//            } else {
//                thisAgent->outputManager->printa_sf(thisAgent, "Invalid argument '%s' to explain --justifications.\n", pStringParameter->c_str());
//            }
//        }
//        return true;
//    }
//    /* Handle options that required a currently discussed chunk/justification */
//    if (!thisAgent->explanationMemory->current_discussed_chunk_exists() && (options.test(EXPLAIN_FORMATION) || options.test(EXPLAIN_CONSTRAINTS) ||
//        options.test(EXPLAIN_IDENTITY_SETS) || options.test(EXPLAIN_STATS) || options.test(EXPLAIN_EXPLANATION_TRACE) || options.test(EXPLAIN_WME_TRACE)))
//    {
//        SetError("Please first specify the chunk you want to discuss with the command 'explain [chunk-name]' or 'explain chunk [chunk ID]'.");
//        return false;
//    }
//    else
//    {
//            if (options.test(EXPLAIN_FORMATION))
//        {
//            thisAgent->explanationMemory->print_formation_explanation();
//        }
//        if (options.test(EXPLAIN_CONSTRAINTS))
//        {
//            thisAgent->explanationMemory->print_constraints_enforced();
//        }
//        if (options.test(EXPLAIN_IDENTITY_SETS))
//        {
//            thisAgent->explanationMemory->print_identity_set_explanation();
//        }
//        if (options.test(EXPLAIN_STATS))
//        {
//            thisAgent->explanationMemory->print_chunk_stats();
//        }
//        if (options.test(EXPLAIN_EXPLANATION_TRACE))
//        {
//            thisAgent->explanationMemory->switch_to_explanation_trace(true);
//        }
//        if (options.test(EXPLAIN_WME_TRACE))
//        {
//            thisAgent->explanationMemory->switch_to_explanation_trace(false);
//        }
//    }
//
//    /* Handle global stats command*/
//    if (options.test(EXPLAIN_GLOBAL_STATS))
//    {
//            thisAgent->explanationMemory->print_explainer_stats();
//            return true;
//    }
//
//    /* Handle global stats command*/
//    if (options.test(EXPLAIN_LIST_ALL))
//    {
//            thisAgent->explanationMemory->print_all_chunks();
//            return true;
//    }
//
//    /* Handle record command*/
//    if (options.test(EXPLAIN_RECORD))
//    {
//        if (pStringParameter->empty())
//        {
//            thisAgent->explanationMemory->print_all_watched_rules();
//        } else {
//            return thisAgent->explanationMemory->watch_rule(pStringParameter);
//        }
//    }
//
//    /* Handle non-option explain commands for rules and Soar data structures */
//    if (!options.any())
//    {
//        if (pStringParameter->empty())
//        {
//                    thisAgent->explanationMemory->print_explain_summary();
//                    return true;
//        } else if (pStringParameter2->empty()) {
//            return thisAgent->explanationMemory->explain_chunk(pStringParameter);
//        } else {
//            return thisAgent->explanationMemory->explain_item(pStringParameter, pStringParameter2);
//        }
//    } else {
//        if (!pStringParameter->empty())
//        {
//            SetError("Those options cannot take additional arguments.  Ignoring.\n");
//            return false;
//        }
//    }
    return false;
}

