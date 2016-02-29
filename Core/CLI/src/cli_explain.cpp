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

using namespace cli;
using namespace sml;

bool is_explain_id(const std::string* pStringParameter)
{
    return false;
}

bool CommandLineInterface::DoExplain(ExplainBitset options, const std::string* pStringParameter, const std::string* pStringParameter2)
{

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    bool lReturn_Value = false;

    /* Handle options that enable/disable recording of chunk formation */
    if (options.test(EXPLAIN_ALL))
    {
        thisAgent->explanationLogger->set_enabled(true);
        print(thisAgent, "Will monitor all chunks created.\n");
        return true;
    }
    if (options.test(EXPLAIN_ONLY_SPECIFIC))
    {
        thisAgent->explanationLogger->set_enabled(false);
        print(thisAgent, "Will only monitor specific chunks or time intervals.\n");
        return true;
    }
    /* Handle options that required a currently discussed chunk/justification */
    if (!thisAgent->explanationLogger->current_discussed_chunk_exists() && (options.test(EXPLAIN_FORMATION) || options.test(EXPLAIN_CONSTRAINTS) ||
        options.test(EXPLAIN_IDENTITY_SETS) || options.test(EXPLAIN_STATS) || options.test(EXPLAIN_EXPLANATION_TRACE) || options.test(EXPLAIN_WME_TRACE)))
    {
        print(thisAgent, "Please first specify the chunk you want to discuss with the command 'explain [chunk-name]' or 'explain chunk [chunk ID]'.");
        return false;
    }
    else
    {
        if (options.test(EXPLAIN_FORMATION))
        {
            thisAgent->explanationLogger->print_formation_explanation();
        }
        if (options.test(EXPLAIN_CONSTRAINTS))
        {
            thisAgent->explanationLogger->print_constraints_enforced();
        }
        if (options.test(EXPLAIN_IDENTITY_SETS))
        {
            thisAgent->explanationLogger->print_identity_set_explanation();
        }
        if (options.test(EXPLAIN_STATS))
        {
            thisAgent->explanationLogger->print_chunk_stats();
        }
        if (options.test(EXPLAIN_EXPLANATION_TRACE))
        {
            thisAgent->explanationLogger->switch_to_explanation_trace(true);
        }
        if (options.test(EXPLAIN_WME_TRACE))
        {
            thisAgent->explanationLogger->switch_to_explanation_trace(false);
        }
    }

    /* Handle global stats command*/
    if (options.test(EXPLAIN_GLOBAL_STATS))
    {
        thisAgent->explanationLogger->print_explainer_stats();
        return true;
    }

    /* Handle global stats command*/
    if (options.test(EXPLAIN_LIST_ALL))
    {
        thisAgent->explanationLogger->print_all_chunks();
        return true;
    }

    /* Handle global stats command*/
    if (options.test(EXPLAIN_RECORD))
    {
        if (pStringParameter->empty())
        {
            thisAgent->explanationLogger->print_all_watched_rules();
        } else {
            return thisAgent->explanationLogger->watch_rule(pStringParameter);
        }
    }

    /* Handle non-option explain commands for rules and Soar data structures */
    if (!options.any())
    {
        if (pStringParameter->empty())
        {
            thisAgent->explanationLogger->print_explain_summary();
            return true;
        } else if (pStringParameter2->empty()) {
            return thisAgent->explanationLogger->explain_chunk(pStringParameter);
        } else {
            return thisAgent->explanationLogger->explain_item(pStringParameter, pStringParameter2);
        }
    } else {
        if (!pStringParameter->empty())
        {
            print(thisAgent, "Those options cannot take additional arguments.  Ignoring.\n");
            return false;
        }
    }
    return false;
}

