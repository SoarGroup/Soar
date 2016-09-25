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
            thisAgent->outputManager->printa_sf(thisAgent, "Visualization setting is already %s.\n", pCurrentSetting ? "enabled" : "disabled");
            return false;
        }
        return true;
    }
    return false;
}

bool CommandLineInterface::DoVisualize(VisualizeBitset options, VisualizeBitset pSettings, const std::string& pObject, const std::string& pObject2, const std::string& pFileName, const std::string& pLineStyle, const std::string& pImageType, int pDepth)
{

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::string lSystemCommand;
    bool lReturn_Value = false;

    if (check_boolean_option(thisAgent, VISUALIZE_GENERATE_IMAGE, options, pSettings, thisAgent->visualizationManager->is_generate_img_enabled()))
    {
        thisAgent->visualizationManager->set_generate_img_enabled(pSettings.test(VISUALIZE_GENERATE_IMAGE));
        thisAgent->outputManager->printa_sf(thisAgent, "Visualizer will%sgenerate an image of visualization and save it to disk.\n", thisAgent->visualizationManager->is_generate_img_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_USE_SAME_FILE, options, pSettings, thisAgent->visualizationManager->is_use_same_file_enabled()))
    {
        thisAgent->visualizationManager->set_use_same_file_enabled(pSettings.test(VISUALIZE_USE_SAME_FILE));
        thisAgent->outputManager->printa_sf(thisAgent, "Visualizer will%soverwrite the same file for each visualization.\n", thisAgent->visualizationManager->is_use_same_file_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_LAUNCH_VIEWER, options, pSettings, thisAgent->visualizationManager->is_viz_launch_img_enabled()))
    {
        thisAgent->visualizationManager->set_viz_launch_img_enabled(pSettings.test(VISUALIZE_LAUNCH_VIEWER));
        thisAgent->outputManager->printa_sf(thisAgent, "Visualizer will%sopen image in viewer.\n", thisAgent->visualizationManager->is_viz_launch_img_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_LAUNCH_EDITOR, options, pSettings, thisAgent->visualizationManager->is_viz_launch_gv_enabled()))
    {
        thisAgent->visualizationManager->set_viz_launch_gv_enabled(pSettings.test(VISUALIZE_LAUNCH_EDITOR));
        thisAgent->outputManager->printa_sf(thisAgent, "Visualizer will%sopen GraphViz file in editor.\n", thisAgent->visualizationManager->is_viz_launch_gv_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_PRINT_TO_SCREEN, options, pSettings, thisAgent->visualizationManager->is_viz_print_enabled()))
    {
        thisAgent->visualizationManager->set_viz_print_enabled(pSettings.test(VISUALIZE_PRINT_TO_SCREEN));
        thisAgent->outputManager->printa_sf(thisAgent, "Graphviz visualization output will%sbe printed to the screen.\n", thisAgent->visualizationManager->is_viz_print_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_ARCH_SHOW, options, pSettings, thisAgent->visualizationManager->is_include_arch_enabled()))
    {
        thisAgent->visualizationManager->set_include_arch_enabled(pSettings.test(VISUALIZE_ARCH_SHOW));
        thisAgent->outputManager->printa_sf(thisAgent, "Graphviz visualizations will%sinclude architectural links.\n", thisAgent->visualizationManager->is_include_arch_enabled() ? " " : " not ");
    }
    if (check_boolean_option(thisAgent, VISUALIZE_ONLY_RULE_NAME, options, pSettings, thisAgent->visualizationManager->is_simple_inst_enabled()))
    {
        thisAgent->visualizationManager->set_simple_inst_enabled(pSettings.test(VISUALIZE_ONLY_RULE_NAME));
        if (thisAgent->visualizationManager->is_simple_inst_enabled())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Visualizer will only print the name of rules.\n");
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "Visualizer will print rule condition, actions and any relevant meta information.\n");
        }
    }
    if (options.test(VISUALIZE_STYLE_LINE))
    {
        thisAgent->outputManager->printa_sf(thisAgent, "Visualizer will connect lines using GraphViz style '%s'.\n", pLineStyle.c_str());
        thisAgent->visualizationManager->set_line_style(pLineStyle);
    }
    if (options.test(VISUALIZE_FILENAME))
    {
        thisAgent->outputManager->printa_sf(thisAgent, "Visualizer wil name Graphviz files using prefix %s.\n", pFileName.c_str());
        thisAgent->visualizationManager->set_filename(pFileName);
    }
    if (options.test(VISUALIZE_IMAGE_TYPE))
    {
        thisAgent->outputManager->printa_sf(thisAgent, "Visualizer will use DOT to generate images of type %s.\n", pFileName.c_str());
        thisAgent->visualizationManager->set_image_type(pImageType);
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
            if (!thisAgent->explanationMemory->current_discussed_chunk_exists())
            {
                return SetError("Please first specify the chunk you want to visualize with the command 'explain [chunk-name]' or 'explain chunk [chunk ID]'.\n");
            }

            thisAgent->explanationMemory->visualize_last_output();
            lValidVisualizationGenerated = true;
        }
        if (options.test(Cli::VISUALIZE_EXPLAIN_IG))
        {
            if (!pObject2.empty())
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
        if (options.test(Cli::VISUALIZE_EXPLAIN_CONTRIBUTORS))
        {
            if (!pObject2.empty())
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
        if (options.test(Cli::VISUALIZE_EPMEM)) {
            uint64_t lEpID;
            if (pObject2.empty() || !from_string(lEpID, pObject2.c_str()))
            {
                return SetError("Please specify the episode id you want to visualize.\n");
            }
            PrintCLIMessage_Header("Visualization of epmem...", 40);
            epmem_visualize_episode(thisAgent, lEpID, &thisAgent->visualizationManager->graphviz_output);
            lValidVisualizationGenerated = true;
        }
        if (options.test(Cli::VISUALIZE_SMEM))
        {
            uint64_t lti_id = NIL;

            // visualizing the store requires an open semantic database
            thisAgent->SMem->attach();

            if (!pObject2.empty())
            {
                soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, pObject2.c_str());
                if (lexeme.type == IDENTIFIER_LEXEME)
                {
                    if (thisAgent->SMem->connected())
                    {
                        lti_id = thisAgent->SMem->lti_exists(lexeme.id_number);
                    }
                }

                if (lti_id == NIL)
                {
                    return SetError("Invalid long-term identifier.");
                }
            }

            thisAgent->visualizationManager->visualize_smem(lti_id, pDepth);
            lValidVisualizationGenerated = true;
        }
        if (options.test(Cli::VISUALIZE_WM))
        {
            thisAgent->visualizationManager->visualize_wm();
            lValidVisualizationGenerated = true;
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
            if (thisAgent->visualizationManager->is_generate_img_enabled() || thisAgent->visualizationManager->is_viz_launch_img_enabled())
            {
                lSystemCommand = "dot -T";
                lSystemCommand += thisAgent->visualizationManager->get_image_type();
                lSystemCommand += ' ';
                lSystemCommand += lFileName;
                lSystemCommand += ".gv -o ";
                lSystemCommand += lFileName;
                lSystemCommand += '.';
                lSystemCommand += thisAgent->visualizationManager->get_image_type();
                system(lSystemCommand.c_str());
            }
            if (thisAgent->visualizationManager->is_viz_launch_img_enabled())
            {
                lSystemCommand = "open ";
                lSystemCommand += lFileName;
                lSystemCommand += '.';
                lSystemCommand += thisAgent->visualizationManager->get_image_type();
                system(lSystemCommand.c_str());
            }
            if (thisAgent->visualizationManager->is_viz_launch_gv_enabled())
            {
                lSystemCommand = "open ";
                lSystemCommand += lFileName;
                lSystemCommand += ".gv";
                system(lSystemCommand.c_str());
            }
            if (thisAgent->visualizationManager->is_viz_print_enabled())
            {
                PrintCLIMessage(thisAgent->visualizationManager->graphviz_output.c_str());
            }
            thisAgent->visualizationManager->clear_visualization();
        }
    } else if (!options.any())
    {
        std::string tempString;
        Output_Manager* outputManager = &Output_Manager::Get_OM();
        outputManager->reset_column_indents();
        outputManager->set_column_indent(0, 40);
        outputManager->set_column_indent(1, 55);
        outputManager->printa_sf(thisAgent, "======= Visualization Commands and Settings =======\n");
        outputManager->printa_sf(thisAgent, "visualize ? %-%-%s\n", "Print this help listing");
        outputManager->printa_sf(thisAgent, "visualize [wm | smem | epmem] [id] %-%-%s\n", "Visualize from memory system");
        outputManager->printa_sf(thisAgent, "visualize [ last | instantiations | contributors] %-%-%s\n", "Visualize explainer analysis");
        outputManager->printa_sf(thisAgent, "------------------ Presentation -------------------\n");
        outputManager->printa_sf(thisAgent, "--only-show-rule-name         %-%s%-%s\n",
            capitalizYesNo(thisAgent->visualizationManager->is_simple_inst_enabled()), "Only print rule name of instantiation");
        outputManager->printa_sf(thisAgent, "--architectural-links         %-%s%-%s\n",
            capitalizYesNo(thisAgent->visualizationManager->is_include_arch_enabled()), "Include architectural links");
        outputManager->printa_sf(thisAgent, "--line-style                  %-%s%-%s\n",
            thisAgent->visualizationManager->get_line_style(), "Graphviz line style that will be used");
        outputManager->printa_sf(thisAgent, "------------------ File Handling ------------------\n");
        outputManager->printa_sf(thisAgent, "--generate-image              %-%s%-%s\n",
            capitalizYesNo(thisAgent->visualizationManager->is_generate_img_enabled()), "Whether an image should be created");
        outputManager->printa_sf(thisAgent, "--image-type                  %-%s%-%s\n",
            thisAgent->visualizationManager->get_image_type(), "Image type that will be generated");
        outputManager->printa_sf(thisAgent, "--use-same-file               %-%s%-%s\n",
            capitalizYesNo(thisAgent->visualizationManager->is_use_same_file_enabled()), "Use same file each time");
        outputManager->printa_sf(thisAgent, "--filename                    %-%s\n",
            thisAgent->visualizationManager->get_filename());
        outputManager->printa_sf(thisAgent, "------------------ Post Actions -------------------\n");
        outputManager->printa_sf(thisAgent, "--print                       %-%s%-%s\n",
            capitalizYesNo(thisAgent->visualizationManager->is_viz_print_enabled()), "Print data file to screen");
        outputManager->printa_sf(thisAgent, "--viewer-launch               %-%s%-%s\n",
            capitalizYesNo(thisAgent->visualizationManager->is_viz_launch_img_enabled()), "Launch image in viewer");
        outputManager->printa_sf(thisAgent, "--editor-launch               %-%s%-%s\n",
            capitalizYesNo(thisAgent->visualizationManager->is_viz_launch_gv_enabled()), "Open data file in editor");
    }
    return true;
}

