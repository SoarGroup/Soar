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

bool CommandLineInterface::DoVisualize(VisualizeBitset options, const std::string* pObject, const std::string* pObject2, const std::string* pFileName, const std::string* pLineStyle, const std::string* pObjectStyle)
{

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    bool lReturn_Value = false;

    /* Handle options that enable/disable recording of chunk formation */
    if (options.test(VISUALIZE_GENERATE_IMAGE))
    {
        //        thisAgent->visualizer->toggle_viz_gen_img_enabled();
        //        print(thisAgent, "Graphviz visualization image will%sbe generated.\n",
        //            thisAgent->visualizer->is_viz_gen_img_enabled() ? " " : " not ");
    }
    if (options.test(VISUALIZE_USE_SAME_FILE))
    {
        //        thisAgent->visualizer->toggle_use_same_file_enabled();
        //        print(thisAgent, "Graphviz visualization will%suse the same filename every time.\n",
        //            thisAgent->visualizer->is_use_same_file_enabled() ? " " : " not ");
    }
    if (options.test(VISUALIZE_LAUNCH_VIEWER))
    {
        thisAgent->visualizer->toggle_viz_launch_img_enabled();
        print(thisAgent, "Graphviz visualization image%swill be launched in viewer.\n",
            thisAgent->visualizer->is_viz_launch_img_enabled() ? " " : " not ");
    }
    if (options.test(VISUALIZE_LAUNCH_EDITOR))
    {
        thisAgent->visualizer->toggle_viz_launch_gv_enabled();
        print(thisAgent, "Graphviz visualization file will%sbe launched in editor.\n",
            thisAgent->visualizer->is_viz_launch_gv_enabled() ? " " : " not ");
    }
    if (options.test(VISUALIZE_STYLE_LINE) && !pLineStyle->empty())
    {
        //        print(thisAgent, "Graphviz visualization line style set to %s.\n", lLineStyle->c_str());
        //        thisAgent->visualizer->set_line_style(lLineStyle);
    }
    if (options.test(VISUALIZE_STYLE_OBJECT) && !pObjectStyle->empty())
    {
        if (!strcmp(pObjectStyle->c_str(),"simple"))
        {
            thisAgent->visualizer->set_simple_inst_enabled(true);
        } else if (!strcmp(pObjectStyle->c_str(),"complex")) {
            thisAgent->visualizer->set_simple_inst_enabled(false);
        }
        print(thisAgent, "Graphviz visualization object style set to %s.\n", pObjectStyle->c_str());
    }
    if (options.test(VISUALIZE_PRINT_TO_SCREEN))
    {
        thisAgent->visualizer->toggle_viz_print_enabled();
        print(thisAgent, "Graphviz visualization output will%sbe printed to the screen.\n",
            thisAgent->visualizer->is_viz_print_enabled() ? " " : " not ");
    }
    if (options.test(VISUALIZE_STYLE_LINE))
    {
    }
    if (options.test(VISUALIZE_ARCH_SHOW))
    {
        //        thisAgent->visualizer->toggle_show_arch_links_enabled();
        //        print(thisAgent, "Graphviz visualization file will%sinclude architectural links.\n",
        //            thisAgent->visualizer->is_show_arch_links_enabled() ? " " : " not ");
    }

    if (options.test(VISUALIZE_FILENAME) && !pFileName->empty())
    {
        //        	thisAgent->outputManager->set_visualization_file();
    }

    if (!pObject->empty())
    {

        char lFirstChar = pObject->at(0);
        char lSecondChar = (pObject->length() > 1 ? pObject->at(1) : 0);
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
            if (!pObject2->empty())
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
            if (!pObject2->empty())
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
            if (!pObject2->empty())
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
            if (pObject2->empty() || !from_string(lEpID, pObject2->c_str()))
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

            if (!pObject2->empty())
            {
                soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, pObject2->c_str());
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
            std::string filename("/Users/mazzin/Soar/SoarSandbox/soar_visualization.gv");
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
            if (thisAgent->visualizer->is_viz_launch_img_enabled())
            {
                system("dot -Tsvg /Users/mazzin/Soar/SoarSandbox/soar_visualization.gv -o /Users/mazzin/Soar/SoarSandbox/soar_visualization.svg");
                system("open /Users/mazzin/Soar/SoarSandbox/soar_visualization.svg");
            }
            if (thisAgent->visualizer->is_viz_launch_gv_enabled())
            {
                system("open /Users/mazzin/Soar/SoarSandbox/soar_visualization.gv");
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
        PrintCLIMessage_Justify("Generate image file (-g):", (thisAgent->visualizer->is_viz_print_enabled() ? "Yes" : "No"), 50);
        PrintCLIMessage_Justify("Use same file each time (-u):", (thisAgent->visualizer->is_viz_print_enabled() ? "Yes" : "No"), 50);
        PrintCLIMessage_Justify("Filename prefix (-f):", "soar_visualization", 50);
        PrintCLIMessage_Section("Presentation", 50);
        PrintCLIMessage_Justify("Object style (-o):", (thisAgent->visualizer->is_simple_inst_enabled() ? "simple" : "complex"), 50);
        PrintCLIMessage_Justify("Line style (-l):", (thisAgent->visualizer->is_include_chunk_enabled() ? "Yes" : "No"), 50);
        PrintCLIMessage("");
    }
    return true;
}

