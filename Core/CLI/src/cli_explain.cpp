/////////////////////////////////////////////////////////////////
// explain command file.
//
// Author: Mazin Assanie
// Date  : 2015
//
/////////////////////////////////////////////////////////////////

#include <explain.h>
#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "sml_Names.h"
#include "sml_Utils.h"


#include "agent.h"
#include "condition.h"
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

    if (options.test(EXPLAIN_ALL))
    {
        thisAgent->explanationLogger->should_explain_all(true);
        print(thisAgent, "Will monitoring all chunks created.\n");
        return true;
    }

    if (options.test(EXPLAIN_SPECIFIC))
    {
        thisAgent->explanationLogger->should_explain_all(false);
        print(thisAgent, "Will only monitor specific chunks or time intervals.\n");
        return true;
    }

    if (!thisAgent->explanationLogger->current_discussed_chunk_exists() && (options.test(EXPLAIN_BACKTRACE) || options.test(EXPLAIN_CONSTRAINTS) || options.test(EXPLAIN_IDENTITY_SETS)))
    {
        print(thisAgent, "Please first specify the chunk you want to discuss with the command 'explain [chunk-name]'.");
        return false;
    }

    if (options.test(EXPLAIN_BACKTRACE))
    {
            PrintCLIMessage_Header("Dependency Analysis", 40);
            thisAgent->explanationLogger->explain_dependency_analysis();
    }
    if (options.test(EXPLAIN_CONSTRAINTS))
    {
            PrintCLIMessage_Header("Constraints enforced during problem-solving", 40);
            thisAgent->explanationLogger->explain_constraints_enforced();
    }
    if (options.test(EXPLAIN_IDENTITY_SETS))
    {
            PrintCLIMessage_Header("Identity set assignments", 40);
            thisAgent->explanationLogger->explain_identity_sets();
    }
    if (options.test(EXPLAIN_STATS))
    {
            thisAgent->explanationLogger->explain_stats();
    }
    if (options.test(EXPLAIN_TIME))
    {
            print(thisAgent, "Not yet implemented.  Will allow you to explain all chunks created in a time interval specified in decision cycles.\n");
            return true;
    }

    if (!options.any())
    {
        if (pStringParameter->empty())
        {
            thisAgent->explanationLogger->explain_chunking_summary();
            return true;
        } else if (pStringParameter2->empty()) {
            print(thisAgent, "Attempting to explain chunk/rule.\n");
            return thisAgent->explanationLogger->explain_rule(pStringParameter);
        } else {
            print(thisAgent, "Attempting to explain condition/instantiation.\n");
            return thisAgent->explanationLogger->explain_item(pStringParameter, pStringParameter2);
        }
    } else {
        if (pStringParameter->empty())
        {
            /* One of the -- options above was executed */
            return true;
        } else {
            print(thisAgent, "Those options cannot take additional arguments.  Ignoring.\n");
            return true;
        }
    }
}

