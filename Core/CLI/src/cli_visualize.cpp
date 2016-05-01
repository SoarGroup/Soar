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
#include "explain.h"
#include "misc.h"
#include "print.h"
#include "visualize.h"

#include <string>
using namespace cli;
using namespace sml;

bool check_boolean_option(agent* thisAgent, size_t pWhichBit, Cli::VisualizeBitset options, Cli::VisualizeBitset pNewSettings, bool pCurrentSetting)
{
    if (options.test(pWhichBit))
    {
        if (pCurrentSetting == pNewSettings.test(pWhichBit))
        {
            print(thisAgent, "Visualization setting is already %s.\n", pCurrentSetting ? "enabled" : "disabled");
            return false;
        }
        return true;
    }
    return false;
}

bool CommandLineInterface::DoVisualize(VisualizeBitset options, VisualizeBitset pSettings, const std::string& pObject, const std::string& pObject2, const std::string& pFileName, const std::string& pLineStyle, const std::string& pImageType)
{

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::string lSystemCommand;
    bool lReturn_Value = false;

    if (check_boolean_option(thisAgent, VISUALIZE_GENERATE_IMAGE, options, pSettings, thisAgent->visualizer->is_generate_img_enabled()))
    {
        thisAgent->visualizer->set_generate_img_enabled(pSettings.test(VISUALIZE_GENERATE_IMAGE));
        print(thisAgent, "Visualizer will%salways generate an image.\n", thisAgent->visualizer->is_generate_img_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_USE_SAME_FILE, options, pSettings, thisAgent->visualizer->is_use_same_file_enabled()))
    {
        thisAgent->visualizer->set_use_same_file_enabled(pSettings.test(VISUALIZE_USE_SAME_FILE));
        print(thisAgent, "Visualizer will%suse overwrite the same file for each visualization.\n", thisAgent->visualizer->is_use_same_file_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_LAUNCH_VIEWER, options, pSettings, thisAgent->visualizer->is_viz_launch_img_enabled()))
    {
        thisAgent->visualizer->set_viz_launch_img_enabled(pSettings.test(VISUALIZE_LAUNCH_VIEWER));
        print(thisAgent, "Visualizer will%sopen image in viewer.\n", thisAgent->visualizer->is_viz_launch_img_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_LAUNCH_EDITOR, options, pSettings, thisAgent->visualizer->is_viz_launch_gv_enabled()))
    {
        thisAgent->visualizer->set_viz_launch_gv_enabled(pSettings.test(VISUALIZE_LAUNCH_EDITOR));
        print(thisAgent, "Visualizer will%sopen GraphViz file in editor.\n", thisAgent->visualizer->is_viz_launch_gv_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_PRINT_TO_SCREEN, options, pSettings, thisAgent->visualizer->is_viz_print_enabled()))
    {
        thisAgent->visualizer->set_viz_print_enabled(pSettings.test(VISUALIZE_PRINT_TO_SCREEN));
        print(thisAgent, "Graphviz visualization output will%sbe printed to the screen.\n", thisAgent->visualizer->is_viz_print_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_ARCH_SHOW, options, pSettings, thisAgent->visualizer->is_include_arch_enabled()))
    {
        thisAgent->visualizer->set_include_arch_enabled(pSettings.test(VISUALIZE_ARCH_SHOW));
        print(thisAgent, "Graphviz visualizations will%sinclude architectural links.\n", thisAgent->visualizer->is_include_arch_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_ONLY_RULE_NAME, options, pSettings, thisAgent->visualizer->is_simple_inst_enabled()))
    {
        thisAgent->visualizer->set_simple_inst_enabled(pSettings.test(VISUALIZE_ONLY_RULE_NAME));
        if (thisAgent->visualizer->is_simple_inst_enabled())
        {
            thisAgent->visualizer->set_simple_inst_enabled(false);
            print(thisAgent, "Visualizer will print rule condition, actions and any relevant meta information.\n");
        } else {
            thisAgent->visualizer->set_simple_inst_enabled(true);
            print(thisAgent, "Visualizer will only print the name of rules.\n");
        }
    }
    if (options.test(VISUALIZE_STYLE_LINE))
    {
        print(thisAgent, "Visualizer will connect lines using GraphViz style '%s'.\n", pLineStyle.c_str());
        thisAgent->visualizer->set_line_style(pLineStyle);
    }
    if (options.test(VISUALIZE_FILENAME))
    {
        print(thisAgent, "Visualizer wil name Graphviz files using prefix %s.\n", pFileName.c_str());
        thisAgent->visualizer->set_filename(pFileName);
    }
    if (options.test(VISUALIZE_IMAGE_TYPE))
    {
        print(thisAgent, "Visualizer will use DOT to generate images of type %s.\n", pFileName.c_str());
        thisAgent->visualizer->set_image_type(pImageType);
    }

    if (!pObject.empty())
    {
        char lFirstChar = pObject.at(0);
        char lSecondChar = (pObject.length() > 1 ? pObject.at(1) : 0);
        switch (lFirstChar)
        {
            case 'c':
                options.set(Cli::VISUALIZE_EXPLAIN_CONTRIBUTORS);
                break;
            case 'e':
                if (!lSecondChar || (lSecondChar == 'x'))
                {
                    options.set(Cli::VISUALIZE_EXPLAIN_LAST);
                } else if (lSecondChar == 'p') {
                    options.set(Cli::VISUALIZE_EPMEM);
                } else {
                    return SetError("That is not a valid visualization type.\n");
                }
                break;
            case 'i':
                options.set(Cli::VISUALIZE_EXPLAIN_IG);
                break;
            case 'w':
                options.set(Cli::VISUALIZE_WM);
                break;
            case 's':
                options.set(Cli::VISUALIZE_SMEM);
                break;
            default:
                return SetError("That is not a valid visualization type.\n");
                break;
        }
        bool lValidVisualizationGenerated = false;
        if (options.test(Cli::VISUALIZE_EXPLAIN_LAST))
        {
            if (!pObject2.empty())
            {
                return SetError("Explanation visualization cannot take an additional argument.\n");
            }
            if (!thisAgent->explanationLogger->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to visualize with the command 'explain [chunk-name]' or 'explain chunk [chunk ID]'.\n");
            }

            thisAgent->explanationLogger->visualize_last_output();
            lValidVisualizationGenerated = true;
        }
        if (options.test(Cli::VISUALIZE_EXPLAIN_IG))
        {
            if (!pObject2.empty())
            {
                return SetError("Explanation visualization cannot take an additional argument.\n");
            }
            if (!thisAgent->explanationLogger->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to visualize with the command 'explain [chunk-name]' or 'explain chunk [chunk ID]'.\n");
            }

            thisAgent->explanationLogger->visualize_instantiation_graph();
            lValidVisualizationGenerated = true;
        }
        if (options.test(Cli::VISUALIZE_EXPLAIN_CONTRIBUTORS))
        {
            if (!pObject2.empty())
            {
                return SetError("Explanation visualization cannot take an additional argument.\n");
            }
            if (!thisAgent->explanationLogger->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to visualize with the command 'explain [chunk-name]' or 'explain chunk [chunk ID]'.\n");
            }

            thisAgent->explanationLogger->visualize_contributors();
            lValidVisualizationGenerated = true;
        }
        if (options.test(Cli::VISUALIZE_EPMEM)) {
            uint64_t lEpID;
            if (pObject2.empty() || !from_string(lEpID, pObject2.c_str()))
            {
                return SetError("Please specify the episode id you want to visualize.\n");
            }
            PrintCLIMessage_Header("Visualization of epmem...", 40);
            epmem_visualize_episode(thisAgent, lEpID, &thisAgent->visualizer->graphviz_output);
            lValidVisualizationGenerated = true;
        }
        if (options.test(Cli::VISUALIZE_SMEM))
        {
            smem_lti_id lti_id = NIL;
            unsigned int depth = 1;

            // visualizing the store requires an open semantic database
            smem_attach(thisAgent);

            if (!pObject2.empty())
            {
                soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, pObject2.c_str());
                if (lexeme.type == IDENTIFIER_LEXEME)
                {
                    if (thisAgent->smem_db->get_status() == soar_module::connected)
                    {
                        lti_id = smem_lti_get_id(thisAgent, lexeme.id_letter, lexeme.id_number);
                        /* Need to add another parameter to logic first */
                        //                          if ((lti_id != NIL) && pVal)
                        //                          {
                        //                              from_c_string(depth, pVal->c_str());
                        //                          }
                    }
                }

                if (lti_id == NIL)
                {
                    return SetError("Invalid long-term identifier.");
                }
            }

            if (lti_id == NIL)
            {
                smem_visualize_store(thisAgent, &thisAgent->visualizer->graphviz_output);
            }
            else
            {
                smem_visualize_lti(thisAgent, lti_id, depth, &thisAgent->visualizer->graphviz_output);
            }
            lValidVisualizationGenerated = true;
        }
        if (options.test(Cli::VISUALIZE_WM))
        {
            thisAgent->visualizer->visualize_wm();
            lValidVisualizationGenerated = true;
        }
        if (lValidVisualizationGenerated)
        {
            if (thisAgent->visualizer->graphviz_output.empty())
            {
                thisAgent->visualizer->clear_visualization();
                return SetError("Visualization produced nothing.");
            }
            PrintCLIMessage_Header("Opening visualization...", 40);
            std::string filename(thisAgent->visualizer->get_filename());
            filename += ".gv";

            if (!DoCLog(LOG_NEW, &filename, 0, true))
            {
                thisAgent->visualizer->clear_visualization();
                return SetError("Error:  Could not open visualization file!\n");
            }

            if (!DoCLog(LOG_ADD, 0, &thisAgent->visualizer->graphviz_output, true))
            {
                thisAgent->visualizer->clear_visualization();
                return SetError("Error:  Could not write visualization output!\n");
            }

            if (!DoCLog(LOG_CLOSE, 0, 0, true))
            {
                thisAgent->visualizer->clear_visualization();
                return SetError("Error:  Could not close file!\n");
            }
            if (thisAgent->visualizer->is_generate_img_enabled() || thisAgent->visualizer->is_viz_launch_img_enabled())
            {
                lSystemCommand = "dot -T";
                lSystemCommand += thisAgent->visualizer->get_image_type();
                lSystemCommand += ' ';
                lSystemCommand += thisAgent->visualizer->get_filename();
                lSystemCommand += ".gv -o ";
                lSystemCommand += thisAgent->visualizer->get_filename();
                lSystemCommand += '.';
                lSystemCommand += thisAgent->visualizer->get_image_type();
                system(lSystemCommand.c_str());
            }
            if (thisAgent->visualizer->is_viz_launch_img_enabled())
            {
                lSystemCommand = "open ";
                lSystemCommand += thisAgent->visualizer->get_filename();
                lSystemCommand += '.';
                lSystemCommand += thisAgent->visualizer->get_image_type();
                system(lSystemCommand.c_str());
            }
            if (thisAgent->visualizer->is_viz_launch_gv_enabled())
            {
                lSystemCommand = "open ";
                lSystemCommand += thisAgent->visualizer->get_filename();
                lSystemCommand += ".gv";
                system(lSystemCommand.c_str());
            }
            if (thisAgent->visualizer->is_viz_print_enabled())
            {
                PrintCLIMessage(thisAgent->visualizer->graphviz_output.c_str());
            }
            thisAgent->visualizer->clear_visualization();
        }
    } else if (!options.any())
    {
        PrintCLIMessage_Header("Visualization Settings", 50);
        PrintCLIMessage_Section("Actions", 50);
        PrintCLIMessage_Justify("Print to screen (-p):", (thisAgent->visualizer->is_viz_print_enabled() ? "Yes" : "No"), 50);
        PrintCLIMessage_Justify("Launch viewer (-v):", (thisAgent->visualizer->is_viz_launch_img_enabled() ? "Yes" : "No"), 50);
        PrintCLIMessage_Justify("Launch editor (-e):", (thisAgent->visualizer->is_viz_launch_gv_enabled() ? "Yes" : "No"), 50);
        PrintCLIMessage_Section("File", 50);
        PrintCLIMessage_Justify("Generate image file (-g):", (thisAgent->visualizer->is_generate_img_enabled() ? "Yes" : "No"), 50);
        PrintCLIMessage_Justify("Image type (-i):", thisAgent->visualizer->get_image_type(), 50);
        PrintCLIMessage_Justify("Use same file each time (-u):", (thisAgent->visualizer->is_include_arch_enabled() ? "Yes" : "No"), 50);
        PrintCLIMessage_Justify("Filename prefix (-f):", thisAgent->visualizer->get_filename(), 50);
        PrintCLIMessage_Section("Presentation", 50);
        PrintCLIMessage_Justify("Object style (-o):", (thisAgent->visualizer->is_simple_inst_enabled() ? "simple" : "complex"), 50);
        PrintCLIMessage_Justify("Line style (-l):", thisAgent->visualizer->get_line_style(), 50);
        PrintCLIMessage_Justify("Include architectural links (-a):", (thisAgent->visualizer->is_include_arch_enabled() ? "Yes" : "No"), 50);
        PrintCLIMessage("");
    }
    return true;
}

