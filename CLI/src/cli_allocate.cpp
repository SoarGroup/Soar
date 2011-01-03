#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoAllocate(const std::string& pool, int blocks) 
{
    if (pool.empty())
    {
        GetMemoryPoolStatistics(); // cli_stats.cpp
        return true;
    }

    agent* agnt = m_pAgentSML->GetSoarAgent();
    for (memory_pool* p = agnt->memory_pools_in_use; p != NIL; p = p->next) 
    {
        if (pool == p->name)
        {
            for (int i = 0; i < blocks; ++i)
                add_block_to_memory_pool(agnt, p);

            m_Result << p->name << " blocks increased by " << blocks;
            return true;
        }
    }
    SetError("Invalid pool: " + pool);
    return false;
}



