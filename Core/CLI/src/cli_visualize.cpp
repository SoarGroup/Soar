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

#include <string>
using namespace cli;
using namespace sml;

bool CommandLineInterface::DoVisualize(VisualizeBitset options, const std::string* pStringParameter, const std::string* pStringParameter2)
{

	agent* thisAgent = m_pAgentSML->GetSoarAgent();
	bool lReturn_Value = false;

	/* Handle options that enable/disable recording of chunk formation */
	if (options.test(VISUALIZE_IMAGE_LAUNCH))
	{
		//          thisAgent->outputManager->print_visualization_file();
		print(thisAgent, "Toggled image launch.\n");
	}
	if (options.test(VISUALIZE_PRINT_TO_SCREEN))
	{
		print(thisAgent, "Toggled screen printing.\n");
	}
	if (options.test(VISUALIZE_RAW_LAUNCH))
	{
		//          thisAgent->outputManager->print_visualization_file();
		print(thisAgent, "Toggled raw viz file launch.\n");
	}

	if (options.test(VISUALIZE_FILENAME))
	{
		if (pStringParameter->empty())
		{
			//          thisAgent->outputManager->print_visualization_file();
		} else {
			//        	thisAgent->outputManager->set_visualization_file();
		}
	}

	/* Handle non-option visualize commands for rules and Soar data structures */
	if (!options.any())
	{
		if (pStringParameter->empty())
		{
			return SetError("Please specify what you want to visualize.\n");
		} else {
			char lFirstChar = pStringParameter->at(0);
			char lSecondChar = (pStringParameter->length() > 1 ? pStringParameter->at(1) : 0);
			switch (lFirstChar)
			{
				case 'e':
					if (!lSecondChar || (lSecondChar == 'x'))
					{
						options.set(Cli::VISUALIZE_EXPLAIN);
					} else if (lSecondChar == 'p') {
						options.set(Cli::VISUALIZE_EPMEM);
					} else {
						return SetError("That is not a valid visualization type.\n");
					}
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
			if (options.test(Cli::VISUALIZE_EXPLAIN))
			{
				if (!pStringParameter2->empty())
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
			if (options.test(Cli::VISUALIZE_EPMEM)) {
				uint64_t lEpID;
				if (pStringParameter2->empty() || !from_string(lEpID, pStringParameter2->c_str()))
				{
					return SetError("Please specify the episode id you want to visualize.\n");
				}
				PrintCLIMessage_Header("Visualization of epmem...", 40);
				epmem_visualize_episode(thisAgent, lEpID, &thisAgent->explanationLogger->graphviz_output);
				lValidVisualizationGenerated = true;
			}
			if (options.test(Cli::VISUALIZE_SMEM))
			{
		        smem_lti_id lti_id = NIL;
		        unsigned int depth = 1;

		        // visualizing the store requires an open semantic database
		        smem_attach(thisAgent);

		        if (!pStringParameter2->empty())
		        {
		            soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, pStringParameter2->c_str());
		            if (lexeme.type == IDENTIFIER_LEXEME)
		            {
		                if (thisAgent->smem_db->get_status() == soar_module::connected)
		                {
		                    lti_id = smem_lti_get_id(thisAgent, lexeme.id_letter, lexeme.id_number);
		                    /* Need to add another parameter to logic first */
//		                    if ((lti_id != NIL) && pVal)
//		                    {
//		                        from_c_string(depth, pVal->c_str());
//		                    }
		                }
		            }

		            if (lti_id == NIL)
		            {
		                return SetError("Invalid long-term identifier.");
		            }
		        }

		        if (lti_id == NIL)
		        {
		            smem_visualize_store(thisAgent, &thisAgent->explanationLogger->graphviz_output);
		        }
		        else
		        {
		            smem_visualize_lti(thisAgent, lti_id, depth, &thisAgent->explanationLogger->graphviz_output);
		        }
				lValidVisualizationGenerated = true;
			}
			if (options.test(Cli::VISUALIZE_WM))
			{
//				thisAgent->outputManager->visualize_wm(thisAgent);
			}
			if (lValidVisualizationGenerated)
			{
		        if (thisAgent->explanationLogger->graphviz_output.empty())
		        {
					thisAgent->explanationLogger->clear_visualization();
		            return SetError("Visualization produced nothing.");
		        }
				PrintCLIMessage_Header("Opening visualization...", 40);
				std::string filename("/Users/mazzin/Soar/SoarSandbox/soar_visualization.gv");
				if (!DoCLog(LOG_NEW, &filename, 0, true))
				{
					thisAgent->explanationLogger->clear_visualization();
					return SetError("Error:  Could not open visualization file!\n");
				}

				if (!DoCLog(LOG_ADD, 0, &thisAgent->explanationLogger->graphviz_output, true))
				{
					thisAgent->explanationLogger->clear_visualization();
					return SetError("Error:  Could not write visualization output!\n");
				}

				if (!DoCLog(LOG_CLOSE, 0, 0, true))
				{
					thisAgent->explanationLogger->clear_visualization();
					return SetError("Error:  Could not close file!\n");
				}
				thisAgent->explanationLogger->clear_visualization();
				system("dot -Tsvg /Users/mazzin/Soar/SoarSandbox/soar_visualization.gv -o /Users/mazzin/Soar/SoarSandbox/soar_visualization.svg");
				system("open /Users/mazzin/Soar/SoarSandbox/soar_visualization.svg");
				system("open /Users/mazzin/Soar/SoarSandbox/soar_visualization.gv");
			}
		}
	} else {
		if (!pStringParameter->empty())
		{
			return SetError("Those options cannot take additional arguments.  Ignoring extra arguments.\n");
		}
	}
	return true;
}

