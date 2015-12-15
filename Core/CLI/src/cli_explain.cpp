/////////////////////////////////////////////////////////////////
// explain command file.
//
// Author: Mazin Assanie
// Date  : 2015
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"

#include "misc.h"
#include "agent.h"
#include "print.h"
#include "explain.h"
#include "condition.h"

using namespace cli;
using namespace sml;

void ExplainList(agent* thisAgent)
{
    explain_chunk_str* chunk;

    chunk = thisAgent->explain_chunk_list;

    if (!chunk)
    {
        print(thisAgent,  "Nothing to explain yet!\n");
    }
    else
    {
        print(thisAgent,  "List of all explained chunks/justifications:\n");
        while (chunk != NULL)
        {
            print(thisAgent,  "Have explanation for %s\n", chunk->name);
            chunk = chunk->next_chunk;
        }
    }
}

bool Explain(agent* thisAgent, const char* pProduction, int mode)
{
    // mode == -1 full
    // mode == 0 name
    // mode > 0 condition

    soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, const_cast<char*>(pProduction));

    if (lexeme.type != STR_CONSTANT_LEXEME)
    {
        return false; // invalid production
    }

    switch (mode)
    {
        case -1: // full
        {
            explain_chunk_str* chunk = find_chunk(thisAgent, thisAgent->explain_chunk_list, lexeme.string());
            if (chunk)
            {
                explain_trace_chunk(thisAgent, chunk);
            }
        }
        break;
        case 0:
        {
            explain_chunk_str* chunk = find_chunk(thisAgent, thisAgent->explain_chunk_list, lexeme.string());
            if (!chunk)
            {
                return false;
            }

            /* First print out the production in "normal" form */
            print(thisAgent,  "(sp %s\n  ", chunk->name);
            print_condition_list(thisAgent, chunk->conds, 2, false);
            print(thisAgent,  "\n-->\n   ");
            print_action_list(thisAgent, chunk->actions, 3, false);
            print(thisAgent,  ")\n\n");

            /* Then list each condition and the associated "ground" WME */
            int i = 0;
            condition* ground = chunk->all_grounds;

            for (condition* cond = chunk->conds; cond != NIL; cond = cond->next)
            {
                i++;
                print(thisAgent,  " %2d : ", i);
                print_condition(thisAgent, cond);

                while (get_printer_output_column(thisAgent) < COLUMNS_PER_LINE - 40)
                {
                    print(thisAgent,  " ");
                }

                print(thisAgent,  " Ground :");
                print_condition(thisAgent, ground);
                print(thisAgent,  "\n");
                ground = ground->next;
            }
        }
        break;
        default:
        {
            explain_chunk_str* chunk = find_chunk(thisAgent, thisAgent->explain_chunk_list, lexeme.string());
            if (!chunk)
            {
                return false;
            }

            condition* ground = find_ground(thisAgent, chunk, mode);
            if (!ground)
            {
                return false;
            }

            explain_trace(thisAgent, lexeme.string(), chunk->backtrace, ground);
        }
        break;
    }
    return true;
}

bool CommandLineInterface::DoExplain(ExplainBitset options, const std::string* pObject)
{

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (options.test(EXPLAIN_ENABLE))
    {
        print(thisAgent, "Explanations enabled.\n");
        return true;
    }
    if (options.test(EXPLAIN_DISABLE))
    {
        print(thisAgent, "Explanations disabled.\n");
        return true;
    }
    if (!pObject)
    {
        // No arguments
        ExplainList(thisAgent);
        return true;
    }

    if (options.test(EXPLAIN_DEPENDENCY))
    {
        print(thisAgent, "Printing dependency analysis for chunk...\n");
    }
    if (options.test(EXPLAIN_CONSTRAINTS))
    {
        print(thisAgent, "Printing floating constraints for chunk...\n");
    }
    if (options.test(EXPLAIN_IDENTITY_SETS))
    {
        print(thisAgent, "Printing identity set mappings for chunk...\n");
    }

    return Explain(thisAgent, pObject->c_str(), 1);
}

