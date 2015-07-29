#include "portability.h"

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "agent.h"
#include "sml_AgentSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoAllocate(const std::string& pool, int blocks)
{
    if (pool.empty())
    {
        GetMemoryPoolStatistics(); // cli_stats.cpp
        return true;
    }

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (thisAgent->memoryManager->add_block_to_memory_pool_by_name(pool, blocks))
    {
        m_Result << pool << " blocks increased by " << blocks;
        return true;
    }

    SetError("Could not allocate memory.  Probably a bad pool name: " + pool);
    return false;
}



