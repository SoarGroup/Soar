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
#include "episodic_memory.h"
#include "explanation_memory.h"
#include "lexer.h"
#include "misc.h"
#include "semantic_memory.h"
#include "output_manager.h"
#include <string>
#include "../../SoarKernel/src/visualizer/visualize.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoVisualize(const std::string* pArg1, const std::string* pArg2, const std::string* pArg3)
{

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempStringStream;
    bool lValidVisualizationGenerated = false;

    std::string lSystemCommand;

    if (!pArg1)
    {
        PrintCLIMessage("Visualize creates graphical representations of Soar's memory systems and past learning events.\n\n"
            "Use 'visualize ?' and 'help visualize' to learn more about the visualize command.");
        return true;
    }
    else
    {
        /* Single command argument */
        soar_module::param* my_param = thisAgent->visualizationManager->settings->get(pArg1->c_str());
        if (!my_param)
        {
            return SetError("Invalid visualize sub-command.  Use 'soar ?' to see a list of valid sub-commands and settings.");
        }
        else if (my_param == thisAgent->visualizationManager->settings->viz_wm)
        {
            if (pArg2)
            {

                soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, pArg2->c_str());
                if (lexeme.type == IDENTIFIER_LEXEME)
                {
                    Symbol* pSym = thisAgent->symbolManager->find_identifier(lexeme.id_letter, lexeme.id_number);
                    if (pSym)
                    {
                        if (pArg3)
                        {
                            int lDepth;
                            if (!from_string(lDepth, pArg3->c_str()) || (lDepth < 1))
                            {
                                return SetError("Invalid depth specified to visualize");
                            } else {
                                thisAgent->visualizationManager->visualize_wm(pSym, lDepth);
                            }

                        } else {
                            thisAgent->visualizationManager->visualize_wm(pSym);
                        }
                    } else {
                        return SetError("Invalid identifier specified to visualize");
                    }
                } else {
                    return SetError("Invalid identifier specified to visualize");
                }
            } else {
                thisAgent->visualizationManager->visualize_wm();
            }
            lValidVisualizationGenerated = true;
        }
        else if (my_param == thisAgent->visualizationManager->settings->viz_smem)
        {
            uint64_t lti_id = NIL;

            // visualizing the store requires an open semantic database
            thisAgent->SMem->attach();

            if (pArg2)
            {

                const char* pAttr_c_str = pArg2->c_str();
                soar::Lexer lexer(thisAgent, pAttr_c_str);
                if (!lexer.get_lexeme()) return SetError("Value not found.");
                if (lexer.current_lexeme.type == AT_LEXEME)
                {
                    if (!lexer.get_lexeme()) return SetError("Nothing found after @");
                }
                if (lexer.current_lexeme.type == INT_CONSTANT_LEXEME)
                {
                    if (thisAgent->SMem->connected())
                    {
                        lti_id = thisAgent->SMem->lti_exists(lexer.current_lexeme.int_val);
                    }
                }

                if (lti_id == NIL)
                {
                    return SetError("Invalid long-term identifier for visualize command.");
                }
            }
            int lDepth = 1;
            if (pArg3 && (!from_string(lDepth, pArg3->c_str()) || (lDepth < 0)))
            {
                return SetError("Invalid depth parameter for visualize command.");
            }
            thisAgent->visualizationManager->visualize_smem(lti_id, lDepth);
            lValidVisualizationGenerated = true;
        }
        else if (my_param == thisAgent->visualizationManager->settings->viz_epmem)
        {
            uint64_t lEpID;
            if (!pArg2 || (pArg2 && (!from_string(lEpID, pArg2->c_str()) || (lEpID < 1))))
            {
                return SetError("Please specify a valid episode id.");
            }
            PrintCLIMessage_Header("Visualization of Episodic Memory", 40);
            epmem_visualize_episode(thisAgent, lEpID, &thisAgent->visualizationManager->graphviz_output);
            lValidVisualizationGenerated = true;
        }
        else if (my_param == thisAgent->visualizationManager->settings->viz_last)
        {
            if (pArg2)
            {
                return SetError("Explanation visualization cannot take an additional argument.\n");
            }
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to visualize with the command 'explain [chunk-name]' or 'explain chunk [chunk ID]'.\n");
            }

            thisAgent->explanationMemory->visualize_last_output();
            lValidVisualizationGenerated = true;
        }
        else if (my_param == thisAgent->visualizationManager->settings->viz_instantiations)
        {
            if (pArg2)
            {
                return SetError("Explanation visualization cannot take an additional argument.\n");
            }
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to visualize with the command 'explain [chunk-name]' or 'explain chunk [chunk ID]'.\n");
            }

            thisAgent->explanationMemory->visualize_instantiation_graph();
            lValidVisualizationGenerated = true;
        }
        else if (my_param == thisAgent->visualizationManager->settings->viz_contributors)
        {
            if (pArg2)
            {
                return SetError("Explanation visualization cannot take an additional argument.\n");
            }
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to visualize with the command 'explain [chunk-name]' or 'explain chunk [chunk ID]'.\n");
            }

            thisAgent->explanationMemory->visualize_contributors();
            lValidVisualizationGenerated = true;
        }
        else if ((my_param == thisAgent->visualizationManager->settings->help_cmd) || (my_param == thisAgent->visualizationManager->settings->qhelp_cmd))
        {
            thisAgent->visualizationManager->settings->print_visualization_settings(thisAgent);
            return true;
        } else {
            /* Done with dummy command parameters.  Remaining parameters are all settings*/
            if (!pArg2)
            {
                tempStringStream << my_param->get_name() << " =" ;
            PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
            return true;
            } else {
                if (!my_param->validate_string(pArg2->c_str()))
                {
                    return SetError("Invalid argument for visualize command. Use 'visualize ?' to see a list of valid sub-commands.");
                }

                bool result = my_param->set_string(pArg2->c_str());

                if (!result)
                {
                    return SetError("The visualize parameter could not be changed.");
                }
                else
                {
                    tempStringStream << my_param->get_name() << " = " << pArg2->c_str();
                    PrintCLIMessage(&tempStringStream);
                }
                return result;
            }
        }

        if (lValidVisualizationGenerated)
        {
            if (thisAgent->visualizationManager->graphviz_output.empty())
            {
                thisAgent->visualizationManager->clear_visualization();
                return SetError("Visualization produced nothing.");
            }
            PrintCLIMessage_Header("Opening visualization...", 40);
            std::string lFileName(thisAgent->visualizationManager->get_next_filename());
            std::string filename(lFileName);
            filename += ".gv";

            if (!DoCLog(LOG_NEW, &filename, 0, true))
            {
                thisAgent->visualizationManager->clear_visualization();
                return SetError("Error:  Could not open visualization file!\n");
            }

            if (!DoCLog(LOG_ADD, 0, &thisAgent->visualizationManager->graphviz_output, true))
            {
                thisAgent->visualizationManager->clear_visualization();
                return SetError("Error:  Could not write visualization output!\n");
            }

            if (!DoCLog(LOG_CLOSE, 0, 0, true))
            {
                thisAgent->visualizationManager->clear_visualization();
                return SetError("Error:  Could not close file!\n");
            }
            if (thisAgent->visualizationManager->settings->gen_image->get_value() || thisAgent->visualizationManager->settings->launch_viewer->get_value())
            {
                lSystemCommand = "dot -T";
                lSystemCommand += thisAgent->visualizationManager->settings->image_type->get_value();
                lSystemCommand += ' ';
                lSystemCommand += lFileName;
                lSystemCommand += ".gv -o ";
                lSystemCommand += lFileName;
                lSystemCommand += '.';
                lSystemCommand += thisAgent->visualizationManager->settings->image_type->get_value();
                system(lSystemCommand.c_str());
            }
            if (thisAgent->visualizationManager->settings->launch_viewer->get_value())
            {
                lSystemCommand = "open ";
                lSystemCommand += lFileName;
                lSystemCommand += '.';
                lSystemCommand += thisAgent->visualizationManager->settings->image_type->get_value();
                system(lSystemCommand.c_str());
            }
            if (thisAgent->visualizationManager->settings->launch_editor->get_value())
            {
                lSystemCommand = "open ";
                lSystemCommand += lFileName;
                lSystemCommand += ".gv";
                system(lSystemCommand.c_str());
            }
            if (thisAgent->visualizationManager->settings->print_gv->get_value())
            {
                PrintCLIMessage(thisAgent->visualizationManager->graphviz_output.c_str());
            }
            thisAgent->visualizationManager->clear_visualization();
        }

        return true;
    }

    return true;
}
